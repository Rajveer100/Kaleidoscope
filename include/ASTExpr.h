//===- ASTExpr.h - AST expressions base class ----------------------------===//
//
// Author: Rajveer <rajveer.developer@icloud.com>
//
//===----------------------------------------------------------------------===//
//
// Abstract Syntax Tree (AST) for expression nodes.
//
//===----------------------------------------------------------------------===//

#ifndef KALEIDOSCOPE_ASTEXPR_H
#define KALEIDOSCOPE_ASTEXPR_H

#include "llvm/IR/Value.h"
#include "llvm/IR/Verifier.h"
#include <string>
#include <vector>

class CodeGen;

/// ExprAST - Base class for all expression nodes.
class ExprAST {
public:
  virtual ~ExprAST() = default;
  virtual llvm::Value *codegen(CodeGen &CG) = 0;
};

/// NumberExprAST - Expression class for numeric literals.
class NumberExprAST : public ExprAST {
  double Val;

public:
  NumberExprAST(double Val) : Val(Val) {}
  llvm::Value *codegen(CodeGen &CG) override;
};

/// VariableExprAST - Expression class for referencing a variable.
class VariableExprAST : public ExprAST {
  std::string Name;

public:
  VariableExprAST(const std::string &Name) : Name(Name) {}
  llvm::Value *codegen(CodeGen &CG) override;
};

/// IfExprAST - This class represents an expression for if/then/else.
class IfExprAST : public ExprAST {
  std::unique_ptr<ExprAST> Cond, Then, Else;

public:
  IfExprAST(std::unique_ptr<ExprAST> Cond, std::unique_ptr<ExprAST> Then,
            std::unique_ptr<ExprAST> Else)
      : Cond(std::move(Cond)), Then(std::move(Then)), Else(std::move(Else)) {}

  llvm::Value *codegen(CodeGen &CG) override;
};

/// ForExprAST - Expression class for for/in.
class ForExprAST : public ExprAST {
  std::string VarName;
  std::unique_ptr<ExprAST> Start, End, Step, Body;

public:
  ForExprAST(const std::string &VarName, std::unique_ptr<ExprAST> Start,
             std::unique_ptr<ExprAST> End, std::unique_ptr<ExprAST> Step,
             std::unique_ptr<ExprAST> Body)
      : VarName(VarName), Start(std::move(Start)), End(std::move(End)),
        Step(std::move(Step)), Body(std::move(Body)) {}

  llvm::Value *codegen(CodeGen &CG) override;
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
  char Op;
  std::unique_ptr<ExprAST> LHS, RHS;

public:
  BinaryExprAST(char Op, std::unique_ptr<ExprAST> LHS,
                std::unique_ptr<ExprAST> RHS)
      : Op(Op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
  llvm::Value *codegen(CodeGen &CG) override;
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
  std::string Callee;
  std::vector<std::unique_ptr<ExprAST>> Args;

public:
  CallExprAST(const std::string &Callee,
              std::vector<std::unique_ptr<ExprAST>> Args)
      : Callee(Callee), Args(std::move(Args)) {}
  llvm::Value *codegen(CodeGen &CG) override;
};

/// ProtoTypeAST - This class represents the "prototype" for a function,
/// which captures its name and its argument names.
class ProtoTypeAST {
  std::string Name;
  std::vector<std::string> Args;

public:
  ProtoTypeAST(const std::string &Name, std::vector<std::string> Args)
      : Name(Name), Args(std::move(Args)) {}

  /// Get the prototype name.
  const std::string &getName() const;
  llvm::Function *codegen(CodeGen &CG);
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST {
  std::unique_ptr<ProtoTypeAST> Proto;
  std::unique_ptr<ExprAST> Body;

public:
  FunctionAST(std::unique_ptr<ProtoTypeAST> Proto,
              std::unique_ptr<ExprAST> Body)
      : Proto(std::move(Proto)), Body(std::move(Body)) {}
  llvm::Function *codegen(CodeGen &CG);
};

#endif // KALEIDOSCOPE_ASTEXPR_H
