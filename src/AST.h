#pragma once

#include <iostream>
#include <string>
#include <cstring>
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

// the koopa IR
extern string output_IR;

// BaseAST
class BaseAST
{
public:
  // name of AST node, "#" if no name
  string name = "#";

  // val of AST node, for exp
  int val;

  virtual ~BaseAST() = default;

  // debug
  virtual void Dump() const = 0;

  // generate IR
  virtual void printIR() = 0;

  // list the param，only overwrite in FuncRParamsAST
  virtual void list_the_param() {}

  // put array init val (complete) in a deque，only overwrite in ConstInitValAST_2, InitValAST_2
  // BaseAst is a const_exp(ConstInitValAST_2) or exp(InitValAST_2) or a new BaseAST and its val is 0
  virtual deque<BaseAST *> *get_array_aggregate(deque<int> array_len) { return nullptr; }

  string get_array_type_in_IR(deque<int> array_len, int i)
  {
    if (array_len.size() == (i + 1))
      return "[i32, " + to_string(array_len.at(i)) + "]";
    else
      return "[" + get_array_type_in_IR(array_len, i + 1) + ", " + to_string(array_len.at(i)) + "]";
  }

  // complete array init val -> string aggregate
  string trans_aggregate(deque<int> array_len, deque<BaseAST *> *array_init_val, int &ind)
  {
    string res;
    if (array_len.size() == 1)
    {
      res += "{";
      for (int i = 0; i < array_len.back(); i++)
      {
        if (i != 0)
        {
          res += ",";
        }
        res += to_string(array_init_val->at(ind)->val);
        ind++;
      }
      res += "}";
    }
    else
    {
      deque<int> tmp_array_len;
      for (int i = 1; i < array_len.size(); i++)
      {
        tmp_array_len.push_back(array_len.at(i));
      }

      res += "{";
      for (int i = 0; i < array_len.at(0); i++)
      {
        if (i != 0)
        {
          res += ",";
        }
        res += trans_aggregate(tmp_array_len, array_init_val, ind);
      }
      res += "}";
    }
    return res;
  }

  void store_init_array(deque<int> array_len, int &ind, deque<BaseAST *> *array_init_val, string base_name)
  {
    if (array_len.size() == 1)
    {
      // deepest
      for (int i = 0; i < array_len.at(0); i++)
      {
        string ptr_name = "%ptr" + to_string(count_ptr++);
        output_IR += ptr_name + " = getelemptr " + base_name;
        output_IR += ", " + to_string(i) + "\n";
        if (array_init_val == nullptr)
        {
          // zero init
          output_IR += "store " + to_string(0) + ", " + ptr_name + "\n";
        }
        else
        {
          output_IR += "store " + to_string(array_init_val->at(ind)->val) + ", " + ptr_name + "\n";
        }
        ind++;
      }
    }
    else
    {
      deque<int> tmp_arr_len;
      for (int i = 1; i < array_len.size(); i++)
      {
        tmp_arr_len.push_back(array_len.at(i));
      }

      for (int i = 0; i < array_len.at(0); i++)
      {
        string ptr_name = "%ptr" + to_string(count_ptr++);
        output_IR += ptr_name + " = getelemptr " + base_name;
        output_IR += ", " + to_string(i) + "\n";
        store_init_array(tmp_arr_len, ind, array_init_val, ptr_name);
      }
    }
  }

  virtual void init_func_param() {}
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

  void printIR() override
  {
    // lib func declaration
    output_IR += "decl @getint(): i32\n";
    output_IR += "decl @getch(): i32\n";
    output_IR += "decl @getarray(*i32): i32\n";
    output_IR += "decl @putint(i32)\n";
    output_IR += "decl @putch(i32)\n";
    output_IR += "decl @putarray(i32, *i32)\n";
    output_IR += "decl @starttime()\n";
    output_IR += "decl @stoptime()\n\n";

    comp_unit->printIR();
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

  void printIR() override
  {

    if (comp_unit != nullptr)
    {
      comp_unit->printIR();
      output_IR += "\n";
    }
    func_def_or_decl->printIR();
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

  void printIR() override
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

    output_IR += "fun @";
    output_IR += ident;
    output_IR += "(";
    if (func_f_params != nullptr)
    {
      func_f_params->printIR();
    }
    output_IR += ")";
    if (func_type == "i32")
      output_IR += ": " + func_type;
    output_IR += " {\n";

    output_IR += "%entry:\n";

    if (func_f_params != nullptr)
    {
      func_f_params->init_func_param();
    }

    block->printIR();

    if (block->name != "ret")
    {
      if (func_type == "")
        output_IR += "ret\n";
      if (func_type == "i32")
        output_IR += "ret 0\n";
    }
    output_IR += "}\n";

    top_symbol_map = top_symbol_map->outer_map;
  }
};

// FuncFParams - func_f_param {"," func_f_param}
class FuncFParamsAST : public BaseAST
{
public:
  deque<unique_ptr<BaseAST>> *func_f_params;

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

  void printIR() override
  {
    for (int i = 0; i < func_f_params->size(); i++)
    {
      func_f_params->at(i)->printIR();
      if (i != func_f_params->size() - 1)
        output_IR += ", ";
    }
  }

  void init_func_param() override
  {
    for (int i = 0; i < func_f_params->size(); i++)
    {
      func_f_params->at(i)->init_func_param();
    }
  }
};

// FuncFParam - BType IDENT;
class FuncFParamAST_1 : public BaseAST
{
public:
  string b_type; // int - "i32"
  string ident;

  void Dump() const override
  {
    cout << b_type << " " << ident;
  }

