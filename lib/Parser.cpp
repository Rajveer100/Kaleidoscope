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
#include "llvm/Support/raw_ostream.h"

std::unique_ptr<ExprAST> Parser::ParseNumberExpr() {
  auto Result = std::make_unique<NumberExprAST>(CurLexer.getNumVal());
  CurLexer.getNextTok(); // consume the number
  return std::move(Result);
}

std::unique_ptr<ExprAST> Parser::ParseParenExpr() {
  CurLexer.getNextTok(); // eat (.
  auto V = ParseExpression();
  if (!V)
    return nullptr;

  if (CurLexer.getCurTok() != ')')
    return Logger::LogError("expected ')'");
  CurLexer.getNextTok(); // eat ).
  return V;
}

std::unique_ptr<ExprAST> Parser::ParseIdentifierExpr() {
  std::string IdName = CurLexer.getIdentifierStr();

  CurLexer.getNextTok(); // eat identifier.

  if (CurLexer.getCurTok() != '(') // Simple variable reference.
    return std::make_unique<VariableExprAST>(IdName);

  // Call.
  CurLexer.getNextTok(); // eat (.
  std::vector<std::unique_ptr<ExprAST>> Args;
  if (CurLexer.getCurTok() != ')') {
    while (true) {
      if (auto Arg = ParseExpression())
        Args.push_back(std::move(Arg));
      else
        return nullptr;

      if (CurLexer.getCurTok() == ')')
        break;

      if (CurLexer.getCurTok() != ',')
        return Logger::LogError("Expected ')' or ',' in argument list");
      CurLexer.getNextTok();
    }
  }

  // eat ).
  CurLexer.getNextTok();

  return std::make_unique<CallExprAST>(IdName, std::move(Args));
}

std::unique_ptr<ExprAST> Parser::ParsePrimary() {
  switch (CurLexer.getCurTok()) {
  default:
    return Logger::LogError("Unknown token when expecting an expression");
  case TOK_IDENTIFIER:
    return ParseIdentifierExpr();
  case TOK_NUMBER:
    return ParseNumberExpr();
  case '(':
    return ParseParenExpr();
  case TOK_IF:
    return ParseIfExpr();
  case TOK_FOR:
    return ParseForExpr();
  }
}

std::unique_ptr<ExprAST> Parser::ParseExpression() {
  auto LHS = ParsePrimary();
  if (!LHS)
    return nullptr;
  return ParseBinOpRHS(0, std::move(LHS));
}

std::unique_ptr<ExprAST> Parser::ParseBinOpRHS(int ExprPrec,
                                               std::unique_ptr<ExprAST> LHS) {
  // If this is a binary operator, find its precedence.
  while (true) {
    int TokPrec = BinOpPrecedence.GetBinOpPrecedence(CurLexer);

    // If this binary operator that binds at least as tightly
    // as the current loop, consume it.
    if (TokPrec < ExprPrec)
      return LHS;

    // This is a binary operator.
    int BinOp = CurLexer.getCurTok();
    CurLexer.getNextTok(); // eat binary operator.

    // Parse the primary expression after the binary operator.
    auto RHS = ParsePrimary();
    if (!RHS)
      return nullptr;

    // If binary operator binds less tightly with RHS than
    // the operator after RHS, the pending operator will take
    // the RHS as its LHS.
    int NextPrec = BinOpPrecedence.GetBinOpPrecedence(CurLexer);
    if (TokPrec < NextPrec) {
      RHS = ParseBinOpRHS(TokPrec + 1, std::move(RHS));
      if (!RHS)
        return nullptr;
    }
    // Merge LHS/RHS.
    LHS =
        std::make_unique<BinaryExprAST>(BinOp, std::move(LHS), std::move(RHS));
  } // back to the while loop.
}

std::unique_ptr<ProtoTypeAST> Parser::ParseProtoType() {
  if (CurLexer.getCurTok() != TOK_IDENTIFIER)
    return Logger::LogErrorP("Expected function name in prototype");

  std::string FnName = CurLexer.getIdentifierStr();
  CurLexer.getNextTok();

  if (CurLexer.getCurTok() != '(')
    return Logger::LogErrorP("Expected '(' in prototype");

  // Read the argument list.
  std::vector<std::string> ArgNames;
  while (CurLexer.getNextTok() == TOK_IDENTIFIER)
    ArgNames.push_back(CurLexer.getIdentifierStr());
  if (CurLexer.getCurTok() != ')')
    return Logger::LogErrorP("Expected ')' in prototype");

  // done.
  CurLexer.getNextTok(); // eat ).

  return std::make_unique<ProtoTypeAST>(FnName, std::move(ArgNames));
}

std::unique_ptr<FunctionAST> Parser::ParseDefinition() {
  CurLexer.getNextTok(); // eat def.
  auto Proto = ParseProtoType();
  if (!Proto)
    return nullptr;

  if (auto E = ParseExpression())
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  return nullptr;
}

std::unique_ptr<ProtoTypeAST> Parser::ParseExtern() {
  CurLexer.getNextTok(); // eat extern.
  return ParseProtoType();
}

