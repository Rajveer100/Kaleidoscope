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

int main() {
  // Install standard binary operators.
  BinOpPrecedence::init();

  // Prime the first token.
  fprintf(stderr, "ready> ");
  Lexer::getNextToken();

  // Make the module, which holds all the code.
  Parser::initialiseModule();

  // Run the main loop.
  Parser::mainLoop();

  // Print out all the generated code.
  CodeGen::Module->print(llvm::errs(), nullptr);

  return 0;
}