  void printIR() override
  {
    string param_name = "@" + ident;
    VarUnion param_tmp;
    param_tmp.kind = var_kind_VAR;
    param_tmp.type.type = b_type;
    param_tmp.def_block_id = count_block;
    param_tmp.var_is_func_param = 1;
    top_symbol_map->insert(param_name, param_tmp);

    output_IR += param_name + ": " + b_type;
  }

  void init_func_param() override
  {
    string tmp_name = "@" + ident;
    VarUnion var_u = top_symbol_map->find(tmp_name);
    // assert(var_u.kind != var_kind_ERROR);

    name = "%" + ident;
    output_IR += name + " = alloc " + var_u.type.type + "\n";
    output_IR += "store " + tmp_name + ", " + name + "\n";
    top_symbol_map->erase(tmp_name);
    var_u.var_is_func_param = 2;
    top_symbol_map->insert(tmp_name, var_u);
  }
};

// FuncFParam - BType IDENT [ "[" "]" { "[" ConstExp "] "}];
class FuncFParamAST_2 : public BaseAST
{
public:
  string b_type; // int - "i32"
  string ident;
  deque<unique_ptr<BaseAST>> *const_exps;

  void Dump() const override
  {
    cout << b_type << " " << ident;
  }

  void printIR() override
  {

    for (int i = 0; i < const_exps->size(); i++)
    {
      const_exps->at(i)->printIR();
      // assert(const_exps->at(i)->name == "#");
    }

    string param_name = "@" + ident;
    VarUnion param_tmp;
    param_tmp.kind = var_kind_VAR;

    if (const_exps->size() == 0)
    {
      param_tmp.type.type = "*" + b_type;
    }
    else
    {
      for (int i = 0; i < const_exps->size(); i++)
      {
        param_tmp.type.array_len.push_back(const_exps->at(i)->val);
      }
      param_tmp.type.type = "*" + get_array_type_in_IR(param_tmp.type.array_len, 0);
    }

    param_tmp.type.array_len.push_front(-1);

    param_tmp.def_block_id = count_block;
    param_tmp.var_is_func_param = 1;
    top_symbol_map->insert(param_name, param_tmp);

    output_IR += param_name + ": " + param_tmp.type.type;
  }

  void init_func_param() override
  {
    string tmp_name = "@" + ident;
    VarUnion var_u = top_symbol_map->find(tmp_name);
    // assert(var_u.kind != var_kind_ERROR);

    name = "%" + ident;
    output_IR += name + " = alloc " + var_u.type.type + "\n";
    output_IR += "store " + tmp_name + ", " + name + "\n";
    top_symbol_map->erase(tmp_name);
    var_u.var_is_func_param = 2;
    top_symbol_map->insert(tmp_name, var_u);
  }
};

// Block - "{" {bock_item} "}"
class BlockAST : public BaseAST
{
public:
  deque<unique_ptr<BaseAST>> *block_items;

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

