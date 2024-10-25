//===- ASTExpr.cpp - AST expressions support code. ------------------------===//
//
// Author: Rajveer <rajveer.developer@icloud.com>
//
//===----------------------------------------------------------------------===//
//
// Implements AST expressions for the nodes.
//
//===----------------------------------------------------------------------===//

#include "ASTExpr.h"
#include "CodeGen.h"
#include "Logger.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"

llvm::Value *NumberExprAST::codegen() {
  return llvm::ConstantFP::get(*CodeGen::Context, llvm::APFloat(Val));
}

llvm::Value *VariableExprAST::codegen() {
  // Lookup variable in the function.
  llvm::Value *V = CodeGen::NamedValues[Name];
  if (!V)
    return Logger::LogErrorV("Unknown variable name");
  return V;
}

llvm::Value *BinaryExprAST::codegen() {
  llvm::Value *L = LHS->codegen();
  llvm::Value *R = RHS->codegen();
  if (!L || !R)
    return nullptr;

  switch (Op) {
  case '+':
    return CodeGen::Builder->CreateFAdd(L, R, "addtmp");
  case '-':
    return CodeGen::Builder->CreateFSub(L, R, "subtmp");
  case '*':
    return CodeGen::Builder->CreateFMul(L, R, "multmp");
  case '<':
    L = CodeGen::Builder->CreateFCmpULE(L, R, "cmptmp");
    // Convert bool 0/1 to double 0.0/1.0
    return CodeGen::Builder->CreateUIToFP(
        L, llvm::Type::getDoubleTy(*CodeGen::Context), "booltmp");
  default:
    return Logger::LogErrorV("invalid binary operator");
  }
}

llvm::Function *getFunction(std::string Name) {
  // Check if the function has already been added to the current module.
  if (auto *F = CodeGen::Module->getFunction(Name))
    return F;

  // If not, check if there is an existing prototype.
  auto FI = CodeGen::FunctionProtos.find(Name);
  if (FI != CodeGen::FunctionProtos.end())
    return FI->second->codegen();

  return nullptr;
}

llvm::Value *CallExprAST::codegen() {
  // Look up name of function in global module table.
  llvm::Function *CalleeF = getFunction(Callee);
  if (!CalleeF)
    return Logger::LogErrorV("Unknown function referenced");

  // If argument mismatch error.
  if (CalleeF->arg_size() != Args.size())
    return Logger::LogErrorV("Incorrect # arguments passed");

  std::vector<llvm::Value *> ArgsV;
  for (unsigned i = 0, e = Args.size(); i != e; ++i) {
    ArgsV.push_back(Args[i]->codegen());
    if (!ArgsV.back())
      return nullptr;
  }

  return CodeGen::Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

const std::string &ProtoTypeAST::getName() const { return Name; }

llvm::Function *ProtoTypeAST::codegen() {
  // Make the function type: double(double, double) etc.
  std::vector<llvm::Type *> Doubles(Args.size(),
                                    llvm::Type::getDoubleTy(*CodeGen::Context));
  llvm::FunctionType *FT = llvm::FunctionType::get(
      llvm::Type::getDoubleTy(*CodeGen::Context), Doubles, false);

  llvm::Function *F = llvm::Function::Create(
      FT, llvm::Function::ExternalLinkage, Name, CodeGen::Module.get());

  // Set names for all arguments.
  unsigned Idx = 0;
  for (auto &Arg : F->args())
    Arg.setName(Args[Idx++]);

  return F;
}

llvm::Function *FunctionAST::codegen() {
  auto &P = *Proto;
  CodeGen::FunctionProtos[Proto->getName()] = std::move(Proto);
  llvm::Function *Function = getFunction(P.getName());

  if (!Function)
    return nullptr;

  if (!Function->empty())
    return (llvm::Function *)Logger::LogErrorV("Function cannot be redefined");

  // Create a new basic block to start insertion into.
  llvm::BasicBlock *BB =
      llvm::BasicBlock::Create(*CodeGen::Context, "entry", Function);
  CodeGen::Builder->SetInsertPoint(BB);

  // Record the function arguments in the NamedValues map.
  CodeGen::NamedValues.clear();
  for (auto &Arg : Function->args())
    CodeGen::NamedValues[std::string(Arg.getName())] = &Arg;

  if (llvm::Value *RetVal = Body->codegen()) {
    // Finish off the function.
    CodeGen::Builder->CreateRet(RetVal);

    // Validate the generated code, checking for consistency.
    llvm::verifyFunction(*Function);

    // Optimize the function.
    CodeGen::FPM->run(*Function, *CodeGen::FAM);

    return Function;
  }

  // Error reading body.
  Function->eraseFromParent();
  return nullptr;
}
