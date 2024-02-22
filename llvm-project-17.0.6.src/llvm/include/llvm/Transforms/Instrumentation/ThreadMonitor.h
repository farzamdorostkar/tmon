//===- Transforms/Instrumentation/ThreadMonitor.h - TMon Pass -----------===//
//
// Author: Farzam Dorostkar
// Email:  farzam.dorostkar@polymtl.ca
// Lab:    DORSAL - Polytechnique Montreal
//
//===--------------------------------------------------------------------===//
//
// This file defines the ThreadMonitor (TMon) pass.
// This file was inspired by "ThreadSanitizer.h".
//
//===--------------------------------------------------------------------===//

#ifndef LLVM_TRANSFORMS_INSTRUMENTATION_THREADMONITOR_H
#define LLVM_TRANSFORMS_INSTRUMENTATION_THREADMONITOR_H

#include "llvm/IR/PassManager.h"

namespace llvm {
class Function;
class Module;

/// A function pass for TMon instrumentation.
///
/// Instruments functions to detect race conditions. This function pass
/// inserts ptwrite instrumentation.
struct ThreadMonitorPass : public PassInfoMixin<ThreadMonitorPass> {
  PreservedAnalyses run(Function &F, FunctionAnalysisManager &FAM);
  static bool isRequired() { return true; }
};

/// A module pass for tmon instrumentation.
///
/// Create ctor and init functions.
struct ModuleThreadMonitorPass
  : public PassInfoMixin<ModuleThreadMonitorPass> {
  PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM);
  static bool isRequired() { return true; }
};

} // namespace llvm
#endif /* LLVM_TRANSFORMS_INSTRUMENTATION_THREADMONITOR_H */