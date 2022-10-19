#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <cstring>
#include <map>
#include <set>
#include "AST.h"
#include "VisitIR.h"
#include "koopa.h"

using namespace std;

extern FILE *yyin;
extern int yyparse(unique_ptr<BaseAST> &ast);
FILE *out_file;

map<string, VarUnion> var_map;
Multi_Symbol_Map *top_symbol_map = new Multi_Symbol_Map;
string tmp_b_type = "";
deque<int> now_in_while;

void add_sysy_func()
{
  string ident[] = {
      "getint",
      "getch",
      "getarray",
      "putint",
      "putch",
      "putarray",
      "starttime",
      "stoptime"};
  string func_type[] = {
      "i32",
      "i32",
      "i32",
      "",
      "",
      "",
      "",
      ""};

  for (int i = 0; i < 8; i++)
  {
    string name = "@" + ident[i];
    VarUnion tmp_func;
    tmp_func.kind = var_kind_FUNC;
    tmp_func.type.type = func_type[i];
    top_symbol_map->insert(name, tmp_func);
  }
}

int main(int argc, const char *argv[])
{
  add_sysy_func();

  assert(argc == 5);
  auto mode = argv[1];
  auto input = argv[2];
  auto output = argv[4];

  // cout << mode << "\n";

  // open input file
  yyin = fopen(input, "r");
  assert(yyin);

  // call parser, parser will call lexer
  unique_ptr<BaseAST> ast;
  auto parse_ret = yyparse(ast);
  assert(!parse_ret);

  // debug
  // ast->Dump();

  // AST to IR
  string IRstring = "";
  ast->printIR(IRstring);
  const char *IRstr = IRstring.c_str();

  // koopa mode
  if (mode[1] == 'k')
  {
    out_file = fopen(output, "w+");
    assert(out_file);
    fprintf(out_file, "%s", IRstr);
    fclose(out_file);
    return 0;
  }

  // str Koopa to program Koopa
  koopa_program_t program;
  koopa_error_code_t koopa_ret = koopa_parse_from_string(IRstr, &program);
  assert(koopa_ret == KOOPA_EC_SUCCESS);
  koopa_raw_program_builder_t builder = koopa_new_raw_program_builder();
  // Koopa IR to raw program
  koopa_raw_program_t raw = koopa_build_raw_program(builder, program);
  koopa_delete_program(program);

  // risecv mode
  if (mode[1] == 'r')
  {
    // output redirection
    freopen(output, "w+", stdout);
  }
  Visit(raw);

  koopa_delete_raw_program_builder(builder);

  return 0;
}
