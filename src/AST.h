#pragma once

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <deque>
#include <assert.h>
#include <map>
#include "SymbolMap.h"

using namespace std;

static int count_block = 0;
static int count_if = 0;

// now in which while - to which break and continue jump to
extern deque<int> now_in_while;

static int count_break_continue = 0;

// count_var - name my tmp var
static int count_var = 0;

// count_ptr - name my tmp ptr
static int count_ptr = 0;

extern Multi_Symbol_Map *top_symbol_map;

// BaseAST
class BaseAST
{
public:
  // name of AST node, "#" if no name (const exp...)
  string name = "#";

  // val of AST node, for exp
  int val;

  virtual ~BaseAST() = default;

  virtual void Dump() const = 0; // debug

  virtual void printIR(string &out) = 0; // generate IR

  virtual void list_the_param(string &out) = 0; // list the param，only overwrite in FuncRParamsAST
};

// ROOT - CompUnit
class ROOTAST : public BaseAST
{
public:
  unique_ptr<BaseAST> comp_unit;

  void Dump() const override
  {
    comp_unit->Dump();
  }

  void printIR(string &out) override
  {
    // lib func declaration
    out += "decl @getint(): i32\n";
    out += "decl @getch(): i32\n";
    out += "decl @getarray(*i32): i32\n";
    out += "decl @putint(i32)\n";
    out += "decl @putch(i32)\n";
    out += "decl @putarray(i32, *i32)\n";
    out += "decl @starttime()\n";
    out += "decl @stoptime()\n\n";

    comp_unit->printIR(out);
  }

  void list_the_param(string &out) override
  {
  }
};

// CompUnit - [CompUnit] FuncDef
class CompUnitAST : public BaseAST
{
public:
  unique_ptr<BaseAST> comp_unit = nullptr;
  unique_ptr<BaseAST> func_def_or_decl;

  void Dump() const override
  {
    cout << "CompUnitAST { ";
    if (comp_unit != nullptr)
      comp_unit->Dump();
    cout << "{";
    func_def_or_decl->Dump();
    cout << "}";
    cout << " }" << endl;
  }

  void printIR(string &out) override
  {

    if (comp_unit != nullptr)
    {
      comp_unit->printIR(out);
      out += "\n";
    }
    func_def_or_decl->printIR(out);
  }

  void list_the_param(string &out) override
  {
  }
};

// FuncDef - func_type ident "(" [func_f_params] ")" block
class FuncDefAST : public BaseAST
{
public:
  string func_type = ""; // int - "i32", void - ""
  string ident;
  unique_ptr<BaseAST> func_f_params = nullptr;
  unique_ptr<BaseAST> block;

  void Dump() const override
  {
    cout << "FuncDefAST { TYPE: " << func_type;
    cout << ", IDENT: " << ident << ", PARAM: ";
    if (func_f_params != nullptr)
      func_f_params->Dump();
    cout << ", BLOCK: ";
    block->Dump();
    cout << " }";
  }

  void printIR(string &out) override
  {
    name = "@" + ident;
    VarUnion tmp_func;
    tmp_func.kind = var_kind_FUNC;
    tmp_func.type.type = func_type;
    top_symbol_map->insert(name, tmp_func);

    Multi_Symbol_Map *new_top = new Multi_Symbol_Map;
    new_top->outer_map = top_symbol_map;
    top_symbol_map = new_top;

    count_block++;

    out += "fun @";
    out += ident;
    out += "(";
    if (func_f_params != nullptr)
    {
      func_f_params->printIR(out);
    }
    out += ")";
    if (func_type == "i32")
      out += ": " + func_type;
    out += " {\n";

    out += "%entry:\n";

    block->printIR(out);

    if (block->name != "ret")
    {
      if (func_type == "")
        out += "ret\n";
      if (func_type == "i32")
        out += "ret 0\n";
    }
    out += "}\n";

    top_symbol_map = top_symbol_map->outer_map;
  }

  void list_the_param(string &out) override
  {
  }
};

// FuncFParams - func_f_param {"," func_f_param}
class FuncFParamsAST : public BaseAST
{
public:
  vector<unique_ptr<BaseAST>> *func_f_params;

  void Dump() const override
  {
    cout << "FuncFParamsAST { ";
    for (int i = 0; i < func_f_params->size(); i++)
    {
      func_f_params->at(i)->Dump();
      cout << ";";
    }
    cout << " }";
  }

  void printIR(string &out) override
  {
    for (int i = 0; i < func_f_params->size(); i++)
    {
      func_f_params->at(i)->printIR(out);
      if (i != func_f_params->size() - 1)
        out += ", ";
    }
  }

  void list_the_param(string &out) override
  {
  }
};

// FuncFParam  ::= BType IDENT;
class FuncFParamAST : public BaseAST
{
public:
  string b_type; // int - "i32"
  string ident;

  void Dump() const override
  {
    cout << b_type << " " << ident;
  }

  void printIR(string &out) override
  {
    string param_name = "@" + ident;
    VarUnion param_tmp;
    param_tmp.kind = var_kind_VAR;
    param_tmp.type.type = b_type;
    param_tmp.def_block_id = count_block;
    param_tmp.var_is_func_param = 1;
    top_symbol_map->insert(param_name, param_tmp);

    out += param_name + ": " + b_type;
  }

