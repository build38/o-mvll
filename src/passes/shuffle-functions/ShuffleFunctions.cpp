//
// This file is distributed under the Apache License v2.0. See LICENSE for
// details.
//

#include "llvm/IR/Module.h"

#include "omvll/log.hpp"
#include "omvll/utils.hpp"
#include "omvll/passes/shuffle-functions/ShuffleFunctions.hpp"
#include "omvll/omvll_config.hpp"

using namespace llvm;

namespace omvll {

PreservedAnalyses ShuffleFunctions::run(Module &M, ModuleAnalysisManager &FAM) {
  if (!Config.ShuffleFunctions)
    return PreservedAnalyses::all();

  if (isModuleGloballyExcluded(&M)) {
    SINFO("Excluding module [{}]", M.getName());
    return PreservedAnalyses::all();
  }

  SINFO("[{}] Executing on module {}", name(), M.getName());
  ScopedTrace TracePassModule(name(), name());

  shuffleFunctions(M);

  SINFO("[{}] Changes applied on module {}", name(), M.getName());

  return PreservedAnalyses::none();
}

} // end namespace omvll
