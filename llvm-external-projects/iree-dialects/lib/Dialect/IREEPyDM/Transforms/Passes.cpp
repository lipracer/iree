// Copyright 2021 The IREE Authors
//
// Licensed under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

#include "iree-dialects/Dialect/IREEPyDM/Transforms/Passes.h"

#include "mlir/Pass/PassManager.h"

using namespace mlir;
using namespace mlir::iree_pydm;

void mlir::iree_pydm::buildLowerToIREEPassPipeline(
    OpPassManager& passManager, const LowerToIREEOptions& options) {
  // TODO: Needs to be iterative, support optimization passes, etc.
  passManager.addPass(createLowerIREEPyDMToRTLPass());
  if (options.linkRtlSource) {
    passManager.addPass(createLinkIREEPyDMRTLPass(options.linkRtlSource));
  }
  passManager.addPass(createConvertIREEPyDMToIREEPass());
}