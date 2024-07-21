//
// Created by Rajveer Singh on 20/07/24.
//

#ifndef KALEIDOSCOPE_CODEGEN_H
#define KALEIDOSCOPE_CODEGEN_H

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
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

#endif //KALEIDOSCOPE_CODEGEN_H
