//
// This file is distributed under the Apache License v2.0. See LICENSE for
// details.
//

#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/ADT/SmallSet.h"
#include "llvm/Demangle/Demangle.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Dominators.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/InlineAsm.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Transforms/Utils/BasicBlockUtils.h"
#include "llvm/Transforms/Utils/Local.h"

#include "omvll/ObfuscationConfig.hpp"
#include "omvll/PyConfig.hpp"
#include "omvll/log.hpp"
#include "omvll/passes/cfg-flattening/ControlFlowFlattening.hpp"
#include "omvll/utils.hpp"

#include <optional>

using namespace llvm;

namespace omvll {

constexpr uint32_t Encode(uint32_t Id, uint32_t X, uint32_t Y) {
  return (Id ^ X) + Y;
}

// Identity inline assembly: the result is a fixed value at runtime, but
// neither the optimizer nor the backend can fold it back into a constant.
template <class IRBTy> Value *EmitOpaqueInt(IRBTy &IRB, uint32_t V) {
  auto *FType = FunctionType::get(IRB.getInt32Ty(), {IRB.getInt32Ty()}, false);
  return IRB.CreateCall(FType,
                        InlineAsm::get(FType, "", "=r,0",
                                       /* hasSideEffects */ false,
                                       /* isStackAligned */ false),
                        {IRB.getInt32(V)});
}

// Absolute state update: the block holds the raw id and the switch label is
// recomputed at runtime from the opaque key, so it never appears literally.
template <class IRBTy>
void EmitTransition(IRBTy &IRB, AllocaInst *SV, BasicBlock *Dispatch,
                    uint32_t RawId, Value *X, Value *Y) {
  IRB.CreateStore(IRB.CreateAdd(IRB.CreateXor(IRB.getInt32(RawId), X), Y), SV,
                  true);
  IRB.CreateBr(Dispatch);
}

template <class IRBTy>
void EmitTransition(IRBTy &IRB, AllocaInst *SV, BasicBlock *Dispatch,
                    Value *Cond, uint32_t TrueRawId, uint32_t FalseRawId,
                    Value *X, Value *Y) {
  Value *RawId =
      IRB.CreateSelect(Cond, IRB.getInt32(TrueRawId), IRB.getInt32(FalseRawId));
  IRB.CreateStore(IRB.CreateAdd(IRB.CreateXor(RawId, X), Y), SV, true);
  IRB.CreateBr(Dispatch);
}

// Relative state update: the next label is derived from the current one, so
// the constant left in the block is meaningless without the predecessor.
template <class IRBTy>
void EmitRelTransition(IRBTy &IRB, AllocaInst *SV, BasicBlock *Dispatch,
                       uint32_t Delta) {
  LoadInst *Cur = IRB.CreateLoad(IRB.getInt32Ty(), SV, true);
  IRB.CreateStore(IRB.CreateXor(Cur, IRB.getInt32(Delta)), SV, true);
  IRB.CreateBr(Dispatch);
}

template <class IRBTy>
void EmitRelTransition(IRBTy &IRB, AllocaInst *SV, BasicBlock *Dispatch,
                       Value *Cond, uint32_t TrueDelta, uint32_t FalseDelta) {
  Value *Delta =
      IRB.CreateSelect(Cond, IRB.getInt32(TrueDelta), IRB.getInt32(FalseDelta));
  LoadInst *Cur = IRB.CreateLoad(IRB.getInt32Ty(), SV, true);
  IRB.CreateStore(IRB.CreateXor(Cur, Delta), SV, true);
  IRB.CreateBr(Dispatch);
}

// A longjmp resumes with whatever state the frame last held, which a relative
// update would never recover from.
static bool canReturnTwice(const Function &F) {
  for (const BasicBlock &BB : F)
    for (const Instruction &I : BB)
      if (const auto *CB = dyn_cast<CallBase>(&I))
        if (CB->hasFnAttr(Attribute::ReturnsTwice))
          return true;
  return false;
}

template <class IRBTy> void EmitDefaultCaseAssembly(IRBTy &IRB, Triple TT) {
  // clang-format off
  auto *FType = FunctionType::get(IRB.getVoidTy(), false);
  if (TT.isAArch64()) {
    IRB.CreateCall(FType, InlineAsm::get(
      FType,
      R"delim(
        ldr x1, #-8;
        blr x1;
        mov x0, x1;
        .byte 0xF1, 0xFF;
        .byte 0xF2, 0xA2;
      )delim",
      "",
      /* hasSideEffects */ true,
      /* isStackAligned */ true
    ));
  } else if (TT.isX86()) {
    // FIXME: This assembly may not confuse a decompiler.
    IRB.CreateCall(FType, InlineAsm::get(
      FType,
      R"delim(
        nop;
        .byte 0xF1, 0xFF;
        .byte 0xF2, 0xA2;
      )delim",
      "",
      /* hasSideEffects */ true,
      /* isStackAligned */ true
    ));
  } else if (TT.isARM() || TT.isThumb()) {
    IRB.CreateCall(FType, InlineAsm::get(
      FType,
      R"delim(
        ldr r1, [pc, #-4];
        bx r1;
        mov r1, r2;
        .byte 0xF1, 0xFF;
        .byte 0xF2, 0xA2;
      )delim",
      "",
      /* hasSideEffects */ true,
      /* isStackAligned */ true
    ));
  } else {
    fatalError("Unsupported target for Control-Flow Flattening obfuscation: " +
               TT.str());
  }
  // clang-format on
}

