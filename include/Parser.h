//===- Parser.h - Parser base class ---------------------------------------===//
//
// Author: Rajveer <rajveer.developer@icloud.com>
//
//===----------------------------------------------------------------------===//
//
// Parser utilises combination of recursive descent parsing (RDP)
// and operator-precedence parsing for the language.
//
//===----------------------------------------------------------------------===//

#ifndef KALEIDOSCOPE_PARSER_H
#define KALEIDOSCOPE_PARSER_H

#include "ASTExpr.h"
#include <map>

/// The parser starts with the most simple literal,
/// which are then used by compound literals to break down
/// each production in the grammar.
class Parser {
  /// Parse numerical expressions.
  ///
  /// NumberExpr ::= Number
  static std::unique_ptr<ExprAST> parseNumberExpr();

  /// Parse expressions with parenthesis.
  ///
  /// ParenExpr ::= '(' Expression ')'
  static std::unique_ptr<ExprAST> parseParenExpr();

  /// Parse identifier expressions.
  ///
  /// IdentifierExpr
  ///   ::= Identifier
  ///   ::= Identifier '(' expression* ')'
  static std::unique_ptr<ExprAST> parseIdentifierExpr();

  /// Parse primary expressions.
  ///
  /// Primary
  ///   ::= IdentifierExpr
  ///   ::= NumberExpr
  ///   ::= ParenExpr
  static std::unique_ptr<ExprAST> parsePrimary();

  /// Parse primary binorph expressions.
  ///
  /// Expression
  ///   ::= Primary Binorphs
  static std::unique_ptr<ExprAST> parseExpression();

  /// Parse RHS with the given LHS for the binorph.
  ///
  /// Binorphs
  ///   ::= ('+' primary)*
  static std::unique_ptr<ExprAST> parseBinOpRHS(int ExprPrec,
                                                std::unique_ptr<ExprAST> LHS);

  /// Parse prototype expressions.
  ///
  /// ProtoType
  ///   ::= id '(' id* ')'
  static std::unique_ptr<ProtoTypeAST> parseProtoType();

  /// Parse definition for the prototype expression.
  ///
  /// Definition ::= 'def' PrototTypeExpr
  static std::unique_ptr<FunctionAST> parseDefinition();

  /// Parse external prototype expressions.
  ///
  /// External ::= 'extern' ProtoType
  static std::unique_ptr<ProtoTypeAST> parseExtern();

  /// Parse top level expressions.
  ///
  /// TopLevelExpr ::= Expression
  static std::unique_ptr<FunctionAST> parseTopLevelExpr();

  /// Helper function to handle prototype definitions.
  static void handleDefinition();

  /// Helper function to handle external prototype expressions.
  static void handleExtern();

  /// Helper function to handle top level expressions.
  static void handleTopLevelExpression();

public:
  /// Initialises the module.
  static void initialiseModuleAndPassManager();

  /// Parse the main loop for beginning the parsing pipeline.
  ///
  /// Top ::= Definition | External | Expression | ';'
  static void mainLoop();
};

/// Holds the precedence value for a valid binary operator.
class BinOpPrecedence {
  inline static std::map<char, int> precedence;

public:
  /// Get the precedence of the operator token.
  static int getTokPrecedence();
  /// Initialise standard binary operators.
  static void init();
};

#endif // KALEIDOSCOPE_PARSER_H