  void list_the_param(string &out) override
  {
  }
};

// Block - "{" {bock_item} "}"
class BlockAST : public BaseAST
{
public:
  vector<unique_ptr<BaseAST>> *block_items;

  void Dump() const override
  {
    cout << "BlockAST { ";
    int size = block_items->size();
    cout << "(has " << size << " item)";
    for (int i = 0; i < size; i++)
    {
      block_items->at(i)->Dump();
    }
    cout << " }";
  }

  void printIR(string &out) override
  {
    Multi_Symbol_Map *new_top = new Multi_Symbol_Map;
    new_top->outer_map = top_symbol_map;
    top_symbol_map = new_top;

    count_block++;
    for (int i = 0; i < block_items->size(); i++)
    {
      block_items->at(i)->printIR(out);
      if (block_items->at(i)->name == "ret")
      {
        name = "ret";
        break;
      }
    }

    top_symbol_map = top_symbol_map->outer_map;
  }

  void list_the_param(string &out) override
  {
  }
};

// BlockItem - decl | stmt
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
    name = decl_or_stmt->name;
  }

  void list_the_param(string &out) override
  {
  }
};

// Stmt - match_stmt
class StmtAST_1 : public BaseAST
{
public:
  unique_ptr<BaseAST> match_stmt;

  void Dump() const override
  {
    cout << "StmtAST { ";
    match_stmt->Dump();
    cout << " }";
  }

  void printIR(string &out) override
  {
    match_stmt->printIR(out);
    name = match_stmt->name;
  }

  void list_the_param(string &out) override
  {
  }
};

// Stmt - unmatch_stmt
class StmtAST_2 : public BaseAST
{
public:
  unique_ptr<BaseAST> unmatch_stmt;

  void Dump() const override
  {
    cout << "StmtAST { ";
    unmatch_stmt->Dump();
    cout << " }";
  }

  void printIR(string &out) override
  {
    unmatch_stmt->printIR(out);
    name = unmatch_stmt->name;
  }

  void list_the_param(string &out) override
  {
  }
};

// MatchStmt - return [exp];
class MatchStmtAST_1 : public BaseAST
{
public:
  unique_ptr<BaseAST> exp = nullptr;

  void Dump() const override
  {
    cout << "MatchStmtAST { return ";
    if (exp != nullptr)
      exp->Dump();
    cout << " }";
  }

  void printIR(string &out) override
  {
    name = "ret";
    if (exp != nullptr)
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
    else
    {
      out += "ret\n";
    }
  }

  void list_the_param(string &out) override
  {
  }
};

// MatchStmt - l_val "=" exp ";"
class MatchStmtAST_2 : public BaseAST
{
public:
  unique_ptr<BaseAST> l_val;
  unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    cout << "MatchStmtAST { ";
    l_val->Dump();
    cout << "=";
    exp->Dump();
    cout << " }";
  }

  void printIR(string &out) override
  {
    l_val->printIR(out);
    assert(l_val->name != "#");
    exp->printIR(out);
    if (exp->name == "#")
    {
      out += "store ";
      out += to_string(exp->val).c_str();
      out += ", ";
      out += l_val->name;
      out += "\n";
    }
    else
    {
      out += "store ";
      out += exp->name;
      out += ", ";
      out += l_val->name;
      out += "\n";
    }
  }

  void list_the_param(string &out) override
  {
  }
};

// MatchStmt - [exp] ";"
class MatchStmtAST_3 : public BaseAST
{
public:
  unique_ptr<BaseAST> exp = nullptr;

  void Dump() const override
  {
    if (exp != nullptr)
    {
      cout << "MatchStmtAST { ";
      exp->Dump();
      cout << " }";
    }
  }

  void printIR(string &out) override
  {
    if (exp != nullptr)
    {
      exp->printIR(out);
    }
  }

  void list_the_param(string &out) override {}
};

// MatchStmt - block
class MatchStmtAST_4 : public BaseAST
{
public:
  unique_ptr<BaseAST> block;

  void Dump() const override
  {
    cout << "MatchStmtAST { ";
    block->Dump();
    cout << " }";
  }

  void printIR(string &out) override
  {
    block->printIR(out);
    name = block->name;
  }

  void list_the_param(string &out) override {}
};

// MatchStmt - "if" "(" exp ")" match_stmt "else" match_stmt
class MatchStmtAST_5 : public BaseAST
{
public:
  unique_ptr<BaseAST> exp;
  unique_ptr<BaseAST> match_stmt_if;
  unique_ptr<BaseAST> match_stmt_else;

  void Dump() const override
  {
    cout << "MatchStmtAST { ";
    cout << "if( ";
    exp->Dump();
    cout << " ){ ";
    match_stmt_if->Dump();
    cout << " }else{ ";
    match_stmt_else->Dump();
    cout << " } ";
    cout << " }";
  }

