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

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"
#include <map>

class CodeGen {
public:
  inline static std::unique_ptr<llvm::LLVMContext> Context;
  inline static std::unique_ptr<llvm::IRBuilder<>> Builder;
  inline static std::unique_ptr<llvm::Module> Module;
  inline static std::map<std::string, llvm::Value *> NamedValues;
};

#endif // KALEIDOSCOPE_CODEGEN_H
