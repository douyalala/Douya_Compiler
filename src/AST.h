#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <assert.h>
#include <map>

using namespace std;

static int count_block = 0;
/**
 * count_var给我新建的变量计数并命名
 *  - 相应的非终结符AST中包含自己的名字var_n就是用这个命名
 *  - 如果那个符号不需要名字，比如是个常数，那var_n = -1
 */
static int count_var = 0;
/**
 * 记录常量的map
 */
extern map<string, int> const_init_map;

// 所有 AST 的基类
class BaseAST
{
public:
  string name = "#";
  int val;
  virtual ~BaseAST() = default;

  virtual void Dump() const = 0; // 输出调试用

  virtual void printIR(string &out) = 0; // 生成中间代码
};

// CompUnit
class CompUnitAST : public BaseAST
{
public:
  unique_ptr<BaseAST> func_def;

  void Dump() const override
  {
    cout << "CompUnitAST { ";
    func_def->Dump();
    cout << " }" << endl;
  }

  void printIR(string &out) override
  {
    func_def->printIR(out);
  }
};

// FuncDef
class FuncDefAST : public BaseAST
{
public:
  unique_ptr<BaseAST> func_type;
  string ident;
  unique_ptr<BaseAST> block;

  void Dump() const override
  {
    cout << "FuncDefAST { ";
    func_type->Dump();
    cout << ", " << ident << ", ";
    block->Dump();
    cout << " }";
  }

  void printIR(string &out) override
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
    cout << "FuncTypeAST { int }";
  }

  void printIR(string &out) override
  {
    out += " i32 ";
  }
};

// Block - "{" {bock_item} "}"
class BlockAST : public BaseAST
{
public:
  //vector<unique_ptr<BaseAST>> bock_items;
  unique_ptr<BaseAST> stmt;

  void Dump() const override
  {
    cout << "BlockAST { ";
    // for (int i = 0; i < bock_items.size(); i++)
    // {
    //   bock_items[i]->Dump();
    //   cout << "\n";
    // }
    stmt->Dump();
    cout << " }";
  }

  void printIR(string &out) override
  {
    out += "%block_";
    out += to_string(count_block);
    out += ":\n";
    count_block++;
    // for (int i = 0; i < bock_items.size(); i++)
    // {
    //   bock_items[i]->printIR(out);
    //   out += "\n";
    // }
    stmt->printIR(out);
  }
};

// BlockItem - decl | stmt;
class BlockItemAST : public BaseAST
{
public:
  unique_ptr<BaseAST> decl_or_stmt;

  void Dump() const override
  {
    cout << "BlockItem: { ";
    decl_or_stmt->Dump();
    cout << "}";
  }

  void printIR(string &out) override
  {
    decl_or_stmt->printIR(out);
  }
};

// Stmt - return exp;
class StmtAST : public BaseAST
{
public:
  unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    cout << "StmtAST { ";
    exp->Dump();
    cout << " }";
  }

  void printIR(string &out) override
  {
    exp->printIR(out);
    if (exp->name == "#")
    {
      out += "ret ";
      out += (to_string(exp->val)).c_str();
      out += "\n";
    }
    else
    {
      out += "ret ";
      out += (exp->name).c_str();
      out += "\n";
    }
  }
};

// Exp - unary_exp
class ExpAST_1 : public BaseAST
{
public:
  unique_ptr<BaseAST> unary_exp;

  void Dump() const override
  {
    unary_exp->Dump();
  }
  void printIR(string &out) override
  {
    unary_exp->printIR(out);
    if (unary_exp->name == "#")
    {
      name = "#";
      val = unary_exp->val;
    }
    else
    {
      name = unary_exp->name;
    }
  }
};

// Exp - add_exp
class ExpAST_2 : public BaseAST
{
public:
  unique_ptr<BaseAST> add_exp;

  void Dump() const override
  {
    add_exp->Dump();
  }
  void printIR(string &out) override
  {
    add_exp->printIR(out);
    if (add_exp->name == "#")
    {
      name = "#";
      val = add_exp->val;
    }
    else
    {
      name = add_exp->name;
    }
  }
};

// Exp - lor_exp
class ExpAST_3 : public BaseAST
{
public:
  unique_ptr<BaseAST> lor_exp;

  void Dump() const override
  {
    lor_exp->Dump();
  }
  void printIR(string &out) override
  {
    lor_exp->printIR(out);
    if (lor_exp->name == "#")
    {
      name = "#";
      val = lor_exp->val;
    }
    else
    {
      name = lor_exp->name;
    }
  }
};

// PrimaryExp - (exp)
class PrimaryExpAST_1 : public BaseAST
{
public:
  unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    cout << "(";
    exp->Dump();
    cout << ")";
  }
  void printIR(string &out) override
  {
    exp->printIR(out);
    if (exp->name == "#")
    {
      name = "#";
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
    cout << number;
  }
  void printIR(string &out) override
  {
    name = "#";
    val = number;
  }
};

