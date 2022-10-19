%code requires {
  #include <iostream>
  #include <memory>
  #include <string>
  #include <vector>
  #include <AST.h>
}

%{

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <AST.h>

// lexer and  yyerror
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// difine parser and yyerror's extra param
%parse-param { std::unique_ptr<BaseAST> &ast }

// yylval difination
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
  vector<unique_ptr<BaseAST>> *ast_list_val;
}


// lexer token's type
%token INT RETURN LE GE EQ NE LAND LOR CONST IF ELSE WHILE CONTINUE BREAK VOID
%token <str_val> IDENT
%token <int_val> INT_CONST

// non-termina's type
%type <ast_val>
CompUnit
FuncDef FuncFParams FuncFParam FuncRParams
Block BlockItem Stmt MatchStmt UnMatchStmt
Exp PrimaryExp UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp 
Decl LVal
ConstDecl ConstDef ConstInitVal ConstExp
VarDecl VarDef InitVal
%type <int_val> Number
%type <str_val> UnaryOp
%type <ast_list_val> BlockItem_list ConstDef_list VarDef_list FuncFParams_list Exp_list

%%

ROOT
  : CompUnit {
    auto root = make_unique<ROOTAST>();
    root->comp_unit = unique_ptr<BaseAST>($1);
    ast = move(root);
  }

