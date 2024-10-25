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
#include "llvm/Passes/StandardInstrumentations.h"
#include <map>

class CodeGen {
public:
  inline static std::unique_ptr<llvm::LLVMContext> Context;
  inline static std::unique_ptr<llvm::IRBuilder<>> Builder;
  inline static std::unique_ptr<llvm::Module> Module;
  inline static std::map<std::string, llvm::Value *> NamedValues;

  inline static std::unique_ptr<llvm::orc::KaleidoscopeJIT> JIT;

  inline static std::unique_ptr<llvm::FunctionPassManager> FPM;
  inline static std::unique_ptr<llvm::LoopAnalysisManager> LAM;
  inline static std::unique_ptr<llvm::FunctionAnalysisManager> FAM;
  inline static std::unique_ptr<llvm::CGSCCAnalysisManager> CGAM;
  inline static std::unique_ptr<llvm::ModuleAnalysisManager> MAM;
  inline static std::unique_ptr<llvm::PassInstrumentationCallbacks> PIC;
  inline static std::unique_ptr<llvm::StandardInstrumentations> SI;

  inline static std::map<std::string, std::unique_ptr<ProtoTypeAST>>
      FunctionProtos;

  inline static llvm::ExitOnError ExitOnError;
};

#endif // KALEIDOSCOPE_CODEGEN_H
