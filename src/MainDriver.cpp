//
// Created by Rajveer Singh on 18/07/24.
// MainDriver.cpp
//
// Main Driver Code.

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
