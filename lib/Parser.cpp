//===- Parser.cpp - Parser support code -----------------------------------===//
//
// Author: Rajveer <rajveer.developer@icloud.com>
//
//===----------------------------------------------------------------------===//
//
// Implements parsing functionalities for the language.
//
//===----------------------------------------------------------------------===//

#include "Parser.h"
#include "CodeGen.h"
#include "Lexer.h"

std::unique_ptr<ExprAST> Parser::parseNumberExpr() {
  auto Result = std::make_unique<NumberExprAST>(Lexer::NumVal);
  Lexer::getNextToken(); // consume the number
  return std::move(Result);
}

std::unique_ptr<ExprAST> Parser::parseParenExpr() {
  Lexer::getNextToken(); // eat (.
  auto V = Parser::parseExpression();
  if (!V)
    return nullptr;

  if (Lexer::CurTok != ')')
    return Logger::LogError("expected ')'");
  Lexer::getNextToken(); // eat ).
  return V;
}

std::unique_ptr<ExprAST> Parser::parseIdentifierExpr() {
  std::string IdName = Lexer::IdentifierStr;

  Lexer::getNextToken(); // eat identifier.

  if (Lexer::CurTok != '(') // Simple variable reference.
    return std::make_unique<VariableExprAST>(IdName);

  // Call.
  Lexer::getNextToken(); // eat (.
  std::vector<std::unique_ptr<ExprAST>> Args;
  if (Lexer::CurTok != ')') {
    while (true) {
      if (auto Arg = Parser::parseExpression())
        Args.push_back(std::move(Arg));
      else
        return nullptr;

      if (Lexer::CurTok == ')')
        break;

      if (Lexer::CurTok != ',')
        return Logger::LogError("Expected ')' or ',' in argument list");
      Lexer::getNextToken();
    }
  }

  // eat ).
  Lexer::getNextToken();

  return std::make_unique<CallExprAST>(IdName, std::move(Args));
}

std::unique_ptr<ExprAST> Parser::parsePrimary() {
  switch (Lexer::CurTok) {
  default:
    return Logger::LogError("Unknown token when expecting an expression");
  case TOK_IDENTIFIER:
    return parseIdentifierExpr();
  case TOK_NUMBER:
    return parseNumberExpr();
  case '(':
    return parseParenExpr();
  }
}

std::unique_ptr<ExprAST> Parser::parseExpression() {
  auto LHS = parsePrimary();
  if (!LHS)
    return nullptr;
  return parseBinOpRHS(0, std::move(LHS));
}

std::unique_ptr<ExprAST> Parser::parseBinOpRHS(int ExprPrec,
                                               std::unique_ptr<ExprAST> LHS) {
  // If this is a binary operator, find its precedence.
  while (true) {
    int TokPrec = BinOpPrecedence::getTokPrecedence();

    // If this binary operator that binds at least as tightly
    // as the current loop, consume it.
    if (TokPrec < ExprPrec)
      return LHS;

    // This is a binary operator.
    int BinOp = Lexer::CurTok;
    Lexer::getNextToken(); // eat binary operator.

    // Parse the primary expression after the binary operator.
    auto RHS = parsePrimary();
    if (!RHS)
      return nullptr;

    // If binary operator binds less tightly with RHS than
    // the operator after RHS, the pending operator will take
    // the RHS as its LHS.
    int NextPrec = BinOpPrecedence::getTokPrecedence();
    if (TokPrec < NextPrec) {
      RHS = parseBinOpRHS(TokPrec + 1, std::move(RHS));
      if (!RHS)
        return nullptr;
    }
    // Merge LHS/RHS.
    LHS =
        std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
  } // back to the while loop.
}