std::unique_ptr<FunctionAST> Parser::ParseTopLevelExpr() {
  if (auto E = ParseExpression()) {
    // Make anonymous Proto.
    auto Proto = std::make_unique<ProtoTypeAST>("__anon_expr",
                                                std::vector<std::string>());
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  }
  return nullptr;
}

std::unique_ptr<ExprAST> Parser::ParseIfExpr() {
  CurLexer.getNextTok(); // eat the if.

  // condition.
  auto Cond = ParseExpression();
  if (!Cond)
    return nullptr;

  if (CurLexer.getCurTok() != TOK_THEN)
    return Logger::LogError("expected then");
  CurLexer.getNextTok(); // eat the then.

  auto Then = ParseExpression();
  if (!Then)
    return nullptr;

  if (CurLexer.getCurTok() != TOK_ELSE)
    return Logger::LogError("expected else");
  CurLexer.getNextTok(); // eat the else.

  auto Else = ParseExpression();
  if (!Else)
    return nullptr;

  return std::make_unique<IfExprAST>(std::move(Cond), std::move(Then),
                                     std::move(Else));
}

std::unique_ptr<ExprAST> Parser::ParseForExpr() {
  CurLexer.getNextTok(); // eat the for.

  if (CurLexer.getCurTok() != TOK_IDENTIFIER)
    return Logger::LogError("expected identifier after for");

  std::string IdName = CurLexer.getIdentifierStr();
  CurLexer.getNextTok(); // eat identifier.

  if (CurLexer.getCurTok() != '=')
    return Logger::LogError("expected '=' after for");
  CurLexer.getNextTok(); // eat '='.

  auto Start = ParseExpression();
  if (!Start)
    return nullptr;
  if (CurLexer.getCurTok() != ',')
    return Logger::LogError("expected ',' after for start value");
  CurLexer.getNextTok();

  auto End = ParseExpression();
  if (!End)
    return nullptr;

  // The step value is optional.
  std::unique_ptr<ExprAST> Step;
  if (CurLexer.getCurTok() == ',') {
    CurLexer.getNextTok();
    Step = ParseExpression();
    if (!Step)
      return nullptr;
  }

  if (CurLexer.getCurTok() != TOK_IN)
    return Logger::LogError("expected 'in' after for");
  CurLexer.getNextTok(); // eat 'in'.

  auto Body = ParseExpression();
  if (!Body)
    return nullptr;

  return std::make_unique<ForExprAST>(IdName, std::move(Start), std::move(End),
                                      std::move(Step), std::move(Body));
}

void Parser::HandleDefinition() {
  if (auto FnAST = ParseDefinition()) {
    if (auto *FnIR = FnAST->codegen(CG)) {
      fprintf(stderr, "Read a function definition:\n");
      FnIR->print(llvm::errs());
      fprintf(stderr, "\n");

      CG.ExitOnError(CG.JIT->addModule(llvm::orc::ThreadSafeModule(
          std::move(CG.Module), std::move(CG.Context))));
      CG.InitialiseModuleAndPassManager();
    }
  } else {
    // Skip token for error recovery.
    CurLexer.getNextTok();
  }
}

void Parser::HandleExtern() {
  if (auto ProtoAST = ParseExtern()) {
    if (auto *FnIR = ProtoAST->codegen(CG)) {
      fprintf(stderr, "Read extern:\n");
      FnIR->print(llvm::errs());
      fprintf(stderr, "\n");
      CG.FunctionProtos[ProtoAST->getName()] = std::move(ProtoAST);
    }
  } else {
    // Skip token for error recovery.
    CurLexer.getNextTok();
  }
}

void Parser::HandleTopLevelExpression() {
  // Evaluate top-level expressions as an anonymous function.
  if (auto FnAST = ParseTopLevelExpr()) {
    if (auto FnIR = FnAST->codegen(CG)) {
      FnIR->print(llvm::errs());
      auto RT = CG.JIT->getMainJITDylib().createResourceTracker();

      auto TSM = llvm::orc::ThreadSafeModule(std::move(CG.Module),
                                             std::move(CG.Context));
      CG.ExitOnError(CG.JIT->addModule(std::move(TSM), RT));
      CG.InitialiseModuleAndPassManager();

      auto ExprSymbol = CG.ExitOnError(CG.JIT->lookup("__anon_expr"));

      double (*FP)() = ExprSymbol.getAddress().toPtr<double (*)()>();
      fprintf(stderr, "Evaluated to %f\n", FP());

      // Without JIT:
      //
      // fprintf(stderr, "Read top-level expression:\n");
      // FnIR->print(llvm::errs());
      // fprintf(stderr, "\n");

      // Remove the anonymous expression.
      // FnIR->eraseFromParent();

      CG.ExitOnError(RT->remove());
    }
  } else {
    // Skip token for error recovery.
    CurLexer.getNextTok();
  }
}

void Parser::MainLoop(Lexer &Lexer) {
  CurLexer = Lexer;
  while (true) {
    fprintf(stderr, "ready> ");
    switch (CurLexer.getCurTok()) {
    case TOK_EOF:
      return;
    case ';':
      CurLexer.getNextTok();
      break;
    case TOK_DEF:
      HandleDefinition();
      break;
    case TOK_EXTERN:
      HandleExtern();
      break;
    default:
      HandleTopLevelExpression();
      break;
    }
  }
}
