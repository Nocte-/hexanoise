
%{
#include <string>

#include <hexanoise/ast.hpp>
#include "parser.hpp"

#define SAVE_TOKEN yylval->string = new std::string(yytext, yyleng)
#define TOKEN(t) (yylval->token = t)

%}

%option reentrant never-interactive noyywrap nounput
%option bison-bridge

%x COMMENT

%%

"/*"                  { BEGIN(COMMENT); }
<COMMENT>"*/"         { BEGIN(INITIAL); }
<COMMENT>.            { }

[ \t\n]             ;
[a-z_]+             SAVE_TOKEN; return TIDENTIFIER;
-?[0-9]+\.?[0-9]*   SAVE_TOKEN; return TVALUE;
\"[a-zA-Z0-9/_.]*\" SAVE_TOKEN; return TSTRING;

":"                 return TOKEN(TCOLON);
","                 return TOKEN(TCOMMA);
"("                 return TOKEN(TLPAREN);
")"                 return TOKEN(TRPAREN);
"{"                 return TOKEN(TLACCOL);
"}"                 return TOKEN(TRACCOL);
"$"                 return TOKEN(TDOLLAR);
"@"                 return TOKEN(TAT);

.                   yyterminate();