  void printIR() override
  {
    Multi_Symbol_Map *new_top = new Multi_Symbol_Map;
    new_top->outer_map = top_symbol_map;
    top_symbol_map = new_top;

    count_block++;
    for (int i = 0; i < block_items->size(); i++)
    {
      block_items->at(i)->printIR();
      if (block_items->at(i)->name == "ret")
      {
        name = "ret";
        break;
      }
    }

    top_symbol_map = top_symbol_map->outer_map;
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

  void printIR() override
  {
    decl_or_stmt->printIR();
    name = decl_or_stmt->name;
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

  void printIR() override
  {
    match_stmt->printIR();
    name = match_stmt->name;
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

  void printIR() override
  {
    unmatch_stmt->printIR();
    name = unmatch_stmt->name;
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

  void printIR() override
  {
    name = "ret";
    if (exp != nullptr)
    {
      exp->printIR();
      if (exp->name == "#")
      {
        output_IR += "ret ";
        output_IR += (to_string(exp->val));
        output_IR += "\n";
      }
      else
      {
        output_IR += "ret ";
        output_IR += (exp->name);
        output_IR += "\n";
      }
    }
    else
    {
      output_IR += "ret\n";
    }
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

  void printIR() override
  {
    l_val->printIR();
    // assert(l_val->name != "#");

    string l_val_name = l_val->name;

    if (l_val->name[0] == 'a')
    {
      // array
      string l_val_name = "%ptr" + to_string(count_ptr);
      count_ptr++;
      output_IR += l_val_name + " = getelemptr " + (l_val->name).substr(1, (l_val->name).length()) + ", 0\n";
    }

    exp->printIR();
    if (exp->name == "#")
    {
      output_IR += "store ";
      output_IR += to_string(exp->val).c_str();
      output_IR += ", ";
      output_IR += l_val_name;
      output_IR += "\n";
    }
    else
    {
      output_IR += "store ";
      output_IR += exp->name;
      output_IR += ", ";
      output_IR += l_val_name;
      output_IR += "\n";
    }
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

  void printIR() override
  {
    if (exp != nullptr)
    {
      exp->printIR();
    }
  }
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

  void printIR() override
  {
    block->printIR();
    name = block->name;
  }
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

  void printIR() override
  {
    int if_else_id = count_if;
    count_if++;

    exp->printIR();

    if (exp->name == "#")
    {
      // exp is const
      output_IR += "jump %const_if_" + to_string(if_else_id) + "\n";
      output_IR += "\n%const_if_" + to_string(if_else_id) + ":\n";
      if (exp->val)
      {
        match_stmt_if->printIR();
        if (match_stmt_if->name != "ret")
          output_IR += "jump %const_end_" + to_string(if_else_id) + "\n";
      }
      else
      {
        match_stmt_else->printIR();
        if (match_stmt_else->name != "ret")
          output_IR += "jump %const_end_" + to_string(if_else_id) + "\n";
      }
      output_IR += "\n%const_end_" + to_string(if_else_id) + ":\n";
    }
    else
    {
      // exp is a var
      output_IR += "br ";
      output_IR += exp->name;
      output_IR += ", %if_" + to_string(if_else_id);
      output_IR += ", %else_" + to_string(if_else_id);
      output_IR += "\n";

      output_IR += "\n%if_" + to_string(if_else_id);
      output_IR += ":\n";
      match_stmt_if->printIR();
      if (match_stmt_if->name != "ret")
        output_IR += "jump %end_" + to_string(if_else_id) + "\n";

      output_IR += "\n%else_" + to_string(if_else_id);
      output_IR += ":\n";
      match_stmt_else->printIR();
      if (match_stmt_else->name != "ret")
        output_IR += "jump %end_" + to_string(if_else_id) + "\n";

      output_IR += "\n%end_" + to_string(if_else_id) + ":\n";
    }
  }
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

  void printIR() override
  {
    int while_id = count_if;
    count_if++;

    now_in_while.push_back(while_id);

    exp->printIR();
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
        output_IR += "br 1, %while_body_" + to_string(while_id) + ", %while_end_" + to_string(while_id) + "\n";
        output_IR += "\n%while_entry_" + to_string(while_id) + ":\n";
        output_IR += "br 1, %while_body_" + to_string(while_id) + ", %while_end_" + to_string(while_id) + "\n";
        output_IR += "\n%while_body_" + to_string(while_id) + ":\n";
        stmt->printIR();
        if (stmt->name != "ret")
          output_IR += "jump %while_entry_" + to_string(while_id) + "\n";
        output_IR += "\n%while_end_" + to_string(while_id) + ":\n";
        now_in_while.pop_back();
        return;
      }
    }
    else
    {
      output_IR += "br " + exp->name + ", " + "%while_body_" + to_string(while_id) + ", %while_end_" + to_string(while_id) + "\n";
      output_IR += "\n%while_entry_" + to_string(while_id) + ":\n";
      exp->printIR();
      output_IR += "br " + exp->name + ", " + "%while_body_" + to_string(while_id) + ", %while_end_" + to_string(while_id) + "\n";
      output_IR += "\n%while_body_" + to_string(while_id) + ":\n";
      stmt->printIR();
      if (stmt->name != "ret")
        output_IR += "jump %while_entry_" + to_string(while_id) + "\n";
      output_IR += "\n%while_end_" + to_string(while_id) + ":\n";
      now_in_while.pop_back();
      return;
    }
  }
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

  void printIR() override
  {
    output_IR += "jump %while_end_" + to_string(now_in_while.back()) + "\n";
    output_IR += "\n%while_body_" + to_string(now_in_while.back()) + "_" + to_string(count_break_continue) + ":\n";
    count_break_continue++;
  }
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

  void printIR() override
  {
    output_IR += "jump %while_entry_" + to_string(now_in_while.back()) + "\n";
    output_IR += "\n%while_body_" + to_string(now_in_while.back()) + "_" + to_string(count_break_continue) + ":\n";
    count_break_continue++;
  }
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

  void printIR() override
  {
    int if_else_id = count_if;
    count_if++;

    exp->printIR();
    if (exp->name == "#")
    {
      // exp is const
      if (exp->val)
      {
        // if(1)
        output_IR += "jump %const_if_" + to_string(if_else_id) + "\n";
        output_IR += "\n%const_if_" + to_string(if_else_id) + ":\n";
        stmt->printIR();
        if (stmt->name != "ret")
        {
          output_IR += "jump %const_end_" + to_string(if_else_id) + "\n";
        }
        output_IR += "\n%const_end_" + to_string(if_else_id) + ":\n";
      }
      // if(0) nothing
    }
    else
    {
      // exp is a var
      output_IR += "br ";
      output_IR += exp->name;
      output_IR += ", %if_" + to_string(if_else_id);
      output_IR += ", %end_" + to_string(if_else_id);
      output_IR += "\n";

      output_IR += "\n%if_" + to_string(if_else_id);
      output_IR += ":\n";
      stmt->printIR();
      if (stmt->name != "ret")
        output_IR += "jump %end_" + to_string(if_else_id) + "\n";

      output_IR += "\n%end_" + to_string(if_else_id);
      output_IR += ":\n";
    }
  }
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

  void printIR() override
  {
    int if_else_id = count_if;
    count_if++;

    exp->printIR();
    if (exp->name == "#")
    {
      // exp is const
      output_IR += "jump %const_if_" + to_string(if_else_id) + "\n";
      output_IR += "\n%const_if_" + to_string(if_else_id) + ":\n";
      if (exp->val)
      {
        match_stmt->printIR();
        if (match_stmt->name != "ret")
        {
          output_IR += "jump %const_end_" + to_string(if_else_id) + "\n";
        }
      }
      else
      {
        unmatch_stmt->printIR();
        if (unmatch_stmt->name != "ret")
        {
          output_IR += "jump %const_end_" + to_string(if_else_id) + "\n";
        }
      }
      output_IR += "\n%const_end_" + to_string(if_else_id) + ":\n";
    }
    else
    {
      // exp is a var
      output_IR += "br ";
      output_IR += exp->name;
      output_IR += ", %if_" + to_string(if_else_id);
      output_IR += ", %else_" + to_string(if_else_id);
      output_IR += "\n";

      output_IR += "\n%if_" + to_string(if_else_id);
      output_IR += ":\n";
      match_stmt->printIR();
      if (match_stmt->name != "ret")
        output_IR += "jump %end_" + to_string(if_else_id) + "\n";

      output_IR += "\n%else_" + to_string(if_else_id);
      output_IR += ":\n";
      unmatch_stmt->printIR();
      if (unmatch_stmt->name != "ret")
        output_IR += "jump %end_" + to_string(if_else_id) + "\n";

      output_IR += "\n%end_" + to_string(if_else_id);
      output_IR += ":\n";
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

  void printIR() override
  {
    unary_exp->printIR();
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

  void printIR() override
  {
    add_exp->printIR();
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

  void printIR() override
  {
    lor_exp->printIR();
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

  void printIR() override
  {
    exp->printIR();
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

  void printIR() override
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

  void printIR() override
  {
    l_val->printIR();

    if (l_val->name[0] == 'a')
    {
      // array
      name = (l_val->name).substr(1, (l_val->name).length());
      name = "%ptr" + to_string(count_ptr);
      count_ptr++;
      output_IR += name + " = getelemptr " + (l_val->name).substr(1, (l_val->name).length()) + ", 0\n";
      return;
    }

    if (l_val->name == "#")
    {
      name = "#";
      val = l_val->val;
    }
    else
    {
      name = "%" + to_string(count_var);
      output_IR += name;
      output_IR += " = load ";
      output_IR += l_val->name;
      output_IR += "\n";
      count_var++;
    }
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

  void printIR() override
  {
    primary_exp->printIR();
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

  void printIR() override
  {
    unary_exp->printIR();

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
        output_IR += name.c_str();
        output_IR += " = sub 0, ";
        output_IR += (unary_exp->name).c_str();
        output_IR += "\n";
        count_var++;
        break;
      case '!':
        name = "%" + to_string(count_var);
        output_IR += name.c_str();
        output_IR += " = eq 0, ";
        output_IR += (unary_exp->name).c_str();
        output_IR += "\n";
        count_var++;
        break;
      }
    }
  }
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

  void printIR() override
  {
    if (func_r_params != nullptr)
    {
      func_r_params->printIR();
    }

    string fuc_name = "@" + ident;
    VarUnion tmp_func = top_symbol_map->find(fuc_name);
    if (tmp_func.type.type != "")
    {
      name = "%" + to_string(count_var);
      count_var++;
      output_IR += name + " = ";
    }
    output_IR += "call " + fuc_name + "(";
    if (func_r_params != nullptr)
    {
      func_r_params->list_the_param();
    }
    output_IR += ")\n";
  }
};

// FuncRParams ::= Exp {"," Exp}
class FuncRParamsAST : public BaseAST
{
public:
  deque<unique_ptr<BaseAST>> *exps = nullptr;

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

  void printIR() override
  {
    if (exps != nullptr)
    {
      for (int i = 0; i < exps->size(); i++)
      {
        exps->at(i)->printIR();
      }
    }
  }

  void list_the_param() override
  {
    if (exps != nullptr)
    {
      for (int i = 0; i < exps->size(); i++)
      {
        if (exps->at(i)->name == "#")
        {
          output_IR += to_string(exps->at(i)->val);
        }
        else
        {
          output_IR += exps->at(i)->name;
        }
        if (i != exps->size() - 1)
        {
          output_IR += ", ";
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

  void printIR() override
  {
    unary_exp->printIR();
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

  void printIR() override
  {
    mul_exp->printIR();
    unary_exp->printIR();

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
    output_IR += name.c_str();

    switch (binary_op[0])
    {
    case '*':
      output_IR += " = mul ";
      break;
    case '/':
      output_IR += " = div ";
      break;
    case '%':
      output_IR += " = mod ";
      break;
    }

    if (mul_exp->name == "#")
      output_IR += (to_string(mul_exp->val)).c_str();
    else
      output_IR += (mul_exp->name).c_str();
    output_IR += ", ";
    if (unary_exp->name == "#")
      output_IR += (to_string(unary_exp->val)).c_str();
    else
      output_IR += (unary_exp->name).c_str();
    output_IR += "\n";
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

  void printIR() override
  {
    mul_exp->printIR();
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

  void printIR() override
  {
    add_exp->printIR();
    mul_exp->printIR();

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
    output_IR += name.c_str();

    switch (binary_op[0])
    {
    case '+':
      output_IR += " = add ";
      break;
    case '-':
      output_IR += " = sub ";
      break;
    }

    if (add_exp->name == "#")
      output_IR += (to_string(add_exp->val)).c_str();
    else
      output_IR += (add_exp->name).c_str();
    output_IR += ", ";
    if (mul_exp->name == "#")
      output_IR += (to_string(mul_exp->val)).c_str();
    else
      output_IR += (mul_exp->name).c_str();
    output_IR += "\n";
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

  void printIR() override
  {
    add_exp->printIR();
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

  void printIR() override
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

    rel_exp->printIR();
    add_exp->printIR();

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
    output_IR += name.c_str();

    switch (type)
    {
    case '<':
      output_IR += " = lt ";
      break;
    case '>':
      output_IR += " = gt ";
      break;
    case '<' + '=':
      output_IR += " = le ";
      break;
    case '>' + '=':
      output_IR += " = ge ";
      break;
    }

    if (rel_exp->name == "#")
      output_IR += (to_string(rel_exp->val)).c_str();
    else
      output_IR += (rel_exp->name).c_str();
    output_IR += ", ";
    if (add_exp->name == "#")
      output_IR += (to_string(add_exp->val)).c_str();
    else
      output_IR += (add_exp->name).c_str();
    output_IR += "\n";
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

  void printIR() override
  {
    rel_exp->printIR();
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

  void printIR() override
  {
    eq_exp->printIR();
    rel_exp->printIR();

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
    output_IR += name.c_str();

    switch (eq_op[0])
    {
    case '=':
      output_IR += " = eq ";
      break;
    case '!':
      output_IR += " = ne ";
      break;
    }

    if (eq_exp->name == "#")
      output_IR += (to_string(eq_exp->val)).c_str();
    else
      output_IR += (eq_exp->name).c_str();
    output_IR += ", ";
    if (rel_exp->name == "#")
      output_IR += (to_string(rel_exp->val)).c_str();
    else
      output_IR += (rel_exp->name).c_str();
    output_IR += "\n";
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

  void printIR() override
  {
    eq_exp->printIR();
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

  void printIR() override
  {
    land_exp->printIR();

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
        eq_exp->printIR();
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
          output_IR += name + " = ne 0, " + eq_exp->name + "\n";
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

      output_IR += tmp_var_name + " = alloc i32\n";
      output_IR += "br " + land_exp->name + ", %if_" + to_string(if_else_id) + ", %else_" + to_string(if_else_id) + "\n";

      output_IR += "\n%if_" + to_string(if_else_id) + ":\n";
      eq_exp->printIR();
      if (eq_exp->name == "#")
      {
        output_IR += "store " + to_string((eq_exp->val) != 0) + ", " + tmp_var_name + "\n";
      }
      else
      {
        string tmp_name = "%" + to_string(count_var);
        count_var++;
        output_IR += tmp_name + " = ne 0, " + eq_exp->name + "\n";
        output_IR += "store " + tmp_name + ", " + tmp_var_name + "\n";
      }
      output_IR += "jump %end_" + to_string(if_else_id) + "\n";

      output_IR += "\n%else_" + to_string(if_else_id) + ":\n";
      output_IR += "store 0, " + tmp_var_name + "\n";
      output_IR += "jump %end_" + to_string(if_else_id) + "\n";

      output_IR += "\n%end_" + to_string(if_else_id) + ":\n";

      name = "%" + to_string(count_var);
      count_var++;
      output_IR += name + " = load " + tmp_var_name + "\n";
    }
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

  void printIR() override
  {
    land_exp->printIR();
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

  void printIR() override
  {
    lor_exp->printIR();

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
        land_exp->printIR();
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
          output_IR += name + " = ne 0, " + land_exp->name + "\n";
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

      output_IR += tmp_var_name + " = alloc i32\n";

      output_IR += "br " + lor_exp->name + ", %if_" + to_string(if_else_id) + ", %else_" + to_string(if_else_id) + "\n";
      output_IR += "\n%if_" + to_string(if_else_id) + ":\n";
      output_IR += "store 1, " + tmp_var_name + "\n";
      output_IR += "jump %end_" + to_string(if_else_id) + "\n";
      output_IR += "\n%else_" + to_string(if_else_id) + ":\n";
      land_exp->printIR();
      if (land_exp->name == "#")
      {
        output_IR += "store " + to_string((land_exp->val) != 0) + ", " + tmp_var_name + "\n";
      }
      else
      {
        string tmp_name = "%" + to_string(count_var);
        count_var++;
        output_IR += tmp_name + " = ne 0, " + land_exp->name + "\n";
        output_IR += "store " + tmp_name + ", " + tmp_var_name + "\n";
      }
      output_IR += "jump %end_" + to_string(if_else_id) + "\n";
      output_IR += "\n%end_" + to_string(if_else_id) + ":\n";

      name = "%" + to_string(count_var);
      count_var++;
      output_IR += name + " = load " + tmp_var_name + "\n";
    }
  }
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

  void printIR() override
  {
    const_decl->printIR();
  }
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

  void printIR() override
  {
    var_decl->printIR();
  }
};

// ConstDecl - "const" int const_def {"," const_def} ";"
class ConstDeclAST : public BaseAST
{
public:
  deque<unique_ptr<BaseAST>> *const_defs;

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

  void printIR() override
  {
    for (int i = 0; i < const_defs->size(); i++)
    {
      const_defs->at(i)->printIR();
    }
  }
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

  void printIR() override
  {
    const_exp->printIR();
    // assert(const_exp->name == "#");
    val = const_exp->val;
  }
};

// ConstInitVal - "{" [const_init_val {"," const_init_val}] "}"
class ConstInitValAST_2 : public BaseAST
{
public:
  deque<unique_ptr<BaseAST>> *const_init_vals;

  void Dump() const override
  {
    for (int i = 0; i < const_init_vals->size(); i++)
    {
      const_init_vals->at(i)->Dump();
      cout << ", ";
    }
  }

  void printIR() override
  {
    name = "{";
    for (int i = 0; i < const_init_vals->size(); i++)
    {
      const_init_vals->at(i)->printIR();
    }
  }

  deque<BaseAST *> *get_array_aggregate(deque<int> array_len) override
  {
    int length = 1;
    for (int i = 0; i < array_len.size(); i++)
    {
      length *= array_len.at(i);
    }

    deque<BaseAST *> *res = new deque<BaseAST *>();
    for (int i = 0; i < const_init_vals->size(); i++)
    {
      // is a number
      if (const_init_vals->at(i)->name != "{")
      {
        res->push_back(const_init_vals->at(i).release());
      }
      // is a list
      else
      {
        int tmp_layer = array_len.size() - 1;
        int mul = array_len.at(tmp_layer);
        // assert(res->size() % array_len.at(tmp_layer) == 0);
        // find the largest layer
        while (1)
        {
          // assert(tmp_layer != 0);
          if ((res->size() % (mul * array_len.at(tmp_layer - 1))) == 0)
          {
            if ((tmp_layer - 1) == 0)
            {
              break;
            }

            mul *= array_len.at(tmp_layer - 1);
            tmp_layer--;
            continue;
          }
          else
          {
            break;
          }
        }
        deque<int> tmp_arr_len;
        for (; tmp_layer < array_len.size(); tmp_layer++)
        {
          tmp_arr_len.push_back(array_len.at(tmp_layer));
        }
        deque<BaseAST *> *inner_list = const_init_vals->at(i)->get_array_aggregate(tmp_arr_len);
        for (int j = 0; j < inner_list->size(); j++)
        {
          res->push_back(inner_list->at(j));
        }
      }
    }
    while (res->size() < length)
    {
      BaseAST *zero = new ROOTAST();
      zero->name = "#";
      zero->val = 0;
      res->push_back(zero);
    }
    return res;
  }
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

  void printIR() override
  {
    name = "@" + ident;
    string b_type = "i32";
    const_init_val->printIR();
    // assert(const_init_val->name == "#");
    VarUnion const_tmp;
    const_tmp.kind = var_kind_CONST;
    const_tmp.type.type = b_type;
    const_tmp.val.push_back(const_init_val->val);
    const_tmp.def_block_id = count_block;
    top_symbol_map->insert(name, const_tmp);
  }
};

// ConstDef - ident "[" const_exp "]" { "[" const_exp "]" } "=" const_init_val
// this is (multi)array
class ConstDefAST_2 : public BaseAST
{
public:
  string ident;
  deque<unique_ptr<BaseAST>> *const_exps;
  unique_ptr<BaseAST> const_init_val;

  void Dump() const override
  {
    cout << ident << " = ";
    const_init_val->Dump();
  }

  void printIR() override
  {
    name = "@" + ident;
    string b_type = "i32";

    for (int i = 0; i < const_exps->size(); i++)
    {
      const_exps->at(i)->printIR();
      // assert(const_exps->at(i)->name == "#");
    }

    const_init_val->printIR();

    VarUnion const_tmp;
    if (top_symbol_map->outer_map == nullptr)
    {
      const_tmp.kind = var_kind_CONST;
      const_tmp.type.type = b_type;
      for (int i = 0; i < const_exps->size(); i++)
      {
        const_tmp.type.array_len.push_back(const_exps->at(i)->val);
      }
      // const arrays don't have a val
      // const_tmp.val.push_back(const_init_val->val);
      const_tmp.def_block_id = count_block;
      top_symbol_map->insert(name, const_tmp);

      deque<BaseAST *> *aggregate_ptr = const_init_val->get_array_aggregate(const_tmp.type.array_len);
      // assert(aggregate_ptr != nullptr);

      int tmp_ind = 0;
      string array_type = get_array_type_in_IR(const_tmp.type.array_len, 0);
      string aggregate = trans_aggregate(const_tmp.type.array_len, aggregate_ptr, tmp_ind);

      output_IR += "global " + name + "_" + to_string(count_block);
      output_IR += " = alloc " + array_type + ", " + aggregate + "\n";
    }
    else
    {
      const_tmp.kind = var_kind_CONST;
      const_tmp.type.type = b_type;
      for (int i = 0; i < const_exps->size(); i++)
      {
        const_tmp.type.array_len.push_back(const_exps->at(i)->val);
      }
      const_tmp.val.push_back(const_init_val->val);
      const_tmp.def_block_id = count_block;
      top_symbol_map->insert(name, const_tmp);

      output_IR += name + "_" + to_string(count_block);
      output_IR += " = alloc " + get_array_type_in_IR(const_tmp.type.array_len, 0) + "\n";

      deque<BaseAST *> *aggregate_ptr = const_init_val->get_array_aggregate(const_tmp.type.array_len);
      // assert(aggregate_ptr != nullptr);

      int tmp_ind = 0;
      store_init_array(const_tmp.type.array_len, tmp_ind, aggregate_ptr, (name + "_" + to_string(count_block)));
    }
  }
};

// VarDecl - int var_def {"," var_def} ";"
class VarDeclAST : public BaseAST
{
public:
  deque<unique_ptr<BaseAST>> *var_defs;

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

  void printIR() override
  {
    for (int i = 0; i < var_defs->size(); i++)
    {
      var_defs->at(i)->printIR();
    }
  }
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

  void printIR() override
  {
    exp->printIR();
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

// InitVal - "{" [init_val {"," init_val}] "}"
class InitValAST_2 : public BaseAST
{
public:
  deque<unique_ptr<BaseAST>> *init_vals;

  void Dump() const override
  {
    for (int i = 0; i < init_vals->size(); i++)
    {
      init_vals->at(i)->Dump();
      cout << ",";
    }
  }

  void printIR() override
  {
    name = "{";
    for (int i = 0; i < init_vals->size(); i++)
    {
      init_vals->at(i)->printIR();
    }
  }

  deque<BaseAST *> *get_array_aggregate(deque<int> array_len) override
  {
    int length = 1;
    for (int i = 0; i < array_len.size(); i++)
    {
      length *= array_len.at(i);
    }

    deque<BaseAST *> *res = new deque<BaseAST *>();
    for (int i = 0; i < init_vals->size(); i++)
    {
      // is a number
      if (init_vals->at(i)->name != "{")
      {
        res->push_back(init_vals->at(i).release());
      }
      // is a list
      else
      {
        int tmp_layer = array_len.size() - 1;
        int mul = array_len.at(tmp_layer);
        // assert(res->size() % array_len.at(tmp_layer) == 0);

        // find the biggest align bound
        while (1)
        {
          // assert(tmp_layer != 0);
          if ((res->size() % (mul * array_len.at(tmp_layer - 1))) == 0)
          {
            if ((tmp_layer - 1) == 0)
            {
              break;
            }

            mul *= array_len.at(tmp_layer - 1);
            tmp_layer--;
            continue;
          }
          else
          {
            break;
          }
        }

        deque<int> tmp_arr_len;
        for (; tmp_layer < array_len.size(); tmp_layer++)
        {
          tmp_arr_len.push_back(array_len.at(tmp_layer));
        }
        deque<BaseAST *> *inner_list = init_vals->at(i)->get_array_aggregate(tmp_arr_len);
        for (int j = 0; j < inner_list->size(); j++)
        {
          res->push_back(inner_list->at(j));
        }
      }
    }
    while (res->size() < length)
    {
      BaseAST *zero = new ROOTAST();
      zero->name = "#";
      zero->val = 0;
      res->push_back(zero);
    }
    return res;
  }
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

  void printIR() override
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

      output_IR += "global " + name + "_" + to_string(count_block);
      output_IR += " = alloc i32, zeroinit\n";
    }
    else
    {
      var_tmp.kind = var_kind_VAR;
      var_tmp.type.type = b_type;
      var_tmp.def_block_id = count_block;
      top_symbol_map->insert(name, var_tmp);

      output_IR += name + "_" + to_string(count_block);
      output_IR += " = alloc i32\n";
    }
  }
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

  void printIR() override
  {
    VarUnion var_tmp;
    string b_type = "i32";
    name = "@" + ident;

    if (top_symbol_map->outer_map == nullptr)
    {
      var_tmp.kind = var_kind_INIT_GLOBAL_VAR;
      var_tmp.type.type = b_type;
      var_tmp.def_block_id = count_block;

      init_val->printIR();
      // assert(init_val->name == "#");
      var_tmp.var_is_func_param = 0;

      top_symbol_map->insert(name, var_tmp);

      output_IR += "global " + name + "_" + to_string(count_block);
      output_IR += " = alloc i32, " + to_string(init_val->val) + "\n";
    }
    else
    {
      var_tmp.kind = var_kind_VAR;
      var_tmp.type.type = b_type;
      var_tmp.def_block_id = count_block;
      top_symbol_map->insert(name, var_tmp);

      output_IR += name + "_" + to_string(count_block);
      output_IR += " = alloc i32\n";

      init_val->printIR();

      if (init_val->name == "#")
      {
        output_IR += "store ";
        output_IR += to_string(init_val->val).c_str();
        output_IR += ", ";
        output_IR += name + "_" + to_string(count_block);
        output_IR += "\n";
      }
      else
      {
        output_IR += "store ";
        output_IR += init_val->name;
        output_IR += ", ";
        output_IR += name + "_" + to_string(count_block);
        output_IR += "\n";
      }
    }
  }
};

// VarDef - ident "[" const_exp "]" { "[" const_exp "]" }
class VarDefAST_3 : public BaseAST
{
public:
  string ident;
  deque<unique_ptr<BaseAST>> *const_exps;

  void Dump() const override
  {
    cout << ident;
  }

  void printIR() override
  {
    name = "@" + ident;
    string b_type = "i32";

    for (int i = 0; i < const_exps->size(); i++)
    {
      const_exps->at(i)->printIR();
      // assert(const_exps->at(i)->name == "#");
    }

    VarUnion var_tmp;
    if (top_symbol_map->outer_map == nullptr)
    {
      var_tmp.kind = var_kind_NOT_INIT_GLOBAL_VAR;
      var_tmp.type.type = b_type;
      for (int i = 0; i < const_exps->size(); i++)
      {
        var_tmp.type.array_len.push_back(const_exps->at(i)->val);
      }
      var_tmp.def_block_id = count_block;
      var_tmp.var_is_func_param = 0;
      top_symbol_map->insert(name, var_tmp);

      string array_type = get_array_type_in_IR(var_tmp.type.array_len, 0);

      output_IR += "global " + name + "_" + to_string(count_block);
      output_IR += " = alloc " + array_type + ", zeroinit\n";
    }
    else
    {
      var_tmp.kind = var_kind_VAR;
      var_tmp.type.type = b_type;
      for (int i = 0; i < const_exps->size(); i++)
      {
        var_tmp.type.array_len.push_back(const_exps->at(i)->val);
      }
      var_tmp.def_block_id = count_block;
      top_symbol_map->insert(name, var_tmp);

      string array_type = get_array_type_in_IR(var_tmp.type.array_len, 0);

      string full_arr_name = name + "_" + to_string(count_block);

      output_IR += name + "_" + to_string(count_block);
      output_IR += " = alloc " + array_type + "\n";

      int tmp_ind = 0;
      store_init_array(var_tmp.type.array_len, tmp_ind, nullptr, (name + "_" + to_string(count_block)));
    }
  }
};

// VarDef - ident "[" const_exp "]" { "[" const_exp "]" } "=" init_val
class VarDefAST_4 : public BaseAST
{
public:
  string ident;
  deque<unique_ptr<BaseAST>> *const_exps;
  unique_ptr<BaseAST> init_val;

  void Dump() const override
  {
    cout << ident;
  }

  void printIR() override
  {
    name = "@" + ident;
    string b_type = "i32";

    // assert(const_exps != nullptr);
    for (int i = 0; i < const_exps->size(); i++)
    {
      const_exps->at(i)->printIR();
      // assert(const_exps->at(i)->name == "#");
    }

    init_val->printIR();

    VarUnion var_tmp;
    if (top_symbol_map->outer_map == nullptr)
    {
      var_tmp.kind = var_kind_INIT_GLOBAL_VAR;
      var_tmp.type.type = b_type;
      for (int i = 0; i < const_exps->size(); i++)
      {
        var_tmp.type.array_len.push_back(const_exps->at(i)->val);
      }
      var_tmp.def_block_id = count_block;
      var_tmp.var_is_func_param = 0;
      top_symbol_map->insert(name, var_tmp);

      deque<BaseAST *> *aggregate_ptr = init_val->get_array_aggregate(var_tmp.type.array_len);
      // assert(aggregate_ptr != nullptr);

      int tmp_ind = 0;
      string array_type = get_array_type_in_IR(var_tmp.type.array_len, 0);
      string aggregate = trans_aggregate(var_tmp.type.array_len, aggregate_ptr, tmp_ind);

      output_IR += "global " + name + "_" + to_string(count_block);
      output_IR += " = alloc " + array_type + ", " + aggregate + "\n";
    }
    else
    {
      var_tmp.kind = var_kind_VAR;
      var_tmp.type.type = b_type;
      for (int i = 0; i < const_exps->size(); i++)
      {
        var_tmp.type.array_len.push_back(const_exps->at(i)->val);
      }
      var_tmp.def_block_id = count_block;
      top_symbol_map->insert(name, var_tmp);

      output_IR += name + "_" + to_string(count_block);
      output_IR += " = alloc " + get_array_type_in_IR(var_tmp.type.array_len, 0) + "\n";

      deque<BaseAST *> *aggregate_ptr = init_val->get_array_aggregate(var_tmp.type.array_len);
      // assert(aggregate_ptr != nullptr);

      int tmp_ind = 0;
      store_init_array(var_tmp.type.array_len, tmp_ind, aggregate_ptr, (name + "_" + to_string(count_block)));
    }
  }
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

  void printIR() override
  {
    string tmp_name = "@" + ident;
    VarUnion var_u = top_symbol_map->find(tmp_name);
    // assert(var_u.kind != var_kind_ERROR);
    if (var_u.kind == var_kind_CONST)
    {
      name = "#";
      if (var_u.type.type == "i32" && var_u.type.array_len.size() == 0)
        val = (var_u.val)[0];
      else
        name = "a" + tmp_name + "_" + to_string(var_u.def_block_id);
    }
    else
    {
      if (var_u.var_is_func_param == 1)
      {
        // assert(false);
      }
      else if (var_u.var_is_func_param == 2)
      {
        name = "%" + ident;
      }
      else
      {
        if (var_u.type.type == "i32" && var_u.type.array_len.size() != 0)
          name = "a" + tmp_name + "_" + to_string(var_u.def_block_id);
        else
          name = tmp_name + "_" + to_string(var_u.def_block_id);
      }
    }
  }
};

// LVal - ident "[" exp "]" { "[" exp "]" }
class LValAST_2 : public BaseAST
{
public:
  string ident;
  deque<unique_ptr<BaseAST>> *exps;

  void Dump() const override
  {
    cout << ident;
  }

  void printIR() override
  {
    for (int i = 0; i < exps->size(); i++)
    {
      exps->at(i)->printIR();
    }

    string tmp_name = "@" + ident;
    VarUnion var_u = top_symbol_map->find(tmp_name);
    // assert(var_u.kind != var_kind_ERROR);

    if (var_u.var_is_func_param == 1)
    {
      // assert(false);
    }
    else if (var_u.var_is_func_param == 2)
    {
      name = "%" + ident;
    }
    else
    {
      tmp_name = tmp_name + "_" + to_string(var_u.def_block_id);
    }

    if (var_u.var_is_func_param == 2)
    {
      tmp_name = "%" + to_string(count_var++);
      output_IR += tmp_name + " = load " + name + "\n";

      for (int i = 0; i < exps->size(); i++)
      {
        name = "%ptr" + to_string(count_ptr);
        count_ptr++;

        if (i == 0)
        {
          output_IR += name + " = getptr " + tmp_name;
        }
        else
        {
          output_IR += name + " = getelemptr " + tmp_name;
        }

        if (exps->at(i)->name == "#")
          output_IR += ", " + to_string(exps->at(i)->val) + "\n";
        else
          output_IR += ", " + exps->at(i)->name + "\n";
        tmp_name = name;
      }

      if (var_u.type.array_len.size() > exps->size())
      {
        name = "a" + tmp_name;
      }
    }
    else
    {
      for (int i = 0; i < exps->size(); i++)
      {
        name = "%ptr" + to_string(count_ptr);
        count_ptr++;

        output_IR += name + " = getelemptr " + tmp_name;
        if (exps->at(i)->name == "#")
          output_IR += ", " + to_string(exps->at(i)->val) + "\n";
        else
          output_IR += ", " + exps->at(i)->name + "\n";
        tmp_name = name;
      }
      if (var_u.type.array_len.size() > exps->size())
      {
        name = "a" + tmp_name;
      }
    }
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

  void printIR() override
  {
    exp->printIR();
    // assert(exp->name == "#");
    val = exp->val;
  }
};