  void printIR(string &out) override
  {
    int if_else_id = count_if;
    count_if++;

    exp->printIR(out);

    if (exp->name == "#")
    {
      // exp is const
      out += "jump %const_if_" + to_string(if_else_id) + "\n";
      out += "\n%const_if_" + to_string(if_else_id) + ":\n";
      if (exp->val)
      {
        match_stmt_if->printIR(out);
        if (match_stmt_if->name != "ret")
          out += "jump %const_end_" + to_string(if_else_id) + "\n";
      }
      else
      {
        match_stmt_else->printIR(out);
        if (match_stmt_else->name != "ret")
          out += "jump %const_end_" + to_string(if_else_id) + "\n";
      }
      out += "\n%const_end_" + to_string(if_else_id) + ":\n";
    }
    else
    {
      // exp is a var
      out += "br ";
      out += exp->name;
      out += ", %if_" + to_string(if_else_id);
      out += ", %else_" + to_string(if_else_id);
      out += "\n";

      out += "\n%if_" + to_string(if_else_id);
      out += ":\n";
      match_stmt_if->printIR(out);
      if (match_stmt_if->name != "ret")
        out += "jump %end_" + to_string(if_else_id) + "\n";

      out += "\n%else_" + to_string(if_else_id);
      out += ":\n";
      match_stmt_else->printIR(out);
      if (match_stmt_else->name != "ret")
        out += "jump %end_" + to_string(if_else_id) + "\n";

      out += "\n%end_" + to_string(if_else_id) + ":\n";
    }
  }

  void list_the_param(string &out) override {}
};

// MatchStmt - "while" "(" exp ")" stmt
class MatchStmtAST_6 : public BaseAST
{
public:
  unique_ptr<BaseAST> exp;
  unique_ptr<BaseAST> stmt;

  void Dump() const override
  {
    cout << "MatchStmtAST { ";
    cout << "while( ";
    exp->Dump();
    cout << " ){ ";
    stmt->Dump();
    cout << " } ";
    cout << " }";
  }

  void printIR(string &out) override
  {
    int while_id = count_if;
    count_if++;

    now_in_while.push_back(while_id);

    exp->printIR(out);
    if (exp->name == "#")
    {
      if (!exp->val)
      {
        // while(0)
        // 无事发生
        now_in_while.pop_back();
        return;
      }
      else
      {
        // while(1)
        out += "br 1, %while_body_" + to_string(while_id) + ", %while_end_" + to_string(while_id) + "\n";
        out += "\n%while_entry_" + to_string(while_id) + ":\n";
        out += "br 1, %while_body_" + to_string(while_id) + ", %while_end_" + to_string(while_id) + "\n";
        out += "\n%while_body_" + to_string(while_id) + ":\n";
        stmt->printIR(out);
        if (stmt->name != "ret")
          out += "jump %while_entry_" + to_string(while_id) + "\n";
        out += "\n%while_end_" + to_string(while_id) + ":\n";
        now_in_while.pop_back();
        return;
      }
    }
    else
    {
      out += "br " + exp->name + ", " + "%while_body_" + to_string(while_id) + ", %while_end_" + to_string(while_id) + "\n";
      out += "\n%while_entry_" + to_string(while_id) + ":\n";
      exp->printIR(out);
      out += "br " + exp->name + ", " + "%while_body_" + to_string(while_id) + ", %while_end_" + to_string(while_id) + "\n";
      out += "\n%while_body_" + to_string(while_id) + ":\n";
      stmt->printIR(out);
      if (stmt->name != "ret")
        out += "jump %while_entry_" + to_string(while_id) + "\n";
      out += "\n%while_end_" + to_string(while_id) + ":\n";
      now_in_while.pop_back();
      return;
    }
  }

  void list_the_param(string &out) override {}
};

// MatchStmt - "break;"
class MatchStmtAST_7 : public BaseAST
{
public:
  void Dump() const override
  {
    cout << "MatchStmtAST { ";
    cout << "break;";
    cout << " }";
  }

  void printIR(string &out) override
  {
    out += "jump %while_end_" + to_string(now_in_while.back()) + "\n";
    out += "\n%while_body_" + to_string(now_in_while.back()) + "_" + to_string(count_break_continue) + ":\n";
    count_break_continue++;
  }

  void list_the_param(string &out) override {}
};

// MatchStmt - "continue;"
class MatchStmtAST_8 : public BaseAST
{
public:
  void Dump() const override
  {
    cout << "MatchStmtAST { ";
    cout << "continue;";
    cout << " }";
  }

  void printIR(string &out) override
  {
    out += "jump %while_entry_" + to_string(now_in_while.back()) + "\n";
    out += "\n%while_body_" + to_string(now_in_while.back()) + "_" + to_string(count_break_continue) + ":\n";
    count_break_continue++;
  }

  void list_the_param(string &out) override {}
};

// UnMatchStmt - "if" "(" exp ")" stmt
class UnMatchStmtAST_1 : public BaseAST
{
public:
  unique_ptr<BaseAST> exp;
  unique_ptr<BaseAST> stmt;

  void Dump() const override
  {
    cout << "UnMatchStmtAST { ";
    cout << "if( ";
    exp->Dump();
    cout << " ){ ";
    stmt->Dump();
    cout << " } ";
    cout << " }";
  }

