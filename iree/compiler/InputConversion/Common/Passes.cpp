// Copyright 2021 The IREE Authors
//
// Licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "iree/compiler/InputConversion/Common/Passes.h"

#include "iree/compiler/Dialect/Flow/Transforms/Passes.h"
#include "mlir/Pass/PassManager.h"
#include "mlir/Pass/PassOptions.h"
#include "mlir/Pass/PassRegistry.h"
#include "mlir/Transforms/Passes.h"

namespace mlir {
namespace iree_compiler {

void registerCommonConversionPassPipelines() {
  PassPipelineRegistration<> common(
      "iree-common-input-transformation-pipeline",
      "Runs the common input transformation pipeline",
      [](OpPassManager &passManager) {
        buildCommonInputConversionPassPipeline(passManager);
      });
}

// Common transformations to prepare input dialects for IREE.
void buildCommonInputConversionPassPipeline(OpPassManager &passManager) {
  passManager.addNestedPass<FuncOp>(
      IREE::Flow::createConvertToFlowTensorOpsPass(
          /*runBeforeDispatchRegionFormation=*/true));
}

namespace {
#define GEN_PASS_REGISTRATION
#include "iree/compiler/InputConversion/Common/Passes.h.inc"  // IWYU pragma: export
}  // namespace

void registerCommonInputConversionPasses() {
  // Generated.
  registerPasses();

  // Pipelines.
  registerCommonConversionPassPipelines();
}

}  // namespace iree_compiler
}  // namespace mlir