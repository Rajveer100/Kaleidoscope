//===- Lexer.h - Lexer base class ----------------------------------------===//
//
// Author: Rajveer <rajveer.developer@icloud.com>
//
//===----------------------------------------------------------------------===//
//
// A lexer for the language by scanning input and breaking it into tokens.
//
//===----------------------------------------------------------------------===//

#ifndef KALEIDOSCOPE_LEXER_H
#define KALEIDOSCOPE_LEXER_H

#include <cstdlib>
#include <string>

/// Token returns enum values when valid, else returns its
/// ASCII value [0-255].
enum Token {
  TOK_EOF = -1,

  // Keywords
  TOK_DEF = -2,
  TOK_EXTERN = -3,

  // Primary
  TOK_IDENTIFIER = -4,
  TOK_NUMBER = -5,

  // Control
  TOK_IF = -6,
  TOK_THEN = -7,
  TOK_ELSE = -8,

  // For
  TOK_FOR = -9,
  TOK_IN = -10,
};

/// Lexer - The lexer returns tokens for valid input, else its ASCII value.
class Lexer {
  std::string IdentifierStr; // Filled in if TOK_IDENTIFIER
  double NumVal;             // Filled in if TOK_NUMBER

  /// Token buffer stores the current token the parser is looking at.
  int CurTok;
  int lastChar = ' ';

  /// Returns token from standard input.
  int getTok() {
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
      if (IdentifierStr == "if")
        return TOK_IF;
      if (IdentifierStr == "then")
        return TOK_THEN;
      if (IdentifierStr == "else")
        return TOK_ELSE;
      if (IdentifierStr == "for")
        return TOK_FOR;
      if (IdentifierStr == "in")
        return TOK_IN;
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

public:
  /// Updates token buffer by reading another token from the lexer.
  int getNextTok() {
    CurTok = getTok();
    return CurTok;
  }
  int &getCurTok() { return CurTok; };
  std::string &getIdentifierStr() { return IdentifierStr; };
  double &getNumVal() { return NumVal; };
};

#endif // KALEIDOSCOPE_LEXER_H