// PrimaryExp - l_val
class PrimaryExpAST_3 : public BaseAST
{
public:
  unique_ptr<BaseAST> l_val;

  void Dump() const override
  {
    l_val->Dump();
  }
  void printIR(string &out) override
  {
    name = l_val->name;
  }
};

// UnaryExp - primary_exp
class UnaryExpAST_1 : public BaseAST
{
public:
  unique_ptr<BaseAST> primary_exp;

  void Dump() const override
  {
    primary_exp->Dump();
  }
  void printIR(string &out) override
  {
    primary_exp->printIR(out);
    if (primary_exp->name == "#")
    {
      name = "#";
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
  string unary_op;
  unique_ptr<BaseAST> unary_exp;

  void Dump() const override
  {
    cout << unary_op;
    unary_exp->Dump();
  }
  void printIR(string &out) override
  {
    unary_exp->printIR(out);
    switch (unary_op[0])
    {
    case '+':
      if (unary_exp->name == "#")
      {
        name = "#";
        val = unary_exp->val;
      }
      else
      {
        name = unary_exp->name;
      }
      break;
    case '-':
      name = "%" + to_string(count_var);
      out += name.c_str();
      out += " = sub 0, ";
      if (unary_exp->name == "#")
        out += (to_string(unary_exp->val)).c_str();
      else
        out += (unary_exp->name).c_str();
      out += "\n";
      count_var++;
      break;
    case '!':
      name = "%" + to_string(count_var);
      out += name.c_str();
      out += " = eq 0, ";
      if (unary_exp->name == "#")
        out += (to_string(unary_exp->val)).c_str();
      else
        out += (unary_exp->name).c_str();
      out += "\n";
      count_var++;
      break;
    }
  }
};

// MulExp - UnaryExp
class MulExpAST_1 : public BaseAST
{
public:
  unique_ptr<BaseAST> unary_exp;

  void Dump() const override
  {
    unary_exp->Dump();
  }
  void printIR(string &out) override
  {
    unary_exp->printIR(out);
    if (unary_exp->name == "#")
    {
      name = "#";
      val = unary_exp->val;
    }
    else
    {
      name = unary_exp->name;
    }
  }
};

// MulExp - mul_exp ("*" | "/" | "%") unary_exp;
class MulExpAST_2 : public BaseAST
{
public:
  unique_ptr<BaseAST> mul_exp;
  string binary_op;
  unique_ptr<BaseAST> unary_exp;

  void Dump() const override
  {
    mul_exp->Dump();
    cout << binary_op;
    unary_exp->Dump();
  }
  void printIR(string &out) override
  {
    mul_exp->printIR(out);
    unary_exp->printIR(out);
    name = "%" + to_string(count_var);
    out += name.c_str();

    switch (binary_op[0])
    {
    case '*':
      out += " = mul ";
      break;
    case '/':
      out += " = div ";
      break;
    case '%':
      out += " = mod ";
      break;
    }

    if (mul_exp->name == "#")
      out += (to_string(mul_exp->val)).c_str();
    else
      out += (mul_exp->name).c_str();
    out += ", ";
    if (unary_exp->name == "#")
      out += (to_string(unary_exp->val)).c_str();
    else
      out += (unary_exp->name).c_str();
    out += "\n";
    count_var++;
  }
};

// AddExp - mul_exp
class AddExpAST_1 : public BaseAST
{
public:
  unique_ptr<BaseAST> mul_exp;

  void Dump() const override
  {
    mul_exp->Dump();
  }
  void printIR(string &out) override
  {
    mul_exp->printIR(out);
    if (mul_exp->name == "#")
    {
      name = "#";
      val = mul_exp->val;
    }
    else
    {
      name = mul_exp->name;
    }
  }
};

// AddExp - add_exp ("+" | "-") mul_exp;
class AddExpAST_2 : public BaseAST
{
public:
  unique_ptr<BaseAST> add_exp;
  string binary_op;
  unique_ptr<BaseAST> mul_exp;

  void Dump() const override
  {
    add_exp->Dump();
    cout << binary_op;
    mul_exp->Dump();
  }
  void printIR(string &out) override
  {
    add_exp->printIR(out);
    mul_exp->printIR(out);
    name = "%" + to_string(count_var);
    out += name.c_str();

    switch (binary_op[0])
    {
    case '+':
      out += " = add ";
      break;
    case '-':
      out += " = sub ";
      break;
    }

    if (add_exp->name == "#")
      out += (to_string(add_exp->val)).c_str();
    else
      out += (add_exp->name).c_str();
    out += ", ";
    if (mul_exp->name == "#")
      out += (to_string(mul_exp->val)).c_str();
    else
      out += (mul_exp->name).c_str();
    out += "\n";
    count_var++;
  }
};

// RelExp - add_exp
class RelExpAST_1 : public BaseAST
{
public:
  unique_ptr<BaseAST> add_exp;

  void Dump() const override
  {
    add_exp->Dump();
  }
  void printIR(string &out) override
  {
    add_exp->printIR(out);
    if (add_exp->name == "#")
    {
      name = "#";
      val = add_exp->val;
    }
    else
    {
      name = add_exp->name;
    }
  }
};

// RelExp - rel_exp ("<" | ">" | "<=" | ">=") add_exp
class RelExpAST_2 : public BaseAST
{
public:
  unique_ptr<BaseAST> rel_exp;
  string cmp_op;
  int type;
  unique_ptr<BaseAST> add_exp;

  void Dump() const override
  {
    rel_exp->Dump();
    cout << cmp_op;
    add_exp->Dump();
  }
  void printIR(string &out) override
  {
    if (cmp_op.length() >= 2)
    {
      type = cmp_op[0] + cmp_op[1];
    }
    else
    {
      type = cmp_op[0];
    }
    rel_exp->printIR(out);
    add_exp->printIR(out);
    name = "%" + to_string(count_var);
    out += name.c_str();

    switch (type)
    {
    case '<':
      out += " = lt ";
      break;
    case '>':
      out += " = gt ";
      break;
    case '<' + '=':
      out += " = le ";
      break;
    case '>' + '=':
      out += " = ge ";
      break;
    }

    if (rel_exp->name == "#")
      out += (to_string(rel_exp->val)).c_str();
    else
      out += (rel_exp->name).c_str();
    out += ", ";
    if (add_exp->name == "#")
      out += (to_string(add_exp->val)).c_str();
    else
      out += (add_exp->name).c_str();
    out += "\n";
    count_var++;
  }
};

// EqExp - rel_exp
class EqExpAST_1 : public BaseAST
{
public:
  unique_ptr<BaseAST> rel_exp;

  void Dump() const override
  {
    rel_exp->Dump();
  }
  void printIR(string &out) override
  {
    rel_exp->printIR(out);
    if (rel_exp->name == "#")
    {
      name = "#";
      val = rel_exp->val;
    }
    else
    {
      name = rel_exp->name;
    }
  }
};

// EqExp - eq_exp ("==" | "!=") rel_exp
class EqExpAST_2 : public BaseAST
{
public:
  unique_ptr<BaseAST> eq_exp;
  string eq_op;
  unique_ptr<BaseAST> rel_exp;

  void Dump() const override
  {
    eq_exp->Dump();
    cout << eq_op;
    rel_exp->Dump();
  }
  void printIR(string &out) override
  {
    eq_exp->printIR(out);
    rel_exp->printIR(out);
    name = "%" + to_string(count_var);
    out += name.c_str();

    switch (eq_op[0])
    {
    case '=':
      out += " = eq ";
      break;
    case '!':
      out += " = ne ";
      break;
    }

    if (eq_exp->name == "#")
      out += (to_string(eq_exp->val)).c_str();
    else
      out += (eq_exp->name).c_str();
    out += ", ";
    if (rel_exp->name == "#")
      out += (to_string(rel_exp->val)).c_str();
    else
      out += (rel_exp->name).c_str();
    out += "\n";
    count_var++;
  }
};

// LAndExp - eq_exp
class LAndExpAST_1 : public BaseAST
{
public:
  unique_ptr<BaseAST> eq_exp;

  void Dump() const override
  {
    eq_exp->Dump();
  }
  void printIR(string &out) override
  {
    eq_exp->printIR(out);
    if (eq_exp->name == "#")
    {
      name = "#";
      val = eq_exp->val;
    }
    else
    {
      name = eq_exp->name;
    }
  }
};

// LAndExp - land_exp "&&" eq_exp
class LAndExpAST_2 : public BaseAST
{
public:
  unique_ptr<BaseAST> land_exp;
  unique_ptr<BaseAST> eq_exp;

  void Dump() const override
  {
    land_exp->Dump();
    cout << "&&";
    eq_exp->Dump();
  }
  void printIR(string &out) override
  {
    // KoppaIR里面貌似只有按位与and，所以先让两边和0比一下一不一样（变成0或1）
    land_exp->printIR(out);
    eq_exp->printIR(out);

    string tmp_name_1 = "%" + to_string(count_var);
    out += tmp_name_1.c_str();

    out += " = ne ";

    if (land_exp->name == "#")
      out += (to_string(land_exp->val)).c_str();
    else
      out += (land_exp->name).c_str();
    out += ", 0\n";
    count_var++;

    string tmp_name_2 = "%" + to_string(count_var);
    out += tmp_name_2.c_str();

    out += " = ne ";

    if (eq_exp->name == "#")
      out += (to_string(eq_exp->val)).c_str();
    else
      out += (eq_exp->name).c_str();
    out += ", 0\n";
    count_var++;

    name = "%" + to_string(count_var);
    out += name.c_str();

    out += " = and ";
    out += tmp_name_1.c_str();
    out += ", ";
    out += tmp_name_2.c_str();
    out += "\n";
    count_var++;
  }
};

// LOrExp - land_exp
class LOrExpAST_1 : public BaseAST
{
public:
  unique_ptr<BaseAST> land_exp;

  void Dump() const override
  {
    land_exp->Dump();
  }
  void printIR(string &out) override
  {
    land_exp->printIR(out);
    if (land_exp->name == "#")
    {
      name = "#";
      val = land_exp->val;
    }
    else
    {
      name = land_exp->name;
    }
  }
};

// LOrExp - lor_exp "||" land_exp
class LOrExpAST_2 : public BaseAST
{
public:
  unique_ptr<BaseAST> lor_exp;
  unique_ptr<BaseAST> land_exp;

  void Dump() const override
  {
    lor_exp->Dump();
    cout << "||";
    land_exp->Dump();
  }
  void printIR(string &out) override
  {
    // KoppaIR里面也只有按位或or，所以先让两边和0比一下一不一样（变成0或1）
    lor_exp->printIR(out);
    land_exp->printIR(out);

    string tmp_name_1 = "%" + to_string(count_var);
    out += tmp_name_1.c_str();

    out += " = ne ";

    if (lor_exp->name == "#")
      out += (to_string(lor_exp->val)).c_str();
    else
      out += (lor_exp->name).c_str();
    out += ", 0\n";
    count_var++;

    string tmp_name_2 = "%" + to_string(count_var);
    out += tmp_name_2.c_str();

    out += " = ne ";

    if (land_exp->name == "#")
      out += (to_string(land_exp->val)).c_str();
    else
      out += (land_exp->name).c_str();
    out += ", 0\n";
    count_var++;

    name = "%" + to_string(count_var);
    out += name.c_str();

    out += " = or ";
    out += tmp_name_1.c_str();
    out += ", ";
    out += tmp_name_2.c_str();
    out += "\n";
    count_var++;
  }
};

// Decl - const_decl
class DeclAST : public BaseAST
{
public:
  unique_ptr<BaseAST> const_decl;

  void Dump() const override
  {
    const_decl->Dump();
  }

  void printIR(string &out) override
  {
    const_decl->printIR(out);
    if (const_decl->name == "#")
    {
      name = "#";
      val = const_decl->val;
    }
    else
    {
      name = const_decl->name;
    }
  }
};

// ConstDecl - "const" b_type const_def {"," const_def} ";";
class ConstDeclAST : public BaseAST
{
public:
  string b_type; // 目前只能是int
  vector<unique_ptr<BaseAST>> const_defs;

  void Dump() const override
  {
    cout << "const " << b_type << " ";
    for (int i = 0; i < const_defs.size(); i++)
    {
      const_defs[i]->Dump();
      if (i == const_defs.size() - 1)
        cout << ";";
      else
        cout << ",";
    }
  }

  void printIR(string &out) override
  {
    for (int i = 0; i < const_defs.size(); i++)
    {
      const_defs[i]->printIR(out);
    }
  }
};

// ConstDef - ident "=" const_init_val;
class ConstDefAST : public BaseAST
{
public:
  string ident;
  unique_ptr<BaseAST> const_init_val;

  void Dump() const override
  {
    cout << ident << " = ";
    const_init_val->Dump();
  }

  void printIR(string &out) override
  {
    name = "@" + ident;
    const_init_val->printIR(out);
    assert(const_init_val->name == "#");
    const_init_map.insert(make_pair(name, const_init_val->val));
  }
};

// ConstInitVal - const_exp;
class ConstInitValAST : public BaseAST
{
public:
  unique_ptr<BaseAST> const_exp;

  void Dump() const override
  {
    const_exp->Dump();
  }

  void printIR(string &out) override
  {
    const_exp->printIR(out);
    assert(const_exp->name == "#");
    val = const_exp->val;
  }
};

// LVal - ident;
class LValAST : public BaseAST
{
public:
  string ident;

  void Dump() const override
  {
    cout << ident;
  }

  void printIR(string &out) override
  {
    string tmp_name = "@" + ident;
    assert(const_init_map.count(tmp_name) != 0);
    name = "#";
    val = const_init_map.find(tmp_name)->second;
  }
};

// ConstExp - Exp;
class ConstExpAST : public BaseAST
{
public:
  unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    exp->Dump();
  }

  void printIR(string &out) override
  {
    exp->printIR(out);
    assert(exp->name == "#");
    val = exp->val;
  }
};
