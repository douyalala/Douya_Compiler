// 这里写一些选项, 可以控制 Flex/Bison 的某些行为
%code requires {
  #include <memory>
  #include <string>
  #include <AST.h>
}

%{

// 这里写一些全局的代码：头文件和一些全局声明/定义

#include <iostream>
#include <memory>
#include <string>
#include <AST.h>

// 声明 lexer 函数和错误处理函数
int yylex();
void yyerror(std::unique_ptr<BaseAST> &ast, const char *s);

using namespace std;

%}

// 对于 Bison, 这里可以定义终结符/非终结符的类型

// 定义 parser 函数和错误处理函数的附加参数
%parse-param { std::unique_ptr<BaseAST> &ast  }

//yylval 的定义
%union {
  std::string *str_val;
  int int_val;
  BaseAST *ast_val;
}


// lexer 返回的所有 token 种类的声明
// 注意 IDENT 和 INT_CONST 会返回 token 的值, 分别对应 str_val 和 int_val
%token INT RETURN LE GE EQ NE LAND LOR
%token <str_val> IDENT
%token <int_val> INT_CONST

// 非终结符的类型定义
%type <ast_val> FuncDef FuncType Block Stmt Exp PrimaryExp UnaryExp MulExp AddExp RelExp EqExp LAndExp LOrExp
%type <int_val> Number
%type <str_val> UnaryOp

%%

// 这里写 Flex/Bison 的规则描述
// 对于 Bison, 这里写的是 parser 遇到某种语法规则后做的操作

CompUnit 
  : FuncDef {
    auto comp_unit = make_unique<CompUnitAST>();
    comp_unit->func_def = unique_ptr<BaseAST>($1);
    ast = move(comp_unit);
  };

FuncDef 
  : FuncType IDENT '(' ')' Block {
    auto ast = new FuncDefAST();
    ast->func_type = unique_ptr<BaseAST>($1);
    ast->ident = *unique_ptr<string>($2);
    ast->block = unique_ptr<BaseAST>($5);
    $$ = ast;
  };

FuncType 
  : INT {
    auto ast = new FuncTypeAST();
    $$ = ast;
  };

Block 
  : '{' Stmt '}' {
    auto ast = new BlockAST();
    ast->stmt = unique_ptr<BaseAST>($2);
    $$ = ast;
  };

Stmt 
  : RETURN Exp ';' {
    auto ast = new StmtAST();
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  };

Exp 
  : UnaryExp {
    auto ast = new ExpAST_1();
    ast->unary_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  };
  | AddExp {
    auto ast = new ExpAST_2();
    ast->add_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  };
  | LOrExp {
    auto ast = new ExpAST_3();
    ast->lor_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  };

PrimaryExp 
  : '(' Exp ')' {
    auto ast = new PrimaryExpAST_1();
    ast->exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  };
  | Number {
    auto ast = new PrimaryExpAST_2();
    ast->number = ($1);
    $$ = ast;
  };

Number 
  : INT_CONST {
    $$ = ($1);
  };

UnaryExp
  : PrimaryExp {
    auto ast = new UnaryExpAST_1();
    ast->primary_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  };
  | UnaryOp UnaryExp {
    auto ast = new UnaryExpAST_2();
    ast->unary_op = ($1)[0];
    ast->unary_exp = unique_ptr<BaseAST>($2);
    $$ = ast;
  };

UnaryOp
  : '+' {
    std::string u_op="+";
    $$ = &u_op;
  };
  | '-' {
    std::string u_op="-";
    $$ = &u_op;
  };
  | '!' {
    std::string u_op="!";
    $$ = &u_op;
  };

MulExp
  : UnaryExp{
    auto ast = new MulExpAST_1();
    ast->unary_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  };
  | MulExp '*' UnaryExp{
    auto ast = new MulExpAST_2();
    ast->mul_exp = unique_ptr<BaseAST>($1);
    ast->binary_op = "*";
    ast->unary_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  };
  | MulExp '/' UnaryExp{
    auto ast = new MulExpAST_2();
    ast->mul_exp = unique_ptr<BaseAST>($1);
    ast->binary_op = "/";
    ast->unary_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  };
  | MulExp '%' UnaryExp{
    auto ast = new MulExpAST_2();
    ast->mul_exp = unique_ptr<BaseAST>($1);
    ast->binary_op = "%";
    ast->unary_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  };

AddExp
  : MulExp{
    auto ast = new AddExpAST_1();
    ast->mul_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  };
  | AddExp '+' MulExp{
    auto ast = new AddExpAST_2();
    ast->add_exp = unique_ptr<BaseAST>($1);
    ast->binary_op = "+";
    ast->mul_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  };
  | AddExp '-' MulExp{
    auto ast = new AddExpAST_2();
    ast->add_exp = unique_ptr<BaseAST>($1);
    ast->binary_op = "-";
    ast->mul_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  };

RelExp
  : AddExp{
    auto ast = new RelExpAST_1();
    ast->add_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  };
  | RelExp '<' AddExp{
    auto ast = new RelExpAST_2();
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->cmp_op = "<";
    ast->add_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  };
  | RelExp '>' AddExp{
    auto ast = new RelExpAST_2();
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->cmp_op = ">";
    ast->add_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  };
  | RelExp LE AddExp{
    auto ast = new RelExpAST_2();
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->cmp_op = "<=";
    ast->add_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  };
  | RelExp GE AddExp{
    auto ast = new RelExpAST_2();
    ast->rel_exp = unique_ptr<BaseAST>($1);
    ast->cmp_op = ">=";
    ast->add_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  };

EqExp
  : RelExp{
    auto ast = new EqExpAST_1();
    ast->rel_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  };
  | EqExp EQ RelExp{
    auto ast = new EqExpAST_2();
    ast->eq_exp = unique_ptr<BaseAST>($1);
    ast->eq_op = "==";
    ast->rel_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  };
  | EqExp NE RelExp{
    auto ast = new EqExpAST_2();
    ast->eq_exp = unique_ptr<BaseAST>($1);
    ast->eq_op = "!=";
    ast->rel_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  };

LAndExp
  : EqExp{
    auto ast = new LAndExpAST_1();
    ast->eq_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  };
  | LAndExp LAND EqExp{
    auto ast = new LAndExpAST_2();
    ast->land_exp = unique_ptr<BaseAST>($1);
    ast->eq_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  };

LOrExp
  : LAndExp{
    auto ast = new LOrExpAST_1();
    ast->land_exp = unique_ptr<BaseAST>($1);
    $$ = ast;
  };
  | LOrExp LOR LAndExp{
    auto ast = new LOrExpAST_2();
    ast->lor_exp = unique_ptr<BaseAST>($1);
    ast->land_exp = unique_ptr<BaseAST>($3);
    $$ = ast;
  };

%%

// 这里写一些用户自定义的代码
// 比如你希望在生成的 C/C++ 文件里定义一个函数, 做一些辅助工作
// 你同时希望在之前的规则描述里调用你定义的函数
// 那么, 你可以把 C/C++ 的函数定义写在这里, 声明写在文件开头

void yyerror(unique_ptr<BaseAST> &ast, const char *s) {
  cerr << "error: " << s << endl;
}
