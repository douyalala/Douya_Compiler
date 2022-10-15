// 这里写一些选项, 可以控制 Flex/Bison 的某些行为
%code requires {
  #include <iostream>
  #include <memory>
  #include <string>
  #include <vector>
  #include <AST.h>
}

%{

// 这里写一些全局的代码：头文件和一些全局声明/定义

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <AST.h>

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 对于 Bison, 这里可以定义终结符/非终结符的类型

// 定义 parser 函数和错误处理函数的附加参数
%parse-param { std::unique_ptr<BaseAST> &ast }

//yylval 的定义
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
  vector<unique_ptr<BaseAST>> *ast_list_val;
}


// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN LE GE EQ NE LAND LOR CONST IF ELSE
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> 
FuncDef FuncType 
Block BlockItem Stmt MatchStmt UnMatchStmt
Exp PrimaryExp UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp 
Decl BType LVal
ConstDecl ConstDef ConstInitVal ConstExp
VarDecl VarDef InitVal
%type <int_val> Number
%type <str_val> UnaryOp
%type <ast_list_val> BlockItem_list ConstDef_list VarDef_list

%%

// 这里写 Flex/Bison 的规则描述
// 对于 Bison, 这里写的是 parser 遇到某种语法规则后做的操作

CompUnit 
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  }

FuncDef 
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = "i32";
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  }

FuncType 
  : INT {}

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
    auto ast = new LValAST();
    ast->ident=*unique_ptr<string>($1);
    $$ = ast;
  }

ConstDecl
  : CONST BType ConstDef ConstDef_list ';' {
    auto ast = new ConstDeclAST();
    ast->b_type="int";
    ($4)->insert(($4)->begin(),unique_ptr<BaseAST>($3));
    ast->const_defs=($4);
    $$ = ast;
  }

BType
  : INT {}

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
    auto ast = new ConstDefAST();
    ast->ident=*unique_ptr<string>($1);
    ast->const_init_val=unique_ptr<BaseAST>($3);
    $$ = ast;
  }

ConstInitVal
  : ConstExp {
    auto ast = new ConstInitValAST();
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
  : BType VarDef VarDef_list ';' {
    auto ast = new VarDeclAST();
    ast->b_type="int";
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
    auto ast = new InitValAST();
    ast->exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  }

%%

// 这里写一些用户自定义的代码
// 比如你希望在生成的 C/C++ 文件里定义一个函数, 做一些辅助工作
// 你同时希望在之前的规则描述里调用你定义的函数
// 那么, 你可以把 C/C++ 的函数定义写在这里, 声明写在文件开头

void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
