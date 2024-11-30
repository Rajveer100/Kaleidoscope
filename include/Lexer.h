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
};

/// Lexer - The lexer returns tokens for valid input, else its ASCII value.
class Lexer {
public:
  inline static std::string IdentifierStr; // Filled in if TOK_IDENTIFIER
  inline static double NumVal;             // Filled in if TOK_NUMBER

  /// Token buffer stores the current token the parser is looking at.
  inline static int CurTok;

  /// Returns token from standard input.
  static int getTok();

  /// Updates token buffer by reading another token from the lexer.
  static int getNextToken();
};

#endif // KALEIDOSCOPE_LEXER_H
