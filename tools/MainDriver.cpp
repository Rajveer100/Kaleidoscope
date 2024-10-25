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
#include "KaleidoscopeJIT.h"
#include "Lexer.h"
#include "Parser.h"
#include "llvm/Support/TargetSelect.h"

int main() {
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  // Install standard binary operators.
  BinOpPrecedence::init();

  // Prime the first token.
  fprintf(stderr, "ready> ");
  Lexer::getNextToken();

  CodeGen::JIT = CodeGen::ExitOnError(llvm::orc::KaleidoscopeJIT::Create());

  // Make the module, which holds all the code.
  Parser::initialiseModuleAndPassManager();

  // Run the main loop.
  Parser::mainLoop();

  // Print out all the generated code.
  CodeGen::Module->print(llvm::errs(), nullptr);

  return 0;
}
