//
// This file is distributed under the Apache License v2.0. See LICENSE for
// details.
//

#include "llvm/Demangle/Demangle.h"
#include "llvm/IR/Module.h"

#include "omvll/log.hpp"
#include "omvll/utils.hpp"
#include "omvll/passes/inline-jni/InlineJni.hpp"
#include "omvll/omvll_config.hpp"

using namespace llvm;

namespace omvll {

PreservedAnalyses InlineJni::run(Module &M, ModuleAnalysisManager &FAM) {
  if (!Config.InlineJniWrappers)
    return PreservedAnalyses::all();

  if (isModuleGloballyExcluded(&M)) {
    SINFO("Excluding module [{}]", M.getName());
    return PreservedAnalyses::all();
  }

  bool Changed = false;
  SINFO("[{}] Executing on module {}", name(), M.getName());
  ScopedTrace TracePassModule(name(), name());

  for (Function &F : M) {
    if (isFunctionGloballyExcluded(&F))
      continue;

    ScopedTrace TracePassFunc(F.getName(), name());
    std::string Name = demangle(F.getName().str());
    StringRef NRef = Name;
    if (NRef.starts_with("_JNIEnv::")) {
      SDEBUG("[{}] Inlining {}", name(), Name);
      F.addFnAttr(Attribute::AlwaysInline);
      Changed = true;
    }
  }

  SINFO("[{}] Changes {} applied on module {}", name(), Changed ? "" : "not",
        M.getName());

  return Changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
}

} // end namespace omvll
