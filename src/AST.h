#pragma once

#include <iostream>

static int count_block = 0;
static int count_var = 0;
/**
 * count_var给我新建的变量计数并命名
 *  - 相应的非终结符AST中包含自己的名字var_n就是用这个命名
 *  - 如果那个符号不需要名字，比如是个常数，那var_n = -1
 */

// 所有 AST 的基类
class BaseAST
{
public:
  int name;
  int val;
  virtual ~BaseAST() = default;

  virtual void Dump() const = 0;

  virtual void printIR(std::string &out) = 0;
};

// CompUnit
class CompUnitAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> func_def;

  void Dump() const override
  {
    std::cout << "CompUnitAST { ";
    func_def->Dump();
    std::cout << " }" << std::endl;
  }

  void printIR(std::string &out) override
  {
    func_def->printIR(out);
  }
};

// FuncDef
class FuncDefAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> func_type;
  std::string ident;
  std::unique_ptr<BaseAST> block;

  void Dump() const override
  {
    std::cout << "FuncDefAST { ";
    func_type->Dump();
    std::cout << ", " << ident << ", ";
    block->Dump();
    std::cout << " }";
  }

  void printIR(std::string &out) override
  {
    out += "fun @";
    out += ident;
    out += "():";
    func_type->printIR(out);
    out += "{\n";
    block->printIR(out);
    out += " } ";
  }
};

// FuncType
class FuncTypeAST : public BaseAST
{
public:
  // level 1:只是个int，没什么用

  void Dump() const override
  {
    std::cout << "FuncTypeAST { int }";
  }

  void printIR(std::string &out) override
  {
    out += " i32 ";
  }
};

// Block
class BlockAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> stmt;

  void Dump() const override
  {
    std::cout << "BlockAST { ";
    stmt->Dump();
    std::cout << " }";
  }

  void printIR(std::string &out) override
  {
    out += "%block_";
    out += std::to_string(count_block);
    out += ":\n";
    count_block++;
    stmt->printIR(out);
  }
};

// Stmt - return exp;
class StmtAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    std::cout << "StmtAST { ";
    exp->Dump();
    std::cout << " }";
  }

  void printIR(std::string &out) override
  {
    exp->printIR(out);
    if (exp->name == -1)
    {
      out += "ret ";
      out += (std::to_string(exp->val)).c_str();
      out += "\n";
    }
    else
    {
      out += "ret %";
      out += (std::to_string(exp->name)).c_str();
      out += "\n";
    }
  }
};

// Exp - unary_exp
class ExpAST : public BaseAST
{
public:
  std::unique_ptr<BaseAST> unary_exp;

  void Dump() const override
  {
    unary_exp->Dump();
  }
  void printIR(std::string &out) override
  {
    unary_exp->printIR(out);
    if (unary_exp->name == -1)
    {
      name = -1;
      val = unary_exp->val;
    }
    else
    {
      name = unary_exp->name;
    }
  }
};

// PrimaryExp - (exp)
class PrimaryExpAST_1 : public BaseAST
{
public:
  std::unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    std::cout << "(";
    exp->Dump();
    std::cout << ")";
  }
  void printIR(std::string &out) override
  {
    exp->printIR(out);
    if (exp->name == -1)
    {
      name = -1;
      val = exp->val;
    }
    else
    {
      name = exp->name;
    }
  }
};

// PrimaryExp - number
class PrimaryExpAST_2 : public BaseAST
{
public:
  int number;

  void Dump() const override
  {
    std::cout<<number;
  }
  void printIR(std::string &out) override
  {
    name = -1;
    val = number;
  }
};

// UnaryExp - primary_exp
class UnaryExpAST_1 : public BaseAST
{
public:
  std::unique_ptr<BaseAST> primary_exp;

  void Dump() const override
  {
    primary_exp->Dump();
  }
  void printIR(std::string &out) override
  {
    primary_exp->printIR(out);
    if (primary_exp->name == -1)
    {
      name = -1;
      val = primary_exp->val;
    }
    else
    {
      name = primary_exp->name;
    }
  }
};

// UnaryExp - unary_op unary_exp
class UnaryExpAST_2 : public BaseAST
{
public:
  std::string unary_op;
  std::unique_ptr<BaseAST> unary_exp;

  void Dump() const override
  {
    std::cout << unary_op;
    unary_exp->Dump();
  }
  void printIR(std::string &out) override
  {
    unary_exp->printIR(out);
    switch (unary_op[0])
    {
    case '+':
      if (unary_exp->name == -1)
      {
        name = -1;
        val = unary_exp->val;
      }
      else
      {
        name = unary_exp->name;
      }
      break;
    case '-':
      name = count_var;
      out += "%";
      out += (std::to_string(count_var)).c_str();
      out += " = sub 0, ";
      if (unary_exp->name == -1)
      {
        out += (std::to_string(unary_exp->val)).c_str();
      }
      else
      {
        out += "%";
        out += (std::to_string(unary_exp->name)).c_str();
      }
      out += "\n";
      count_var++;
      break;
    case '!':
      name = count_var;
      out += "%";
      out += (std::to_string(count_var)).c_str();
      out += " = eq 0, ";
      if (unary_exp->name == -1)
      {
        out += (std::to_string(unary_exp->val)).c_str();
      }
      else
      {
        out += "%";
        out += (std::to_string(unary_exp->name)).c_str();
      }
      out += "\n";
      count_var++;
      break;
    default:
      out += "UnaryExpAST_2_err\n";
    }
  }
};