std::unique_ptr<ProtoTypeAST> Parser::parseProtoType() {
  if (Lexer::CurTok != TOK_IDENTIFIER)
    return Logger::LogErrorP("Expected function name in prototype");

  std::string FnName = Lexer::IdentifierStr;
  Lexer::getNextToken();

  if (Lexer::CurTok != '(')
    return Logger::LogErrorP("Expected '(' in prototype");

  // Read the argument list.
  std::vector<std::string> ArgNames;
  while (Lexer::getNextToken() == TOK_IDENTIFIER)
    ArgNames.push_back(Lexer::IdentifierStr);
  if (Lexer::CurTok != ')')
    return Logger::LogErrorP("Expected ')' in prototype");

  // done.
  Lexer::getNextToken(); // eat ).

  return std::make_unique<ProtoTypeAST>(FnName, std::move(ArgNames));
}

std::unique_ptr<FunctionAST> Parser::parseDefinition() {
  Lexer::getNextToken(); // eat def.
  auto Proto = parseProtoType();
  if (!Proto)
    return nullptr;

  if (auto E = parseExpression())
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  return nullptr;
}

std::unique_ptr<ProtoTypeAST> Parser::parseExtern() {
  Lexer::getNextToken(); // eat extern.
  return parseProtoType();
}

std::unique_ptr<FunctionAST> Parser::parseTopLevelExpr() {
  if (auto E = parseExpression()) {
    // Make anonymous Proto.
    auto Proto = std::make_unique<ProtoTypeAST>("", std::vector<std::string>());
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  }
  return nullptr;
}

void Parser::handleDefinition() {
  if (auto FnAST = parseDefinition()) {
    if (auto FnIR = FnAST->codegen()) {
      fprintf(stderr, "Read a function definition:\n");
      FnIR->print(llvm::errs());
      fprintf(stderr, "\n");
    }
  } else {
    // Skip token for error recovery.
    Lexer::getNextToken();
  }
}

void Parser::handleExtern() {
  if (auto ProtoAST = parseExtern()) {
    if (auto *FnIR = ProtoAST->codegen()) {
      fprintf(stderr, "Read extern:\n");
      FnIR->print(llvm::errs());
      fprintf(stderr, "\n");
    }
  } else {
    // Skip token for error recovery.
    Lexer::getNextToken();
  }
}

void Parser::handleTopLevelExpression() {
  // Evaluate top-level expressions as an anonymous function.
  if (auto FnAST = parseTopLevelExpr()) {
    if (auto *FnIR = FnAST->codegen()) {
      fprintf(stderr, "Read top-level expression:\n");
      FnIR->print(llvm::errs());
      fprintf(stderr, "\n");

      // Remove the anonymous expression.
      FnIR->eraseFromParent();
    }
  } else {
    // Skip token for error recovery.
    Lexer::getNextToken();
  }
}

void Parser::initialiseModule() {
  // Open a new context and module.
  CodeGen::Context = std::make_unique<llvm::LLVMContext>();
  CodeGen::Module =
      std::make_unique<llvm::Module>("KaleidoscopeJIT", *CodeGen::Context);

  // Create a new builder for the module.
  CodeGen::Builder = std::make_unique<llvm::IRBuilder<>>(*CodeGen::Context);
}

int BinOpPrecedence::getTokPrecedence() {
  if (!isascii(Lexer::CurTok))
    return -1;

  // Check if the Op is declared.
  int TokPrec = precedence[Lexer::CurTok];
  if (TokPrec <= 0)
    return -1;
  return TokPrec;
}

void BinOpPrecedence::init() {
  precedence['<'] = 10;
  precedence['+'] = 20;
  precedence['-'] = 30;
  precedence['*'] = 40; // highest
}

void Parser::mainLoop() {
  while (true) {
    fprintf(stderr, "ready> ");
    switch (Lexer::CurTok) {
    case TOK_EOF:
      return;
    case ';':
      Lexer::getNextToken();
      break;
    case TOK_DEF:
      handleDefinition();
      break;
    case TOK_EXTERN:
      handleExtern();
      break;
    default:
      handleTopLevelExpression();
      break;
    }
  }
}