CompUnit 
  : Decl {
    auto ast = new CompUnitAST();
    ast->func_def_or_decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | FuncDef {
    auto ast = new CompUnitAST();
    ast->func_def_or_decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | CompUnit FuncDef {
    auto ast = new CompUnitAST();
    ast->comp_unit = unique_ptr<BaseAST>($1);
    ast->func_def_or_decl = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | CompUnit Decl {
    auto ast = new CompUnitAST();
    ast->comp_unit = unique_ptr<BaseAST>($1);
    ast->func_def_or_decl = unique_ptr<BaseAST>($2);
    $$ = ast;
  }



FuncDef 
  : INT IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = "i32";
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | VOID IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = "";
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | INT IDENT '(' FuncFParams ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = "i32";
    ast->ident = *unique_ptr<string>($2);
    ast->func_f_params = unique_ptr<BaseAST>($4);
    ast->block = unique_ptr<BaseAST>($6);
    $$ = ast;
  }
  | VOID IDENT '(' FuncFParams ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = "";
    ast->ident = *unique_ptr<string>($2);
    ast->func_f_params = unique_ptr<BaseAST>($4);
    ast->block = unique_ptr<BaseAST>($6);
    $$ = ast;
  }

FuncFParams
  : FuncFParam FuncFParams_list {
    auto ast = new FuncFParamsAST();
    ($2)->insert(($2)->begin(),unique_ptr<BaseAST>($1));
    ast->func_f_params = ($2);
    $$ = ast;
  }

FuncFParams_list
  : ',' FuncFParam FuncFParams_list {
    ($3)->insert(($3)->begin(),unique_ptr<BaseAST>($2));
    $$ = ($3);
  }
  | {
    auto func_f_params = new vector<unique_ptr<BaseAST>>();
    $$ = func_f_params;
  }

FuncFParam
  : INT IDENT {
    auto ast = new FuncFParamAST();
    ast->b_type = "i32"; 
    ast->ident = *unique_ptr<string>($2);
    $$ = ast;
  }

Block 
  : '{' BlockItem_list '}' {
    auto ast = new BlockAST();
    ast->block_items=($2);
    $$ = ast;
  }

BlockItem_list
  : BlockItem_list BlockItem {
    ($1)->push_back(unique_ptr<BaseAST>($2));
    $$ = ($1);
  }
  | {
    auto block_items = new vector<unique_ptr<BaseAST>>();
    $$ = block_items;
  }

BlockItem 
  : Decl {
    auto ast = new BlockItemAST();
    ast->decl_or_stmt=unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | Stmt {
    auto ast = new BlockItemAST();
    ast->decl_or_stmt=unique_ptr<BaseAST>($1);
    $$ = ast;
  }

Stmt
  : MatchStmt {
    auto ast = new StmtAST_1();
    ast->match_stmt = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | UnMatchStmt {
    auto ast = new StmtAST_2();
    ast->unmatch_stmt = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

MatchStmt 
  : RETURN Exp ';' {
    auto ast = new MatchStmtAST_1();
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | RETURN ';' {
    auto ast = new MatchStmtAST_1();
    $$ = ast;
  }
  | LVal '=' Exp ';' {
    auto ast = new MatchStmtAST_2();
    ast->l_val = unique_ptr<BaseAST>($1);
    ast->exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | Exp ';' {
    auto ast = new MatchStmtAST_3();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | ';' {
    auto ast = new MatchStmtAST_3();
    $$ = ast;
  }
  | Block {
    auto ast = new MatchStmtAST_4();
    ast->block = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | IF '(' Exp ')' MatchStmt ELSE MatchStmt {
    auto ast = new MatchStmtAST_5();
    ast->exp = unique_ptr<BaseAST>($3);
    ast->match_stmt_if = unique_ptr<BaseAST>($5);
    ast->match_stmt_else = unique_ptr<BaseAST>($7);
    $$ = ast;
  }
  | WHILE '(' Exp ')' Stmt {
    auto ast = new MatchStmtAST_6();
    ast->exp = unique_ptr<BaseAST>($3);
    ast->stmt = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | BREAK ';' {
    auto ast = new MatchStmtAST_7();
    $$ = ast;
  }
  | CONTINUE ';' {
    auto ast = new MatchStmtAST_8();
    $$ = ast;
  }

UnMatchStmt 
  : IF '(' Exp ')' Stmt {
    auto ast = new UnMatchStmtAST_1();
    ast->exp = unique_ptr<BaseAST>($3);
    ast->stmt = unique_ptr<BaseAST>($5);
    $$ = ast;
  }
  | IF '(' Exp ')' MatchStmt ELSE UnMatchStmt {
    auto ast = new UnMatchStmtAST_2();
    ast->exp = unique_ptr<BaseAST>($3);
    ast->match_stmt = unique_ptr<BaseAST>($5);
    ast->unmatch_stmt = unique_ptr<BaseAST>($7);
    $$ = ast;
  }

Exp 
  : UnaryExp {
    auto ast = new ExpAST_1();
    ast->unary_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | AddExp {
    auto ast = new ExpAST_2();
    ast->add_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | LOrExp {
    auto ast = new ExpAST_3();
    ast->lor_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

PrimaryExp 
  : '(' Exp ')' {
    auto ast = new PrimaryExpAST_1();
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | Number {
    auto ast = new PrimaryExpAST_2();
    ast->number = ($1);
    $$ = ast;
  }
  | LVal {
    auto ast = new PrimaryExpAST_3();
    ast->l_val = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

Number 
  : INT_CONST {
    $$ = ($1);
  }

UnaryExp
  : PrimaryExp {
    auto ast = new UnaryExpAST_1();
    ast->primary_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | UnaryOp UnaryExp {
    auto ast = new UnaryExpAST_2();
    ast->unary_op = ($1)[0];
    ast->unary_exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  }
  | IDENT '(' FuncRParams ')' {
    auto ast = new UnaryExpAST_3();
    ast->ident = *unique_ptr<string>($1);
    ast->func_r_params = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | IDENT '(' ')' {
    auto ast = new UnaryExpAST_3();
    ast->ident = *unique_ptr<string>($1);
    $$ = ast;
  }

FuncRParams
  : Exp Exp_list {
    auto ast = new FuncRParamsAST();
    ($2)->insert(($2)->begin(),unique_ptr<BaseAST>($1));
    ast->exps = ($2);
    $$ = ast;
  }

Exp_list
  : ',' Exp Exp_list {
    ($3)->insert(($3)->begin(),unique_ptr<BaseAST>($2));
    $$ = ($3);
  }
  | {
    auto exps = new vector<unique_ptr<BaseAST>>();
    $$ = exps;
  }

UnaryOp
  : '+' {
    std::string u_op="+";
    $$ = &u_op;
  }
  | '-' {
    std::string u_op="-";
    $$ = &u_op;
  }
  | '!' {
    std::string u_op="!";
    $$ = &u_op;
  }

MulExp
  : UnaryExp{
    auto ast = new MulExpAST_1();
    ast->unary_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | MulExp '*' UnaryExp{
    auto ast = new MulExpAST_2();
    ast->mul_exp = unique_ptr<BaseAST>($1);
    ast->binary_op = "*";
    ast->unary_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | MulExp '/' UnaryExp{
    auto ast = new MulExpAST_2();
    ast->mul_exp = unique_ptr<BaseAST>($1);
    ast->binary_op = "/";
    ast->unary_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | MulExp '%' UnaryExp{
    auto ast = new MulExpAST_2();
    ast->mul_exp = unique_ptr<BaseAST>($1);
    ast->binary_op = "%";
    ast->unary_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

AddExp
  : MulExp{
    auto ast = new AddExpAST_1();
    ast->mul_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | AddExp '+' MulExp{
    auto ast = new AddExpAST_2();
    ast->add_exp = unique_ptr<BaseAST>($1);
    ast->binary_op = "+";
    ast->mul_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | AddExp '-' MulExp{
    auto ast = new AddExpAST_2();
    ast->add_exp = unique_ptr<BaseAST>($1);
    ast->binary_op = "-";
    ast->mul_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

RelExp
  : AddExp{
    auto ast = new RelExpAST_1();
    ast->add_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | RelExp '<' AddExp{
    auto ast = new RelExpAST_2();
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->cmp_op = "<";
    ast->add_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | RelExp '>' AddExp{
    auto ast = new RelExpAST_2();
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->cmp_op = ">";
    ast->add_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | RelExp LE AddExp{
    auto ast = new RelExpAST_2();
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->cmp_op = "<=";
    ast->add_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | RelExp GE AddExp{
    auto ast = new RelExpAST_2();
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->cmp_op = ">=";
    ast->add_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

EqExp
  : RelExp{
    auto ast = new EqExpAST_1();
    ast->rel_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | EqExp EQ RelExp{
    auto ast = new EqExpAST_2();
    ast->eq_exp = unique_ptr<BaseAST>($1);
    ast->eq_op = "==";
    ast->rel_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | EqExp NE RelExp{
    auto ast = new EqExpAST_2();
    ast->eq_exp = unique_ptr<BaseAST>($1);
    ast->eq_op = "!=";
    ast->rel_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

LAndExp
  : EqExp{
    auto ast = new LAndExpAST_1();
    ast->eq_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | LAndExp LAND EqExp{
    auto ast = new LAndExpAST_2();
    ast->land_exp = unique_ptr<BaseAST>($1);
    ast->eq_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

LOrExp
  : LAndExp{
    auto ast = new LOrExpAST_1();
    ast->land_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | LOrExp LOR LAndExp{
    auto ast = new LOrExpAST_2();
    ast->lor_exp = unique_ptr<BaseAST>($1);
    ast->land_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

Decl
  : ConstDecl {
    auto ast = new DeclAST_1();
    ast->const_decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }
  | VarDecl {
    auto ast = new DeclAST_2();
    ast->var_decl = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

LVal
  : IDENT {
    auto ast = new LValAST_1();
    ast->ident=*unique_ptr<string>($1);
    $$ = ast;
  }

ConstDecl
  : CONST INT ConstDef ConstDef_list ';' {
    auto ast = new ConstDeclAST();
    ($4)->insert(($4)->begin(),unique_ptr<BaseAST>($3));
    ast->const_defs=($4);
    $$ = ast;
  }

ConstDef_list
  : ',' ConstDef ConstDef_list {
    ($3)->insert(($3)->begin(),unique_ptr<BaseAST>($2));
    $$ = ($3);
  }
  | {
    auto const_defs = new vector<unique_ptr<BaseAST>>();
    $$ = const_defs;
  }

ConstDef
  : IDENT '=' ConstInitVal {
    auto ast = new ConstDefAST_1();
    ast->ident=*unique_ptr<string>($1);
    ast->const_init_val=unique_ptr<BaseAST>($3);
    $$ = ast;
  }
  | IDENT '[' ConstExp ']' '=' ConstInitVal {
    
  }

ConstInitVal
  : ConstExp {
    auto ast = new ConstInitValAST_1();
    ast->const_exp=unique_ptr<BaseAST>($1);
    $$ = ast;
  }

ConstExp
  : Exp {
    auto ast = new ConstExpAST();
    ast->exp=unique_ptr<BaseAST>($1);
    $$ = ast;
  }

VarDecl
  : INT VarDef VarDef_list ';' {
    auto ast = new VarDeclAST();
    ($3)->insert(($3)->begin(),unique_ptr<BaseAST>($2));
    ast->var_defs=($3);
    $$ = ast;
  }

VarDef_list
  : ',' VarDef VarDef_list {
    ($3)->insert(($3)->begin(),unique_ptr<BaseAST>($2));
    $$ = ($3);
  }
  | {
    auto var_defs = new vector<unique_ptr<BaseAST>>();
    $$ = var_defs;
  }

VarDef
  : IDENT {
    auto ast = new VarDefAST_1();
    ast->ident = *unique_ptr<string>($1);
    $$ = ast;
  }
  | IDENT '=' InitVal {
    auto ast = new VarDefAST_2();
    ast->ident = *unique_ptr<string>($1);
    ast->init_val = unique_ptr<BaseAST>($3);
    $$ = ast;
  }

InitVal
  : Exp {
    auto ast = new InitValAST_1();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

%%

void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
