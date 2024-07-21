//
// Created by Rajveer Singh on 21/07/24.
// Logger.cpp

#include "ASTExpr.h"
#include "Logger.h"

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