bool ControlFlowFlattening::runOnFunction(Function &F) {
  if (F.getInstructionCount() == 0)
    return false;

  // DemotePHIToStack / DemoteRegToStack cannot safely spill values that flow
  // into swifterror argument positions: the LLVM verifier requires those to
  // come from an alloca or parameter, not a stack-reloaded pointer.  Skip any
  // function whose entry block carries a swifterror alloca.
  for (const BasicBlock &BB : F)
    if (containsSwiftErrorAlloca(BB))
      return false;

  // The dispatcher takes over the entry block's terminator, so it has to be
  // one whose edges the switch can take over.
  const Instruction *EntryTerm = F.getEntryBlock().getTerminator();
  if (EntryTerm->getNumSuccessors() == 0 || isa<IndirectBrInst>(EntryTerm) ||
      isa<CallBrInst>(EntryTerm))
    return false;

  bool Changed = false;
  std::string DemangledName = demangle(F.getName().str());
  const uint32_t X = RandomGenerator::generateRange(10, UINT32_MAX);
  const uint32_t Y = RandomGenerator::generateRange(10, UINT32_MAX);
  const bool UseRelative = !canReturnTwice(F);

  SINFO("[{}] Visiting function {}", ControlFlowFlattening::name(),
        DemangledName);
  ScopedTrace TracePassFunc(F.getName().str(), name());

  SmallVector<BasicBlock *, 20> FlattedBBs;
  demotePHINode(F);

  BasicBlock *EntryBlock = &F.getEntryBlock();
  SmallPtrSet<BasicBlock *, 8> NormalDest2Split;

  // Blocks that keep an edge bypassing the dispatcher. The state they observe
  // belongs to their predecessor, so they cannot update it relatively.
  SmallPtrSet<BasicBlock *, 8> DirectEntry;

  auto IsFromInvoke = [](const auto &BB) {
    return any_of(predecessors(&BB), [](const auto *Pred) {
      return isa<InvokeInst>(Pred->getTerminator());
    });
  };

  for (BasicBlock &BB : F) {
    if (BB.isLandingPad())
      continue;

    if (IsFromInvoke(BB))
      NormalDest2Split.insert(&BB);
  }

  /*
   * +------------------+  +------------------+
   * |                  |  |                  |
   * +--------+---------+  +---------+--------+
   * | Normal | Unwind  |  |         |        |
   * +---+----+---------+  +----+----+--------+
   *     |                      |
   * +---+----+            +----+----+
   * |        |            |         +--+ Intermediate Block
   * |        |            +---------+  |
   * |        |                         |
   * +--------+            +---------+<-+ Original Block
   * Normal Dst            |         |
   *                       |         |
   *                       |         |
   *                       +---------+
   */

  for (BasicBlock *BB : NormalDest2Split) {
    auto *Trampoline = BasicBlock::Create(BB->getContext(), ".normal_split",
                                          BB->getParent(), BB);
    DirectEntry.insert(Trampoline);
    for (BasicBlock *Pred : predecessors(BB)) {
      // Handle Invoke
      if (auto *Invoke = dyn_cast<InvokeInst>(Pred->getTerminator())) {
        Invoke->setNormalDest(Trampoline);
        continue;
      }

      // Handle Branch
      if (auto *Branch = dyn_cast<BranchInst>(Pred->getTerminator())) {
        for (size_t Idx = 0; Idx < Branch->getNumSuccessors(); ++Idx) {
          if (Branch->getSuccessor(Idx) == BB)
            Branch->setSuccessor(Idx, Trampoline);
        }
        continue;
      }

      // Handle Switch
      if (auto *Switch = dyn_cast<SwitchInst>(Pred->getTerminator())) {
        SwitchInst::CaseIt Begin = Switch->case_begin();
        SwitchInst::CaseIt End = Switch->case_end();
        for (auto It = Begin; It != End; ++It) {
          if (It->getCaseSuccessor() == BB)
            It->setSuccessor(Trampoline);
        }
        continue;
      }
    }

    // Branch the Trampoline to the original BasicBlock.
    BranchInst::Create(BB, Trampoline);
  }

  for (BasicBlock &BB : F) {
    if (EntryBlock == &BB)
      continue;

    FlattedBBs.push_back(&BB);
  }

  // Terminators left untouched below keep branching to their successors
  // directly, and landing pads are still reached through the unwind edge.
  for (BasicBlock &BB : F) {
    Instruction *Term = BB.getTerminator();
    if (isa<IndirectBrInst>(Term) || isa<CallBrInst>(Term)) {
      for (BasicBlock *Succ : successors(&BB))
        DirectEntry.insert(Succ);
      continue;
    }

    // Only the cases of a switch are routed through the dispatcher below, its
    // default edge stays as it is.
    if (auto *Switch = dyn_cast<SwitchInst>(Term))
      DirectEntry.insert(Switch->getDefaultDest());
  }

  const size_t BlockSize =
      count_if(FlattedBBs, [](const auto *BB) { return !BB->isLandingPad(); });
  if (BlockSize <= 1) {
    SWARN("[{}] Block too small (#{}) to be flattened",
          ControlFlowFlattening::name(), FlattedBBs.size());
    return false;
  }

  if (auto *Branch = dyn_cast<BranchInst>(EntryBlock->getTerminator())) {
    if (Branch->isConditional()) {
      BasicBlock *EntrySplit =
          EntryBlock->splitBasicBlockBefore(Branch, "EntrySplit");
      FlattedBBs.insert(FlattedBBs.begin(), EntryBlock);

#ifdef OMVLL_DEBUG
        for (Instruction &I : *EntrySplit) {
          SDEBUG("[{}][EntrySplit] {}", ControlFlowFlattening::name(),
                 ToString(I));
        }

        for (Instruction &I : *EntryBlock) {
          SDEBUG("[{}][EntryBlock] {}", ControlFlowFlattening::name(),
                 ToString(I));
        }
#endif

        EntryBlock = EntrySplit;
    } else {
      SWARN("[{}] Found condition that is not an instruction",
            ControlFlowFlattening::name());
    }
  } else if (auto *Switch = dyn_cast<SwitchInst>(EntryBlock->getTerminator())) {
    BasicBlock *EntrySplit =
        EntryBlock->splitBasicBlockBefore(Switch, "EntrySplit");
    FlattedBBs.insert(FlattedBBs.begin(), EntryBlock);
    EntryBlock = EntrySplit;
  } else if (auto *Invoke = dyn_cast<InvokeInst>(EntryBlock->getTerminator())) {
    BasicBlock *EntrySplit =
        EntryBlock->splitBasicBlockBefore(Invoke, "EntrySplit");
    FlattedBBs.insert(FlattedBBs.begin(), EntryBlock);
    EntryBlock = EntrySplit;
  }

  SDEBUG("[{}] Erasing {}", ControlFlowFlattening::name(),
         ToString(*EntryBlock->getTerminator()));

  // The block the entry used to fall through to becomes the initial state. It
  // is not necessarily the one that comes next in the layout.
  BasicBlock *FirstBlock = EntryBlock->getTerminator()->getSuccessor(0);
  EntryBlock->getTerminator()->eraseFromParent();

  // Create a state encoding for the BB to flatten.
  DenseMap<BasicBlock *, uint32_t> SwitchEnc;
  SmallSet<uint32_t, 20> SwitchRnd;
  for (BasicBlock *ToFlat : FlattedBBs) {
    if (ToFlat->isLandingPad())
      // Landing pads are not embedded in the switch.
      continue;

    uint32_t Rnd = 0;
    do {
      Rnd = RandomGenerator::generateRange(10, UINT32_MAX);
      uint32_t Enc = Encode(Rnd, X, Y);
      if (!SwitchRnd.contains(Rnd) && !SwitchRnd.contains(Enc)) {
        SwitchRnd.insert(Rnd);
        SwitchRnd.insert(Enc);
        break;
      }
    } while (true);
    SwitchEnc[ToFlat] = Rnd;
  }

  IRBuilder<> EntryIR(EntryBlock);
  AllocaInst *SwitchVar =
      EntryIR.CreateAlloca(EntryIR.getInt32Ty(), 0, "SwitchVar");

  Value *OpaqueX = EmitOpaqueInt(EntryIR, X);
  Value *OpaqueY = EmitOpaqueInt(EntryIR, Y);

  auto ItFirst = SwitchEnc.find(FirstBlock);
  if (ItFirst == SwitchEnc.end())
    fatalError(
        fmt::format("Unable to find the encoded id for the entry successor: {}",
                    ToString(*FirstBlock)));

  EntryIR.CreateStore(
      EntryIR.CreateAdd(
          EntryIR.CreateXor(EntryIR.getInt32(ItFirst->second), OpaqueX),
          OpaqueY),
      SwitchVar, true);

  auto &Ctx = F.getContext();
  auto *FlatLoopEntry =
      BasicBlock::Create(Ctx, "FlatLoopEntry", &F, EntryBlock);
  auto *FlatLoopEnd = BasicBlock::Create(Ctx, "FlatLoopEnd", &F, EntryBlock);
  auto *DefaultCase = BasicBlock::Create(Ctx, "DefaultCase", &F, FlatLoopEnd);

  IRBuilder<> FlatLoopEntryIR(FlatLoopEntry), FlatLoopEndIR(FlatLoopEnd),
      DefaultCaseIR(DefaultCase);

  LoadInst *LoadSwitchVar = FlatLoopEntryIR.CreateLoad(
      FlatLoopEntryIR.getInt32Ty(), SwitchVar, "SwitchVar");
  EntryBlock->moveBefore(FlatLoopEntry);
  EntryIR.CreateBr(FlatLoopEntry);
  FlatLoopEndIR.CreateBr(FlatLoopEntry);

  EmitDefaultCaseAssembly(DefaultCaseIR,
                          Triple(F.getParent()->getTargetTriple()));
  DefaultCaseIR.CreateBr(FlatLoopEnd);

  SwitchInst *Switch = FlatLoopEntryIR.CreateSwitch(LoadSwitchVar, DefaultCase);

  for (BasicBlock *ToFlat : FlattedBBs) {
    if (ToFlat->isLandingPad())
      // Landing pads should not be present in the switch case since they are
      // not directly called by flattened blocks.
      continue;

    auto ItEncId = SwitchEnc.find(ToFlat);
    if (ItEncId == SwitchEnc.end())
      fatalError(
          fmt::format("Cannot find the encoded index for the basic block: {}",
                      ToString(*ToFlat)));

    uint32_t SwitchId = Encode(ItEncId->second, X, Y);
    ToFlat->moveBefore(FlatLoopEnd);
    auto *Id = dyn_cast<ConstantInt>(
        ConstantInt::get(Switch->getCondition()->getType(), SwitchId));
    Switch->addCase(Id, ToFlat);
  }

  // The state observed by a block is its own label, unless the block can also
  // be entered without going through the dispatcher.
  auto CurrentLabel = [&](BasicBlock *BB) -> std::optional<uint32_t> {
    if (!UseRelative || DirectEntry.contains(BB))
      return std::nullopt;
    auto It = SwitchEnc.find(BB);
    if (It == SwitchEnc.end())
      return std::nullopt;
    return Encode(It->second, X, Y);
  };

  // Update the basic block with the switch var.
  for (BasicBlock *ToFlat : FlattedBBs) {
    Instruction *Term = ToFlat->getTerminator();
    std::optional<uint32_t> CurLabel = CurrentLabel(ToFlat);
    SDEBUG("[{}] Flattening {} ({})", ControlFlowFlattening::name(),
           ToString(*ToFlat), ToString(*Term));

    if (isa<ReturnInst>(Term) || isa<UnreachableInst>(Term)) {
      /* Typically a ret instruction
       * {
       *  if (...) {
       *    return X;
       *  }
       * }
       *
       */
      continue;
    }

    if (isa<IndirectBrInst>(Term))
      // Avoid flattening when encountering an indirectbr.
      continue;

    if (isa<CallBrInst>(Term))
      // Avoid flattening when encountering an callbr.
      continue;

    if (isa<ResumeInst>(Term))
      // Nothing to do as it will 'resume' from information already known.
      continue;

    if (isa<InvokeInst>(Term))
      // Already processed with the early 'split'.
      continue;

    if (isa<SwitchInst>(Term)) {
      auto *SwitchTerm = dyn_cast<SwitchInst>(Term);

      std::vector<const SwitchInst::CaseHandle *> Cases;
      std::transform(
          SwitchTerm->case_begin(), SwitchTerm->case_end(),
          std::back_inserter(Cases),
          [](const SwitchInst::CaseHandle &Handle) { return &Handle; });

      for (const SwitchInst::CaseHandle &Handle : SwitchTerm->cases()) {
        BasicBlock *Target = Handle.getCaseSuccessor();
        auto ItEncId = SwitchEnc.find(Target);
        if (ItEncId == SwitchEnc.end())
          fatalError("Unable to find the encoded id for the basic block: " +
                     ToString(*Target));

        ConstantInt *DestId = Switch->findCaseDest(Target);
        if (!DestId)
          fatalError(fmt::format("Unable to find {} in the switch case",
                                 ToString(*Target)));

        const uint32_t EncId = ItEncId->second;
        auto *DispatchBlock = BasicBlock::Create(Ctx, "", &F, FlatLoopEnd);
        IRBuilder IRB(DispatchBlock);
        if (CurLabel)
          EmitRelTransition(IRB, SwitchVar, FlatLoopEnd,
                            *CurLabel ^ Encode(EncId, X, Y));
        else
          EmitTransition(IRB, SwitchVar, FlatLoopEnd, EncId, OpaqueX, OpaqueY);
        SwitchTerm->setSuccessor(Handle.getSuccessorIndex(), DispatchBlock);
      }

      continue;
    }

    auto *Branch = dyn_cast<BranchInst>(Term);
    if (!Branch)
      fatalError(fmt::format("[{}] Weird '{}' is not a branch",
                             ControlFlowFlattening::name().str(),
                             ToString(*Term)));

    if (Branch->isUnconditional()) {
      BasicBlock *Target = Branch->getSuccessor(0);
      auto ItEncId = SwitchEnc.find(Target);
      if (ItEncId == SwitchEnc.end())
        fatalError(fmt::format(
            "Unable to find the encoded id for the basic block: '{}'",
            ToString(*Target)));

      ConstantInt *DestId = Switch->findCaseDest(Target);
      if (!DestId)
        fatalError(fmt::format("Unable to find {} in the switch case",
                               ToString(*Target)));

      const uint32_t EncId = ItEncId->second;
      IRBuilder IRB(Branch);
      if (CurLabel)
        EmitRelTransition(IRB, SwitchVar, FlatLoopEnd,
                          *CurLabel ^ Encode(EncId, X, Y));
      else
        EmitTransition(IRB, SwitchVar, FlatLoopEnd, EncId, OpaqueX, OpaqueY);
      Branch->eraseFromParent();
      continue;
    }

    if (Branch->isConditional()) {
      BasicBlock *TrueCase = Branch->getSuccessor(0);
      BasicBlock *FalseCase = Branch->getSuccessor(1);

      auto ItTrue = SwitchEnc.find(TrueCase);
      auto ItFalse = SwitchEnc.find(FalseCase);

      if (ItTrue == SwitchEnc.end())
        fatalError(fmt::format(
            "Unable to find the encoded id for the (true) basic block: '{}'",
            ToString(*TrueCase)));

      if (ItFalse == SwitchEnc.end())
        fatalError(fmt::format(
            "Unable to find the encoded id for the (false) basic block: '{}'",
            ToString(*FalseCase)));

      ConstantInt *TrueId = Switch->findCaseDest(TrueCase);
      ConstantInt *FalseId = Switch->findCaseDest(FalseCase);

      if (!TrueId)
        fatalError(
            fmt::format("Unable to find {} (true case) in the switch case",
                        ToString(*TrueCase)));

      if (!FalseId)
        fatalError(
            fmt::format("Unable to find {} (false case) in the switch case",
                        ToString(*FalseCase)));

      const uint32_t TrueEncId = ItTrue->second;
      const uint32_t FalseEncId = ItFalse->second;

      IRBuilder IRB(Branch);
      if (CurLabel)
        EmitRelTransition(IRB, SwitchVar, FlatLoopEnd, Branch->getCondition(),
                          *CurLabel ^ Encode(TrueEncId, X, Y),
                          *CurLabel ^ Encode(FalseEncId, X, Y));
      else
        EmitTransition(IRB, SwitchVar, FlatLoopEnd, Branch->getCondition(),
                       TrueEncId, FalseEncId, OpaqueX, OpaqueY);
      Branch->eraseFromParent();
      continue;
    }
  }

  Changed = true;
  return Changed;
}

PreservedAnalyses ControlFlowFlattening::run(Module &M,
                                             ModuleAnalysisManager &MAM) {
  bool Changed = false;
  if (isModuleGloballyExcluded(&M)) {
    SINFO("Excluding module [{}]", M.getName());
    return PreservedAnalyses::all();
  }

  PyConfig &Config = PyConfig::instance();
  SINFO("[{}] Executing on module {}", name(), M.getName());
  ScopedTrace TracePassModule(name(), name());

  for (Function &F : M) {
    if (isFunctionGloballyExcluded(&F) ||
        !Config.getUserConfig()->controlFlowGraphFlattening(&M, &F) ||
        F.isDeclaration() || F.isIntrinsic() ||
        F.getName().starts_with("__omvll"))
      continue;

    if (isCoroutine(&F))
      continue;

    bool MadeChange = runOnFunction(F);
    if (MadeChange)
      reg2mem(F);

    Changed |= MadeChange;
  }

  SINFO("[{}] Changes {} applied on module {}", name(), Changed ? "" : "not",
        M.getName());

  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

} // end namespace omvll