  void printIR(string &out) override
  {
    int if_else_id = count_if;
    count_if++;

    exp->printIR(out);
    if (exp->name == "#")
    {
      // exp is const
      if (exp->val)
      {
        // if(1)
        out += "jump %const_if_" + to_string(if_else_id) + "\n";
        out += "\n%const_if_" + to_string(if_else_id) + ":\n";
        stmt->printIR(out);
        if (stmt->name != "ret")
        {
          out += "jump %const_end_" + to_string(if_else_id) + "\n";
        }
        out += "\n%const_end_" + to_string(if_else_id) + ":\n";
      }
      // if(0) nothing
    }
    else
    {
      // exp is a var
      out += "br ";
      out += exp->name;
      out += ", %if_" + to_string(if_else_id);
      out += ", %end_" + to_string(if_else_id);
      out += "\n";

      out += "\n%if_" + to_string(if_else_id);
      out += ":\n";
      stmt->printIR(out);
      if (stmt->name != "ret")
        out += "jump %end_" + to_string(if_else_id) + "\n";

      out += "\n%end_" + to_string(if_else_id);
      out += ":\n";
    }
  }

  void list_the_param(string &out) override {}
};

// UnMatchStmt - "if" "(" exp ")" match_stmt "else" unmatch_stmt
class UnMatchStmtAST_2 : public BaseAST
{
public:
  unique_ptr<BaseAST> exp;
  unique_ptr<BaseAST> match_stmt;
  unique_ptr<BaseAST> unmatch_stmt;

  void Dump() const override
  {
    cout << "UnMatchStmtAST { ";
    cout << "if( ";
    exp->Dump();
    cout << " ){ ";
    match_stmt->Dump();
    cout << " }else{ ";
    unmatch_stmt->Dump();
    cout << " }";
    cout << " }";
  }

  void printIR(string &out) override
  {
    int if_else_id = count_if;
    count_if++;

    exp->printIR(out);
    if (exp->name == "#")
    {
      // exp is const
      out += "jump %const_if_" + to_string(if_else_id) + "\n";
      out += "\n%const_if_" + to_string(if_else_id) + ":\n";
      if (exp->val)
      {
        match_stmt->printIR(out);
        if (match_stmt->name != "ret")
        {
          out += "jump %const_end_" + to_string(if_else_id) + "\n";
        }
      }
      else
      {
        unmatch_stmt->printIR(out);
        if (unmatch_stmt->name != "ret")
        {
          out += "jump %const_end_" + to_string(if_else_id) + "\n";
        }
      }
      out += "\n%const_end_" + to_string(if_else_id) + ":\n";
    }
    else
    {
      // exp is a var
      out += "br ";
      out += exp->name;
      out += ", %if_" + to_string(if_else_id);
      out += ", %else_" + to_string(if_else_id);
      out += "\n";

      out += "\n%if_" + to_string(if_else_id);
      out += ":\n";
      match_stmt->printIR(out);
      if (match_stmt->name != "ret")
        out += "jump %end_" + to_string(if_else_id) + "\n";

      out += "\n%else_" + to_string(if_else_id);
      out += ":\n";
      unmatch_stmt->printIR(out);
      if (unmatch_stmt->name != "ret")
        out += "jump %end_" + to_string(if_else_id) + "\n";

      out += "\n%end_" + to_string(if_else_id);
      out += ":\n";
    }
  }

  void list_the_param(string &out) override {}
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

  void list_the_param(string &out) override {}
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

  void list_the_param(string &out) override {}
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

  void list_the_param(string &out) override {}
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

  void list_the_param(string &out) override {}
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

  void list_the_param(string &out) override {}
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
    l_val->printIR(out);

    if (l_val->name == "#")
    {
      name = "#";
      val = l_val->val;
    }
    else
    {
      name = "%" + to_string(count_var);
      out += name;
      out += " = load ";
      out += l_val->name;
      out += "\n";
      count_var++;
    }
  }

  void list_the_param(string &out) override {}
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

  void list_the_param(string &out) override {}
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

    if (unary_exp->name == "#")
    {
      name = "#";
      switch (unary_op[0])
      {
      case '+':
        val = unary_exp->val;
        break;
      case '-':
        val = 0 - (unary_exp->val);
        break;
      case '!':
        val = !(unary_exp->val);
        break;
      }
    }
    else
    {
      switch (unary_op[0])
      {
      case '+':
        name = unary_exp->name;
        break;
      case '-':
        name = "%" + to_string(count_var);
        out += name.c_str();
        out += " = sub 0, ";
        out += (unary_exp->name).c_str();
        out += "\n";
        count_var++;
        break;
      case '!':
        name = "%" + to_string(count_var);
        out += name.c_str();
        out += " = eq 0, ";
        out += (unary_exp->name).c_str();
        out += "\n";
        count_var++;
        break;
      }
    }
  }

  void list_the_param(string &out) override {}
};

// UnaryExp - ident "(" [func_r_params] ")"
// this is func call
class UnaryExpAST_3 : public BaseAST
{
public:
  string ident;
  unique_ptr<BaseAST> func_r_params = nullptr;

  void Dump() const override
  {
    cout << ident << "(";
    if (func_r_params != nullptr)
      func_r_params->Dump();
    cout << ")";
  }

  void printIR(string &out) override
  {
    if (func_r_params != nullptr)
    {
      func_r_params->printIR(out);
    }

    string fuc_name = "@" + ident;
    VarUnion tmp_func = top_symbol_map->find(fuc_name);
    if (tmp_func.type.type != "")
    {
      name = "%" + to_string(count_var);
      count_var++;
      out += name + " = ";
    }
    out += "call " + fuc_name + "(";
    if (func_r_params != nullptr)
    {
      func_r_params->list_the_param(out);
    }
    out += ")\n";
  }

