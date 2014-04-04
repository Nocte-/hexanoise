%{
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include <hexanoise/ast.hpp>
#include "parser.hpp"
#include "tokens.hpp"

void yyerror(hexa::noise::function** func, yyscan_t scanner, const char* s) 
{ 
    throw std::runtime_error(s);
}

using namespace hexa::noise;

%}

%code requires {

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

}

%define api.pure
%lex-param { yyscan_t scanner }
%parse-param { hexa::noise::function** func }
%parse-param { yyscan_t scanner }

%union {
    hexa::noise::function* func;
    std::vector<hexa::noise::function*>* param_list;
    std::string* string;
    int token;
}

%token <string> TIDENTIFIER TVALUE TSTRING
%token <token>  TCOLON TCOMMA TLPAREN TRPAREN TLACCOL TRACCOL TDOLLAR TAT

%type <func> function 
%type <param_list> param_list

%left TCOLON

%%

input : function { *func = $1; } 
         ;

function : TVALUE 
                { $$ = new function(); $$->type = function::const_v; $$->value = std::stod(*$1); delete $1; }
         | TSTRING
                { $$ = new function(); $$->type = function::const_s; $$->name = *$1; delete $1; }
         | TDOLLAR TIDENTIFIER
                { $$ = new function(*$2); $$->type = function::global; delete $2; }
         | TAT TIDENTIFIER
                { $$ = new function(*$2); $$->type = function::external; delete $2; }
         | TIDENTIFIER
                { $$ = new function(*$1); delete $1; }
         | TIDENTIFIER TLPAREN param_list TRPAREN
                { $$ = new function(*$1); $$->args = $3; delete $1; }
         | function TCOLON function
                { $3->input = $1; $$ = $3; }
         | TLACCOL function TRACCOL
                { $$ = new function(); 
                  $$->type = function::lambda; 
                  $$->args = new std::vector<function*>();
                  $$->args->push_back($2);
                }
         ;

param_list : function
                { $$ = new std::vector<function*>(); $$->push_back($1); }
           | param_list TCOMMA function
                { $$->push_back($3); }
           ;

%%

