//===- CodeGen.h - Code Generation base class ----------------------------===//
//
// Author: Rajveer <rajveer.developer@icloud.com>
//
//===----------------------------------------------------------------------===//
//
// Generates LLVM intermediate representation (IR) code for the AST nodes.
//
//===----------------------------------------------------------------------===//

#ifndef KALEIDOSCOPE_CODEGEN_H
#define KALEIDOSCOPE_CODEGEN_H

#include "ASTExpr.h"
#include "KaleidoscopeJIT.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassInstrumentation.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Value.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"
#include <map>

class CodeGen {
public:
  std::unique_ptr<llvm::LLVMContext> Context;
  std::unique_ptr<llvm::IRBuilder<>> Builder;
  std::unique_ptr<llvm::Module> Module;
  std::map<std::string, llvm::Value *> NamedValues;

  std::unique_ptr<llvm::orc::KaleidoscopeJIT> JIT;

  std::unique_ptr<llvm::FunctionPassManager> FPM;
  std::unique_ptr<llvm::LoopAnalysisManager> LAM;
  std::unique_ptr<llvm::FunctionAnalysisManager> FAM;
  std::unique_ptr<llvm::CGSCCAnalysisManager> CGAM;
  std::unique_ptr<llvm::ModuleAnalysisManager> MAM;
  std::unique_ptr<llvm::PassInstrumentationCallbacks> PIC;
  std::unique_ptr<llvm::StandardInstrumentations> SI;

  std::map<std::string, std::unique_ptr<ProtoTypeAST>> FunctionProtos;

  llvm::ExitOnError ExitOnError;

  void InitialiseModuleAndPassManager() {
    JIT = CodeGen::ExitOnError(llvm::orc::KaleidoscopeJIT::Create());

    // Open a new context and module.
    Context = std::make_unique<llvm::LLVMContext>();
    Module = std::make_unique<llvm::Module>("KaleidoscopeJIT", *Context);
    Module->setDataLayout(JIT->getDataLayout());

    // Create a new builder for the module.
    Builder = std::make_unique<llvm::IRBuilder<>>(*Context);

    // Create new pass and analysis managers.
    FPM = std::make_unique<llvm::FunctionPassManager>();
    LAM = std::make_unique<llvm::LoopAnalysisManager>();
    FAM = std::make_unique<llvm::FunctionAnalysisManager>();
    CGAM = std::make_unique<llvm::CGSCCAnalysisManager>();
    MAM = std::make_unique<llvm::ModuleAnalysisManager>();
    PIC = std::make_unique<llvm::PassInstrumentationCallbacks>();
    SI = std::make_unique<llvm::StandardInstrumentations>(
        *Context, /*DebugLogging*/ true);

    SI->registerCallbacks(*PIC, MAM.get());

    // Transform passes for simple 'peephole' and bit-twiddling optimizations.
    FPM->addPass(llvm::InstCombinePass());
    // Reassociate expressions.
    FPM->addPass(llvm::ReassociatePass());
    // Eliminate common sub-expressions.
    FPM->addPass(llvm::GVNPass());
    // Simplify the control flow graph (ex: deleting unreachable blocks, ...)
    FPM->addPass(llvm::SimplifyCFGPass());

    // Register analysis passes used in these transform passes.
    llvm::PassBuilder PB;
    PB.registerModuleAnalyses(*MAM);
    PB.registerFunctionAnalyses(*FAM);
    PB.crossRegisterProxies(*LAM, *FAM, *CGAM, *MAM);
  }
};

#endif // KALEIDOSCOPE_CODEGEN_H
