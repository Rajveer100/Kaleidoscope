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
#include "llvm/ADT/APFloat.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Value.h"

llvm::Value *NumberExprAST::codegen(CodeGen &CG) {
  return llvm::ConstantFP::get(*CG.Context, llvm::APFloat(Val));
}

llvm::Value *VariableExprAST::codegen(CodeGen &CG) {
  // Lookup variable in the function.
  llvm::Value *V = CG.NamedValues[Name];
  if (!V)
    return Logger::LogErrorV("Unknown variable name");
  return V;
}

llvm::Value *IfExprAST::codegen(CodeGen &CG) {
  llvm::Value *CondV = Cond->codegen(CG);
  if (!CondV)
    return nullptr;

  // Convert condition to a bool by comparing non-equal to 0.0.
  CondV = CG.Builder->CreateFCmpONE(
      CondV, llvm::ConstantFP::get(*CG.Context, llvm::APFloat(0.0)), "ifcond");
  llvm::Function *Function = CG.Builder->GetInsertBlock()->getParent();

  // Create blocks for the then and else cases. Insert the 'then' block at the
  // end of the function.
  llvm::BasicBlock *ThenBB =
      llvm::BasicBlock::Create(*CG.Context, "then", Function);
  llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(*CG.Context, "else");
  llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(*CG.Context, "ifcont");

  CG.Builder->CreateCondBr(CondV, ThenBB, ElseBB);

  // Emit then value.
  CG.Builder->SetInsertPoint(ThenBB);

  llvm::Value *ThenV = Then->codegen(CG);
  if (!ThenV)
    return nullptr;

  CG.Builder->CreateBr(MergeBB);
  // Codegen of 'Then' can change the current block, update the ThenBB for the
  // PHI.
  ThenBB = CG.Builder->GetInsertBlock();

  Function->insert(Function->end(), ElseBB);
  CG.Builder->SetInsertPoint(ElseBB);

  llvm::Value *ElseV = Else->codegen(CG);
  if (!ElseV)
    return nullptr;

  CG.Builder->CreateBr(MergeBB);
  // Codegen of 'Else' can change the current block, update the ElseBB for the
  // PHI.
  ElseBB = CG.Builder->GetInsertBlock();

  // Emit merge block.
  Function->insert(Function->end(), MergeBB);
  CG.Builder->SetInsertPoint(MergeBB);
  llvm::PHINode *PN =
      CG.Builder->CreatePHI(llvm::Type::getDoubleTy(*CG.Context), 2, "iftmp");

  PN->addIncoming(ThenV, ThenBB);
  PN->addIncoming(ElseV, ElseBB);
  return PN;
}

llvm::Value *ForExprAST::codegen(CodeGen &CG) {
  // Emit the start code first, without 'variable' in scope.
  llvm::Value *StartV = Start->codegen(CG);
  if (!StartV)
    return nullptr;

  // Make the new basic block for the loop header, inserting after current
  // block.
  llvm::Function *Function = CG.Builder->GetInsertBlock()->getParent();
  llvm::BasicBlock *PreheaderBB = CG.Builder->GetInsertBlock();
  llvm::BasicBlock *LoopBB =
      llvm::BasicBlock::Create(*CG.Context, "loop", Function);

  // Explicit fall through from the current block to the LoopBB.
  CG.Builder->CreateBr(LoopBB);

  // Start insertion in LoopBB.
  CG.Builder->SetInsertPoint(LoopBB);

  // PHI node with an entry for Start.
  llvm::PHINode *Variable =
      CG.Builder->CreatePHI(llvm::Type::getDoubleTy(*CG.Context), 2, VarName);
  Variable->addIncoming(StartV, PreheaderBB);

  // Restore any shadowed existing variable within the loop.
  llvm::Value *OldVal = CG.NamedValues[VarName];
  CG.NamedValues[VarName] = Variable;

  // Emit body of the loop, ignoring value computed by it.
  if (!Body->codegen(CG))
    return nullptr;

  // Emit the step value.
  llvm::Value *StepV = nullptr;
  if (Step) {
    StepV = Step->codegen(CG);
    if (StepV)
      return nullptr;
  } else {
    // Default to using 1.0.
    StepV = llvm::ConstantFP::get(*CG.Context, llvm::APFloat(1.0));
  }

  llvm::Value *NextVar = CG.Builder->CreateFAdd(Variable, StepV, "nextvar");

  // Compute the end condition.
  llvm::Value *EndCond = End->codegen(CG);
  if (!EndCond)
    return nullptr;

  // Convert condition to a bool by comparing non-equal to 0.0.
  EndCond = CG.Builder->CreateFCmpONE(
      EndCond, llvm::ConstantFP::get(*CG.Context, llvm::APFloat(0.0)),
      "loopcond");

  // Create the "after loop" block and insert it.
  llvm::BasicBlock *LoopEndBB = CG.Builder->GetInsertBlock();
  llvm::BasicBlock *AfterBB =
      llvm::BasicBlock::Create(*CG.Context, "afterloop", Function);

  // Insert conditional branch into the end of LoopEndBB.
  CG.Builder->CreateCondBr(EndCond, LoopBB, AfterBB);

  // Any new code will be inserted in AfterBB.
  CG.Builder->SetInsertPoint(AfterBB);

  // Add a new entry to the PHI node for the backedge.
  Variable->addIncoming(NextVar, LoopEndBB);

  // Restore the unshadowed variable.
  if (OldVal)
    CG.NamedValues[VarName] = OldVal;
  else
    CG.NamedValues.erase(VarName);

  // for expr always returns 0.0.
  return llvm::Constant::getNullValue(llvm::Type::getDoubleTy((*CG.Context)));
}