  void list_the_param(string &out) override
  {
  }
};

// FuncRParams ::= Exp {"," Expvoid list_the_param(string &out) override{}}
class FuncRParamsAST : public BaseAST
{
public:
  vector<unique_ptr<BaseAST>> *exps = nullptr;

  void Dump() const override
  {
    if (exps != nullptr)
    {
      for (int i = 0; i < exps->size(); i++)
      {
        exps->at(i)->Dump();
        if (i != exps->size() - 1)
        {
          cout << ", ";
        }
      }
    }
  }

  void printIR(string &out) override
  {
    if (exps != nullptr)
    {
      for (int i = 0; i < exps->size(); i++)
      {
        exps->at(i)->printIR(out);
      }
    }
  }

  void list_the_param(string &out) override
  {
    if (exps != nullptr)
    {
      for (int i = 0; i < exps->size(); i++)
      {
        if (exps->at(i)->name == "#")
        {
          out += to_string(exps->at(i)->val);
        }
        else
        {
          out += exps->at(i)->name;
        }
        if (i != exps->size() - 1)
        {
          out += ", ";
        }
      }
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

  void list_the_param(string &out) override {}
};

// MulExp - mul_exp ("*" | "/" | "%") unary_exp
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

    if (mul_exp->name == "#" && unary_exp->name == "#")
    {
      name = "#";
      switch (binary_op[0])
      {
      case '*':
        val = (mul_exp->val) * (unary_exp->val);
        break;
      case '/':
        val = (mul_exp->val) / (unary_exp->val);
        break;
      case '%':
        val = (mul_exp->val) % (unary_exp->val);
        break;
      }
      return;
    }

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

  void list_the_param(string &out) override {}
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

  void list_the_param(string &out) override {}
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

    if (add_exp->name == "#" && mul_exp->name == "#")
    {
      name = "#";
      switch (binary_op[0])
      {
      case '+':
        val = (add_exp->val) + (mul_exp->val);
        break;
      case '-':
        val = (add_exp->val) - (mul_exp->val);
        break;
      }
      return;
    }

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

  void list_the_param(string &out) override {}
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

  void list_the_param(string &out) override {}
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

    // prepare for switch
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

    if (rel_exp->name == "#" && add_exp->name == "#")
    {
      name = "#";
      switch (type)
      {
      case '<':
        val = (rel_exp->val) < (add_exp->val);
        break;
      case '>':
        val = (rel_exp->val) > (add_exp->val);
        break;
      case '<' + '=':
        val = (rel_exp->val) <= (add_exp->val);
        break;
      case '>' + '=':
        val = (rel_exp->val) >= (add_exp->val);
        break;
      }
      return;
    }

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

  void list_the_param(string &out) override {}
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

  void list_the_param(string &out) override {}
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

    if (eq_exp->name == "#" && rel_exp->name == "#")
    {
      name = "#";
      switch (eq_op[0])
      {
      case '=':
        val = ((eq_exp->val) == (rel_exp->val));
        break;
      case '!':
        val = ((eq_exp->val) != (rel_exp->val));
        break;
      }
      return;
    }

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

  void list_the_param(string &out) override {}
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

  void list_the_param(string &out) override {}
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
    land_exp->printIR(out);

    if (land_exp->name == "#")
    {
      if (!land_exp->val)
      {
        name = "#";
        val = 0;
        return;
      }
      else
      {
        eq_exp->printIR(out);
        if (eq_exp->name == "#")
        {
          name = "#";
          val = (eq_exp->val != 0);
          return;
        }
        else
        {
          name = "%" + to_string(count_var);
          count_var++;
          out += name + " = ne 0, " + eq_exp->name + "\n";
          return;
        }
      }
    }
    else
    {
      string tmp_var_name = "%" + to_string(count_var);
      count_var++;

      int if_else_id = count_if;
      count_if++;

      out += tmp_var_name + " = alloc i32\n";
      out += "br " + land_exp->name + ", %if_" + to_string(if_else_id) + ", %else_" + to_string(if_else_id) + "\n";

      out += "\n%if_" + to_string(if_else_id) + ":\n";
      eq_exp->printIR(out);
      if (eq_exp->name == "#")
      {
        out += "store " + to_string((eq_exp->val) != 0) + ", " + tmp_var_name + "\n";
      }
      else
      {
        string tmp_name = "%" + to_string(count_var);
        count_var++;
        out += tmp_name + " = ne 0, " + eq_exp->name + "\n";
        out += "store " + tmp_name + ", " + tmp_var_name + "\n";
      }
      out += "jump %end_" + to_string(if_else_id) + "\n";

      out += "\n%else_" + to_string(if_else_id) + ":\n";
      out += "store 0, " + tmp_var_name + "\n";
      out += "jump %end_" + to_string(if_else_id) + "\n";

      out += "\n%end_" + to_string(if_else_id) + ":\n";

      name = "%" + to_string(count_var);
      count_var++;
      out += name + " = load " + tmp_var_name + "\n";
    }
  }

  void list_the_param(string &out) override {}
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

  void list_the_param(string &out) override {}
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
    lor_exp->printIR(out);

