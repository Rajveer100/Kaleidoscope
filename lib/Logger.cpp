//===- Logger.cpp - Logger support code -----------------------------------===//
//
// Author: Rajveer <rajveer.developer@icloud.com>
//
//===----------------------------------------------------------------------===//
//
// Implements logging helper functions.
//
//===----------------------------------------------------------------------===//

#include "Logger.h"
#include "ASTExpr.h"

std::unique_ptr<ExprAST> Logger::LogError(const char *Str) {
  fprintf(stderr, "Error: %s\n", Str);
  return nullptr;
}

std::unique_ptr<ProtoTypeAST> Logger::LogErrorP(const char *Str) {
  LogError(Str);
  return nullptr;
}

llvm::Value *Logger::LogErrorV(const char *Str) {
  LogError(Str);
  return nullptr;
}
