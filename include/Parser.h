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
#include "CodeGen.h"
#include "Lexer.h"
#include "Logger.h"
#include <map>

/// The parser starts with the most simple literal,
/// which are then used by compound literals to break down
/// each production in the grammar.
class Parser {
public:
  /// Parse numerical expressions.
  ///
  /// NumberExpr ::= Number
  std::unique_ptr<ExprAST> ParseNumberExpr();

  /// Parse expressions with parenthesis.
  ///
  /// ParenExpr ::= '(' Expression ')'
  std::unique_ptr<ExprAST> ParseParenExpr();

  /// Parse identifier expressions.
  ///
  /// IdentifierExpr
  ///   ::= Identifier
  ///   ::= Identifier '(' expression* ')'
  std::unique_ptr<ExprAST> ParseIdentifierExpr();

  /// Parse primary expressions.
  ///
  /// Primary
  ///   ::= IdentifierExpr
  ///   ::= NumberExpr
  ///   ::= ParenExpr
  std::unique_ptr<ExprAST> ParsePrimary();

  /// Parse primary binorph expressions.
  ///
  /// Expression
  ///   ::= Primary Binorphs
  std::unique_ptr<ExprAST> ParseExpression();

  /// Parse RHS with the given LHS for the binorph.
  ///
  /// Binorphs
  ///   ::= ('+' primary)*
  std::unique_ptr<ExprAST> ParseBinOpRHS(int ExprPrec,
                                         std::unique_ptr<ExprAST> LHS);

  /// Parse prototype expressions.
  ///
  /// ProtoType
  ///   ::= id '(' id* ')'
  std::unique_ptr<ProtoTypeAST> ParseProtoType();

  /// Parse definition for the prototype expression.
  ///
  /// Definition ::= 'def' PrototTypeExpr
  std::unique_ptr<FunctionAST> ParseDefinition();

  /// Parse external prototype expressions.
  ///
  /// External ::= 'extern' ProtoType
  std::unique_ptr<ProtoTypeAST> ParseExtern();

  /// Parse top level expressions.
  ///
  /// TopLevelExpr ::= Expression
  std::unique_ptr<FunctionAST> ParseTopLevelExpr();

  /// Parse if/then/else expressions.
  ///
  /// IfExpr ::= 'if' expression 'then' expression 'else' expression
  std::unique_ptr<ExprAST> ParseIfExpr();

  /// Parse for/in expressions.
  ///
  /// ForExpr ::= 'for' identifier '=' expr ',' expr (',' expr)? 'in' expression
  std::unique_ptr<ExprAST> ParseForExpr();

  /// Helper function to handle prototype definitions.
  void HandleDefinition();

  /// Helper function to handle external prototype expressions.
  void HandleExtern();

  /// Helper function to handle top level expressions.
  void HandleTopLevelExpression();

private:
  /// Holds the precedence value for a valid binary operator.
  class OpPrecedence {
    std::map<char, int> BinOpPrecedence;

  public:
    /// Get the precedence of the operator token.
    int GetBinOpPrecedence(Lexer &CurLexer) {
      int curToken = CurLexer.getCurTok();
      if (!isascii(curToken))
        return -1;

      // Check if the Op is declared.
      int TokPrec = BinOpPrecedence[curToken];
      if (TokPrec <= 0)
        return -1;
      return TokPrec;
    }

    /// Initialise standard binary operators.
    OpPrecedence() {
      BinOpPrecedence['<'] = 10;
      BinOpPrecedence['+'] = 20;
      BinOpPrecedence['-'] = 30;
      BinOpPrecedence['*'] = 40; // highest
    }
  };

public:
  Lexer CurLexer;

  /// Parse the main loop for beginning the parsing pipeline.
  ///
  /// Top ::= Definition | External | Expression | ';'
  void MainLoop(Lexer &Lexer);

  CodeGen CG;

  OpPrecedence BinOpPrecedence;

  Parser() { CG.InitialiseModuleAndPassManager(); }
};

#endif // KALEIDOSCOPE_PARSER_H