    if (lor_exp->name == "#")
    {
      if (lor_exp->val)
      {
        name = "#";
        val = 1;
        return;
      }
      else
      {
        land_exp->printIR(out);
        if (land_exp->name == "#")
        {
          name = "#";
          val = (land_exp->val != 0);
          return;
        }
        else
        {
          name = "%" + to_string(count_var);
          count_var++;
          out += name + " = ne 0, " + land_exp->name + "\n";
          return;
        }
      }
    }
    else
    {
      string tmp_var_name = "%" + to_string(count_var);
      count_var++;

      int if_else_id = count_if;
      count_if++;

      out += tmp_var_name + " = alloc i32\n";

      out += "br " + lor_exp->name + ", %if_" + to_string(if_else_id) + ", %else_" + to_string(if_else_id) + "\n";
      out += "\n%if_" + to_string(if_else_id) + ":\n";
      out += "store 1, " + tmp_var_name + "\n";
      out += "jump %end_" + to_string(if_else_id) + "\n";
      out += "\n%else_" + to_string(if_else_id) + ":\n";
      land_exp->printIR(out);
      if (land_exp->name == "#")
      {
        out += "store " + to_string((land_exp->val) != 0) + ", " + tmp_var_name + "\n";
      }
      else
      {
        string tmp_name = "%" + to_string(count_var);
        count_var++;
        out += tmp_name + " = ne 0, " + land_exp->name + "\n";
        out += "store " + tmp_name + ", " + tmp_var_name + "\n";
      }
      out += "jump %end_" + to_string(if_else_id) + "\n";
      out += "\n%end_" + to_string(if_else_id) + ":\n";

      name = "%" + to_string(count_var);
      count_var++;
      out += name + " = load " + tmp_var_name + "\n";
    }
  }

  void list_the_param(string &out) override {}
};

// Decl - const_decl
class DeclAST_1 : public BaseAST
{
public:
  unique_ptr<BaseAST> const_decl;

  void Dump() const override
  {
    cout << "DeclAST { ";
    const_decl->Dump();
    cout << " }";
  }

  void printIR(string &out) override
  {
    const_decl->printIR(out);
  }

  void list_the_param(string &out) override {}
};

// Decl - var_decl
class DeclAST_2 : public BaseAST
{
public:
  unique_ptr<BaseAST> var_decl;

  void Dump() const override
  {
    cout << "DeclAST { ";
    var_decl->Dump();
    cout << " }";
  }

  void printIR(string &out) override
  {
    var_decl->printIR(out);
  }

  void list_the_param(string &out) override {}
};

// ConstDecl - "const" int const_def {"," const_def} ";"
class ConstDeclAST : public BaseAST
{
public:
  vector<unique_ptr<BaseAST>> *const_defs;

  void Dump() const override
  {
    cout << "const i32 ";
    for (int i = 0; i < const_defs->size(); i++)
    {
      const_defs->at(i)->Dump();
      if (i == const_defs->size() - 1)
        cout << "; ";
      else
        cout << ",";
    }
  }

  void printIR(string &out) override
  {
    for (int i = 0; i < const_defs->size(); i++)
    {
      const_defs->at(i)->printIR(out);
    }
  }

  void list_the_param(string &out) override {}
};

// ConstInitVal - const_exp
class ConstInitValAST_1 : public BaseAST
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

  void list_the_param(string &out) override {}
};

// ConstInitVal - "{" [const_exp {"," const_exp}] "}"
class ConstInitValAST_2 : public BaseAST
{
public:
  vector<unique_ptr<BaseAST>> *const_exps;

  void Dump() const override
  {
    for (int i = 0; i < const_exps->size(); i++)
    {
      const_exps->at(i)->Dump();
      cout << ", ";
    }
  }

  void printIR(string &out) override
  {
    for (int i = 0; i < const_exps->size(); i++)
    {
      const_exps->at(i)->printIR(out);
      assert(const_exps->at(i)->name == "#");
    }
  }

  void list_the_param(string &out) override {}
};

// ConstDef - ident "=" const_init_val
class ConstDefAST_1 : public BaseAST
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
    string b_type = "i32";
    const_init_val->printIR(out);
    assert(const_init_val->name == "#");
    VarUnion const_tmp;
    const_tmp.kind = var_kind_CONST;
    const_tmp.type.type = b_type;
    const_tmp.val.push_back(const_init_val->val);
    const_tmp.def_block_id = count_block;
    top_symbol_map->insert(name, const_tmp);
  }

  void list_the_param(string &out) override {}
};

// ConstDef - ident "[" const_exp "]" "=" const_init_val
class ConstDefAST_2 : public BaseAST
{
public:
  string ident;
  unique_ptr<BaseAST> const_exp;
  unique_ptr<ConstInitValAST_2> const_init_val;

  void Dump() const override
  {
    cout << ident << " = ";
    const_init_val->Dump();
  }

  void printIR(string &out) override
  {
    name = "@" + ident;
    string b_type = "i32";

    const_exp->printIR(out);
    assert(const_exp->name == "#");

    const_init_val->printIR(out);

    VarUnion const_tmp;
    const_tmp.kind = var_kind_CONST;
    const_tmp.type.type = b_type;
    const_tmp.type.array_len.push_back(const_exp->val);
    const_tmp.val.push_back(const_init_val->val);
    const_tmp.def_block_id = count_block;
    top_symbol_map->insert(name, const_tmp);
  }

  void list_the_param(string &out) override {}
};

