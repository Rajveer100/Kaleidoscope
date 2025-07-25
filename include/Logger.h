//===- Logger.h - Lexer base class ---------------------------------------===//
//
// Author: Rajveer <rajveer.developer@icloud.com>
//
//===----------------------------------------------------------------------===//
//
// Helper functions for logging errors.
//
//===----------------------------------------------------------------------===//

#ifndef KALEIDOSCOPE_LOGGER_H
#define KALEIDOSCOPE_LOGGER_H

#include "ASTExpr.h"
#include "llvm/IR/Value.h"

class Logger {
public:
  /// Error handling helper function for ExprAST.
  static std::unique_ptr<ExprAST> LogError(const char *Str) {
    fprintf(stderr, "Error: %s\n", Str);
    return nullptr;
  }

  /// Error handling helper function for ProtoTypeAST.
  static std::unique_ptr<ProtoTypeAST> LogErrorP(const char *Str) {
    LogError(Str);
    return nullptr;
  }

  /// Error handling helper function for Value.
  static llvm::Value *LogErrorV(const char *Str) {
    LogError(Str);
    return nullptr;
  }
};

#endif // KALEIDOSCOPE_LOGGER_H
