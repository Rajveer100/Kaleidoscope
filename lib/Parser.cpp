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
#include "ASTExpr.h"
#include "CodeGen.h"
#include "Lexer.h"
#include "Logger.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/ExecutionEngine/Orc/ThreadSafeModule.h"
#include "llvm/IR/PassInstrumentation.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Scalar/Reassociate.h"
#include "llvm/Transforms/Scalar/SimplifyCFG.h"

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
  case TOK_IF:
    return parseIfExpr();
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
    auto Proto = std::make_unique<ProtoTypeAST>("__anon_expr",
                                                std::vector<std::string>());
    return std::make_unique<FunctionAST>(std::move(Proto), std::move(E));
  }
  return nullptr;
}

std::unique_ptr<ExprAST> Parser::parseIfExpr() {
  Lexer::getNextToken(); // eat the if.

  // condition.
  auto Cond = parseExpression();
  if (!Cond)
    return nullptr;

  if (Lexer::CurTok != TOK_THEN)
    return Logger::LogError("expected then");
  Lexer::getNextToken(); // eat the then.

  auto Then = parseExpression();
  if (!Then)
    return nullptr;

  if (Lexer::CurTok != TOK_ELSE)
    return Logger::LogError("expected else");
  Lexer::getNextToken(); // eat the else.

  auto Else = parseExpression();
  if (!Else)
    return nullptr;

  return std::make_unique<IfExprAST>(std::move(Cond), std::move(Then),
                                     std::move(Else));
}

void Parser::handleDefinition() {
  if (auto FnAST = parseDefinition()) {
    if (auto *FnIR = FnAST->codegen()) {
      fprintf(stderr, "Read a function definition:\n");
      FnIR->print(llvm::errs());
      fprintf(stderr, "\n");

      CodeGen::ExitOnError(CodeGen::JIT->addModule(llvm::orc::ThreadSafeModule(
          std::move(CodeGen::Module), std::move(CodeGen::Context))));
      initialiseModuleAndPassManager();
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
      CodeGen::FunctionProtos[ProtoAST->getName()] = std::move(ProtoAST);
    }
  } else {
    // Skip token for error recovery.
    Lexer::getNextToken();
  }
}

void Parser::handleTopLevelExpression() {
  // Evaluate top-level expressions as an anonymous function.
  if (auto FnAST = parseTopLevelExpr()) {
    if (FnAST->codegen()) {
      auto RT = CodeGen::JIT->getMainJITDylib().createResourceTracker();

      auto TSM = llvm::orc::ThreadSafeModule(std::move(CodeGen::Module),
                                             std::move(CodeGen::Context));
      CodeGen::ExitOnError(CodeGen::JIT->addModule(std::move(TSM), RT));
      initialiseModuleAndPassManager();

      auto ExprSymbol =
          CodeGen::ExitOnError(CodeGen::JIT->lookup("__anon_expr"));

      double (*FP)() = ExprSymbol.getAddress().toPtr<double (*)()>();
      fprintf(stderr, "Evaluated to %f\n", FP());

      // Without JIT:
      //
      // fprintf(stderr, "Read top-level expression:\n");
      // FnIR->print(llvm::errs());
      // fprintf(stderr, "\n");

      // Remove the anonymous expression.
      // FnIR->eraseFromParent();

      CodeGen::ExitOnError(RT->remove());
    }
  } else {
    // Skip token for error recovery.
    Lexer::getNextToken();
  }
}

void Parser::initialiseModuleAndPassManager() {
  // Open a new context and module.
  CodeGen::Context = std::make_unique<llvm::LLVMContext>();
  CodeGen::Module =
      std::make_unique<llvm::Module>("KaleidoscopeJIT", *CodeGen::Context);
  CodeGen::Module->setDataLayout(CodeGen::JIT->getDataLayout());

  // Create a new builder for the module.
  CodeGen::Builder = std::make_unique<llvm::IRBuilder<>>(*CodeGen::Context);

  // Create new pass and analysis managers.
  CodeGen::FPM = std::make_unique<llvm::FunctionPassManager>();
  CodeGen::LAM = std::make_unique<llvm::LoopAnalysisManager>();
  CodeGen::FAM = std::make_unique<llvm::FunctionAnalysisManager>();
  CodeGen::CGAM = std::make_unique<llvm::CGSCCAnalysisManager>();
  CodeGen::MAM = std::make_unique<llvm::ModuleAnalysisManager>();
  CodeGen::PIC = std::make_unique<llvm::PassInstrumentationCallbacks>();
  CodeGen::SI = std::make_unique<llvm::StandardInstrumentations>(
      *CodeGen::Context, /*DebugLogging*/ true);

  CodeGen::SI->registerCallbacks(*CodeGen::PIC, CodeGen::MAM.get());

  // Transform passes for simple 'peephole' and bit-twiddling optimizations.
  CodeGen::FPM->addPass(llvm::InstCombinePass());
  // Reassociate expressions.
  CodeGen::FPM->addPass(llvm::ReassociatePass());
  // Eliminate common sub-expressions.
  CodeGen::FPM->addPass(llvm::GVNPass());
  // Simplify the control flow graph (ex: deleting unreachable blocks, ...)
  CodeGen::FPM->addPass(llvm::SimplifyCFGPass());

  // Register analysis passes used in these transform passes.
  llvm::PassBuilder PB;
  PB.registerModuleAnalyses(*CodeGen::MAM);
  PB.registerFunctionAnalyses(*CodeGen::FAM);
  PB.crossRegisterProxies(*CodeGen::LAM, *CodeGen::FAM, *CodeGen::CGAM,
                          *CodeGen::MAM);
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