// VarDecl - int var_def {"," var_def} ";"
class VarDeclAST : public BaseAST
{
public:
  vector<unique_ptr<BaseAST>> *var_defs;

  void Dump() const override
  {
    cout << " i32 ";
    for (int i = 0; i < var_defs->size(); i++)
    {
      var_defs->at(i)->Dump();
      if (i == var_defs->size() - 1)
        cout << "; ";
      else
        cout << ",";
    }
  }

  void printIR(string &out) override
  {
    for (int i = 0; i < var_defs->size(); i++)
    {
      var_defs->at(i)->printIR(out);
    }
  }

  void list_the_param(string &out) override {}
};

// InitVal - exp
class InitValAST_1 : public BaseAST
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

  void list_the_param(string &out) override {}
};

// InitVal - "{" [Exp {"," Exp}] "}"
class InitValAST_2 : public BaseAST
{
public:
  vector<unique_ptr<BaseAST>> *exps;

  void Dump() const override
  {
    for (int i = 0; i < exps->size(); i++)
    {
      exps->at(i)->Dump();
      cout << ",";
    }
  }

  void printIR(string &out) override
  {
    for (int i = 0; i < exps->size(); i++)
    {
      exps->at(i)->printIR(out);
    }
  }

  void list_the_param(string &out) override {}
};

// VarDef - ident
class VarDefAST_1 : public BaseAST
{
public:
  string ident;

  void Dump() const override
  {
    cout << ident;
  }

  void printIR(string &out) override
  {
    name = "@" + ident;
    string b_type = "i32";

    VarUnion var_tmp;
    if (top_symbol_map->outer_map == nullptr)
    {
      var_tmp.kind = var_kind_NOT_INIT_GLOBAL_VAR;
      var_tmp.type.type = b_type;
      var_tmp.def_block_id = count_block;
      var_tmp.var_is_func_param = 0;
      top_symbol_map->insert(name, var_tmp);

      out += "global " + name + "_" + to_string(count_block);
      out += " = alloc i32, zeroinit\n";
    }
    else
    {
      var_tmp.kind = var_kind_VAR;
      var_tmp.type.type = b_type;
      var_tmp.def_block_id = count_block;
      top_symbol_map->insert(name, var_tmp);

      out += name + "_" + to_string(count_block);
      out += " = alloc i32\n";
    }
  }

  void list_the_param(string &out) override {}
};

// VarDef - ident "=" init_val
class VarDefAST_2 : public BaseAST
{
public:
  string ident;
  unique_ptr<BaseAST> init_val;

  void Dump() const override
  {
    cout << ident << " = ";
    init_val->Dump();
  }

  void printIR(string &out) override
  {
    VarUnion var_tmp;
    string b_type = "i32";
    name = "@" + ident;

    if (top_symbol_map->outer_map == nullptr)
    {
      var_tmp.kind = var_kind_INIT_GLOBAL_VAR;
      var_tmp.type.type = b_type;
      var_tmp.def_block_id = count_block;

      init_val->printIR(out);
      assert(init_val->name == "#");
      var_tmp.var_is_func_param = 0;

      top_symbol_map->insert(name, var_tmp);

      out += "global " + name + "_" + to_string(count_block);
      out += " = alloc i32, " + to_string(init_val->val) + "\n";
    }
    else
    {
      var_tmp.kind = var_kind_VAR;
      var_tmp.type.type = b_type;
      var_tmp.def_block_id = count_block;
      top_symbol_map->insert(name, var_tmp);

      out += name + "_" + to_string(count_block);
      out += " = alloc i32\n";

      init_val->printIR(out);

      if (init_val->name == "#")
      {
        out += "store ";
        out += to_string(init_val->val).c_str();
        out += ", ";
        out += name + "_" + to_string(count_block);
        out += "\n";
      }
      else
      {
        out += "store ";
        out += init_val->name;
        out += ", ";
        out += name + "_" + to_string(count_block);
        out += "\n";
      }
    }
  }

  void list_the_param(string &out) override {}
};

// VarDef - ident "[" const_exp "]"
class VarDefAST_3 : public BaseAST
{
public:
  string ident;
  unique_ptr<BaseAST> const_exp;

  void Dump() const override
  {
    cout << ident;
  }

  void printIR(string &out) override
  {
    name = "@" + ident;
    string b_type = "i32";

    const_exp->printIR(out);
    assert(const_exp->name == "#");

    VarUnion var_tmp;
    if (top_symbol_map->outer_map == nullptr)
    {
      var_tmp.kind = var_kind_NOT_INIT_GLOBAL_VAR;
      var_tmp.type.type = b_type;
      var_tmp.type.array_len.push_back(const_exp->val);
      var_tmp.def_block_id = count_block;
      var_tmp.var_is_func_param = 0;
      top_symbol_map->insert(name, var_tmp);

      out += "global " + name + "_" + to_string(count_block);
      out += " = alloc [i32, " + to_string(const_exp->val) + "], zeroinit\n";
    }
    else
    {
      var_tmp.kind = var_kind_VAR;
      var_tmp.type.type = b_type;
      var_tmp.type.array_len.push_back(const_exp->val);
      var_tmp.def_block_id = count_block;
      top_symbol_map->insert(name, var_tmp);

      out += name + "_" + to_string(count_block);
      out += " = alloc [i32, " + to_string(const_exp->val) + "]\n";
    }
  }

