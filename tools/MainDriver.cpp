//===- MainDriver.cpp - Main Driver support code --------------------------===//
//
// Author: Rajveer <rajveer.developer@icloud.com>
//
//===----------------------------------------------------------------------===//
//
// Implementation of the main driver.
//
//===----------------------------------------------------------------------===//

#include "CodeGen.h"
#include "Lexer.h"
#include "Parser.h"
#include "llvm/Support/TargetSelect.h"
#include <cstdio>

extern "C" double putchard(double x) {
  fputc((char)x, stderr);
  return 0;
}

extern "C" double printd(double x) {
  fprintf(stderr, "%f\n", x);
  return 0;
}

int main() {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  LLVMInitializeAArch64Target();
  LLVMInitializeAArch64AsmPrinter();
  LLVMInitializeAArch64AsmParser();

  Lexer Lexer;
  Parser Parser;

  // Prime the first token.
  fprintf(stderr, "ready> ");
  Lexer.getNextTok();

  // Run the main loop.
  Parser.MainLoop(Lexer);

  // Print out all the generated code.
  Parser.CG.Module->print(llvm::errs(), nullptr);

  return 0;
}
