//
// Created by Rajveer Singh on 17/07/24.
// Parser.cpp
//
// Parser utilises combination of recursive descent parsing (RDP)
// and operator-precedence parsing for the language.

#ifndef KALEIDOSCOPE_PARSER_H
#define KALEIDOSCOPE_PARSER_H

#include <map>
#include "Lexer.h"
#include "ASTExpr.h"
#include "Logger.h"

// Parser - The parser starts with the most simple literal,
// which are then used by compound literals to break down
// each production in the grammar.
class Parser {
  // NumberExpr ::= Number
  static std::unique_ptr<ExprAST> parseNumberExpr();

  // ParenExpr ::= '(' Expression ')'
  static std::unique_ptr<ExprAST> parseParenExpr();

  // IdentifierExpr
  //   ::= Identifier
  //   ::= Identifier '(' expression* ')'
  static std::unique_ptr<ExprAST> parseIdentifierExpr();

  // Primary
  //   ::= IdentifierExpr
  //   ::= NumberExpr
  //   ::= ParenExpr
  static std::unique_ptr<ExprAST> parsePrimary();

  // Expression
  //   ::= Primary Binorphs
  static std::unique_ptr<ExprAST> parseExpression();

  // Binorphs
  //   ::= ('+' primary)*
  static std::unique_ptr<ExprAST> parseBinOpRHS(int ExprPrec,
                                                std::unique_ptr<ExprAST> LHS);

  // ProtoType
  //   ::= id '(' id* ')'
  static std::unique_ptr<ProtoTypeAST> parseProtoType();

  // Definition ::= 'def' PrototTypeExpr
  static std::unique_ptr<FunctionAST> parseDefinition();

  // External ::= 'extern' ProtoType
  static std::unique_ptr<ProtoTypeAST> parseExtern();

  // TopLevelExpr ::= Expression
  static std::unique_ptr<FunctionAST> parseTopLevelExpr();

  static void handleDefinition();
  static void handleExtern();
  static void handleTopLevelExpression();

 public:

  //
  static void initialiseModule();

  // Top ::= Definition | External | Expression | ';'
  static void mainLoop();
};

// BinOpPrecedence - Holds the precedence for a valid binary operator.
class BinOpPrecedence {
  inline static std::map<char, int> precedence;

 public:
  // Get the precedence of the operator token.
  static int getTokPrecedence();
  // Initialise standard binary operators.
  static void init();
};

#endif //KALEIDOSCOPE_PARSER_H