  void list_the_param(string &out) override {}
};

// VarDef - ident "[" const_exp "]" "=" init_val
class VarDefAST_4 : public BaseAST
{
public:
  string ident;
  unique_ptr<BaseAST> const_exp;
  unique_ptr<InitValAST_2> init_val;

  void Dump() const override
  {
    cout << ident;
  }

  void printIR(string &out) override
  {
    name = "@" + ident;
    string b_type = "i32";

    const_exp->printIR(out);
    assert(const_exp->name == "#");

    init_val->printIR(out);

    VarUnion var_tmp;
    if (top_symbol_map->outer_map == nullptr)
    {
      var_tmp.kind = var_kind_NOT_INIT_GLOBAL_VAR;
      var_tmp.type.type = b_type;
      var_tmp.type.array_len.push_back(const_exp->val);
      var_tmp.def_block_id = count_block;
      var_tmp.var_is_func_param = 0;
      top_symbol_map->insert(name, var_tmp);

      string aggregate = "{";
      for (int i = 0; i <= const_exp->val; i++)
      {
        if (i < init_val->exps->size())
        {
          aggregate += to_string(init_val->exps->at(i)->val);
        }
        else
        {
          aggregate += "0";
        }
        if (i != const_exp->val)
        {
          aggregate += ", ";
        }
      }
      aggregate += "}";

      out += "global " + name + "_" + to_string(count_block);
      out += " = alloc [i32, " + to_string(const_exp->val) + "], " + aggregate + "\n";
    }
    else
    {
      var_tmp.kind = var_kind_VAR;
      var_tmp.type.type = b_type;
      var_tmp.type.array_len.push_back(const_exp->val);
      var_tmp.def_block_id = count_block;
      top_symbol_map->insert(name, var_tmp);

      out += name + "_" + to_string(count_block);
      out += " = alloc [i32, " + to_string(const_exp->val) + "]\n";

      for (int i = 0; i <= const_exp->val; i++)
      {
        if (i < init_val->exps->size())
        {
          out += "%ptr" + to_string(count_ptr) + " = getelemptr ";
          out += name + "_" + to_string(count_block);
          out += ", " + to_string(i) + "\n";
          if (init_val->exps->at(i)->name == "#")
            out += "store " + to_string(init_val->exps->at(i)->val) + ", " + name + "_" + to_string(count_block) + "\n";
          else
            out += "store " + init_val->exps->at(i)->name + ", " + name + "_" + to_string(count_block) + "\n";
          count_ptr++;
        }
      }
    }
  }

  void list_the_param(string &out) override {}
};

// LVal - ident
class LValAST_1 : public BaseAST
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
    VarUnion var_u = top_symbol_map->find(tmp_name);
    assert(var_u.kind != var_kind_ERROR);
    if (var_u.kind == var_kind_CONST)
    {
      name = "#";
      if (var_u.type.type == "i32" && var_u.type.array_len.size() == 0)
        val = (var_u.val)[0];
    }
    else
    {
      if (var_u.var_is_func_param == 1)
      {
        name = "%" + ident;
        out += name + " = alloc i32\n";
        out += "store " + tmp_name + ", " + name + "\n";
        top_symbol_map->erase(tmp_name);
        var_u.var_is_func_param = 2;
        top_symbol_map->insert(tmp_name, var_u);
      }
      else if (var_u.var_is_func_param == 2)
      {
        name = "%" + ident;
      }
      else
      {
        name = tmp_name + "_" + to_string(var_u.def_block_id);
      }
    }
  }

  void list_the_param(string &out) override {}
};

// LVal - IDENT "[" Exp "]"
class LValAST_2 : public BaseAST
{
public:
  string ident;
  unique_ptr<BaseAST> exp;

  void Dump() const override
  {
    cout << ident;
  }

  void printIR(string &out) override
  {
    exp->printIR(out);

    string tmp_name = "@" + ident;
    VarUnion var_u = top_symbol_map->find(tmp_name);
    assert(var_u.kind != var_kind_ERROR);

    if (var_u.kind == var_kind_CONST)
    {
      name = "%" + to_string(count_var);
      count_var++;

      string tmp_ptr_name = "%ptr" + to_string(count_ptr);
      count_ptr++;

      out += tmp_ptr_name + " = getelemptr " + tmp_name + "_" + to_string(var_u.def_block_id);
      if (exp->name == "#")
        out += ", " + to_string(exp->val) + "\n";
      else
        out += ", " + exp->name + "\n";

      out += name + " = load " + tmp_ptr_name;
    }
    // TODO
    // else
    // {
    //   if (var_u.var_is_func_param == 1)
    //   {
    //     name = "%" + ident;
    //     out += name + " = alloc i32\n";
    //     out += "store " + tmp_name + ", " + name + "\n";
    //     top_symbol_map->erase(tmp_name);
    //     var_u.var_is_func_param = 2;
    //     top_symbol_map->insert(tmp_name, var_u);
    //   }
    //   else if (var_u.var_is_func_param == 2)
    //   {
    //     name = "%" + ident;
    //   }
    //   else
    //   {
    //     name = tmp_name + "_" + to_string(var_u.def_block_id);
    //   }
    // }
  }

  void list_the_param(string &out) override {}
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

  void list_the_param(string &out) override {}
};