llvm::Value *BinaryExprAST::codegen(CodeGen &CG) {
  llvm::Value *L = LHS->codegen(CG);
  llvm::Value *R = RHS->codegen(CG);
  if (!L || !R)
    return nullptr;

  switch (Op) {
  case '+':
    return CG.Builder->CreateFAdd(L, R, "addtmp");
  case '-':
    return CG.Builder->CreateFSub(L, R, "subtmp");
  case '*':
    return CG.Builder->CreateFMul(L, R, "multmp");
  case '<':
    L = CG.Builder->CreateFCmpULT(L, R, "cmptmp");
    // Convert bool 0/1 to double 0.0/1.0
    return CG.Builder->CreateUIToFP(L, llvm::Type::getDoubleTy(*CG.Context),
                                    "booltmp");
  default:
    return Logger::LogErrorV("invalid binary operator");
  }
}

llvm::Function *getFunction(CodeGen &CG, std::string Name) {
  // Check if the function has already been added to the current module.
  if (auto *F = CG.Module->getFunction(Name))
    return F;

  // If not, check if there is an existing prototype.
  auto FI = CG.FunctionProtos.find(Name);
  if (FI != CG.FunctionProtos.end())
    return FI->second->codegen(CG);

  return nullptr;
}

llvm::Value *CallExprAST::codegen(CodeGen &CG) {
  // Look up name of function in global module table.
  llvm::Function *CalleeF = getFunction(CG, Callee);
  if (!CalleeF)
    return Logger::LogErrorV("Unknown function referenced");

  // If argument mismatch error.
  if (CalleeF->arg_size() != Args.size())
    return Logger::LogErrorV("Incorrect # arguments passed");

  std::vector<llvm::Value *> ArgsV;
  for (unsigned i = 0, e = Args.size(); i != e; ++i) {
    ArgsV.push_back(Args[i]->codegen(CG));
    if (!ArgsV.back())
      return nullptr;
  }

  return CG.Builder->CreateCall(CalleeF, ArgsV, "calltmp");
}

const std::string &ProtoTypeAST::getName() const { return Name; }

llvm::Function *ProtoTypeAST::codegen(CodeGen &CG) {
  // Make the function type: double(double, double) etc.
  std::vector<llvm::Type *> Doubles(Args.size(),
                                    llvm::Type::getDoubleTy(*CG.Context));
  llvm::FunctionType *FT = llvm::FunctionType::get(
      llvm::Type::getDoubleTy(*CG.Context), Doubles, false);

  llvm::Function *F = llvm::Function::Create(
      FT, llvm::Function::ExternalLinkage, Name, CG.Module.get());

  // Set names for all arguments.
  unsigned Idx = 0;
  for (auto &Arg : F->args())
    Arg.setName(Args[Idx++]);

  return F;
}

llvm::Function *FunctionAST::codegen(CodeGen &CG) {
  auto &P = *Proto;
  CG.FunctionProtos[Proto->getName()] = std::move(Proto);
  llvm::Function *Function = getFunction(CG, P.getName());

  if (!Function)
    return nullptr;

  if (!Function->empty())
    return (llvm::Function *)Logger::LogErrorV("Function cannot be redefined");

  // Create a new basic block to start insertion into.
  llvm::BasicBlock *BB =
      llvm::BasicBlock::Create(*CG.Context, "entry", Function);
  CG.Builder->SetInsertPoint(BB);

  // Record the function arguments in the NamedValues map.
  CG.NamedValues.clear();
  for (auto &Arg : Function->args())
    CG.NamedValues[std::string(Arg.getName())] = &Arg;

  if (llvm::Value *RetVal = Body->codegen(CG)) {
    // Finish off the function.
    CG.Builder->CreateRet(RetVal);

    // Validate the generated code, checking for consistency.
    llvm::verifyFunction(*Function);

    // Optimize the function.
    CG.FPM->run(*Function, *CG.FAM);

    return Function;
  }

  // Error reading body.
  Function->eraseFromParent();
  return nullptr;
}
