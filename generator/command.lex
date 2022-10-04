%{
#include "common.hpp"
#include "parser/cmd/commandParser.hpp"
#include "parser/cmd/commandNode.hpp"

#undef yywrap
#define yywrap() 1

using ctoken = command_yytokentype;

#define yyterminate()   return ctoken::CEOF_TK
%}

%option noyywrap batch nounput
%option never-interactive
%option nounistd
%option prefix="command_yy"
%%

".scan"     return ctoken::CSCAN_TK;
"-t"        return ctoken::CTABLE_TK;
"-e"        return ctoken::CEXP_TK;
"-k"        return ctoken::CKEYS_TK;
"["         return *yytext;
"]"         return *yytext;
","         return *yytext;

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.svalue = strdup(yytext);
    return ctoken::CIDENTIFIER_TK;
}

[a-zA-Z_][a-zA-Z0-9_]*(\.[a-zA-Z][a-zA-Z0-9]*)* {
    yylval.svalue = strdup(yytext);
    return ctoken::CFILENAME_TK;
}

'(\\.|[^'\\])*' {
    yylval.svalue = strdup(yytext);
    return ctoken::CPATTERN_TK;
}


[ \t\r]+ {}               /* ignore whitespace */
%%
