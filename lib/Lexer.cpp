//===- Lexer.cpp - Lexer support code -------------------------------------===//
//
// Author: Rajveer <rajveer.developer@icloud.com>
//
//===----------------------------------------------------------------------===//
//
// Implementation of the lexer for this language.
//
//===----------------------------------------------------------------------===//

#include "Lexer.h"

int Lexer::getTok() {
  static int lastChar = ' ';

  // Skip any whitespace.
  while (isspace(lastChar))
    lastChar = getchar();

  if (isalpha(lastChar)) { // Identifier: [a-zA-Z][a-zA-Z0-9]*
    IdentifierStr = lastChar;
    while (isalnum(lastChar = getchar())) {
      IdentifierStr += lastChar;
    }

    if (IdentifierStr == "def")
      return TOK_DEF;
    if (IdentifierStr == "extern")
      return TOK_EXTERN;
    return TOK_IDENTIFIER;
  }

  if (isdigit(lastChar) || lastChar == '.') { // Number: [0-9.]+
    std::string numStr;
    do {
      numStr += lastChar;
      lastChar = getchar();
    } while (isdigit(lastChar) || lastChar == '.');

    NumVal = strtod(numStr.c_str(), nullptr);
    return TOK_NUMBER;
  }

  if (lastChar == '#') {
    // Comment until end of line.
    do
      lastChar = getchar();
    while (lastChar != EOF && lastChar != '\n' && lastChar != '\r');

    if (lastChar != EOF)
      return getTok();
  }

  // If it's end of file, don't eat EOF.
  if (lastChar == EOF)
    return TOK_EOF;

  // Return ASCII value of character if none of the above conditions are
  // satisfied.
  int thisChar = lastChar;
  lastChar = getchar();
  return thisChar;
}

int Lexer::getNextToken() { return CurTok = getTok(); }
