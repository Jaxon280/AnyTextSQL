%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include "parser/regex/regexParser.hpp"
#include "parser/regex/regexNode.hpp"

#undef yywrap
#define yywrap() 1

using rtoken = regex_yytokentype;

#define yyterminate()   return rtoken::REOF_TK
%}

%option noyywrap batch nounput
%option never-interactive
%option nounistd
%option prefix="regex_yy"

%%

("INT"|"int")                 return rtoken::RINT_TK;
("DOUBLE"|"double")              return rtoken::RDOUBLE_TK;
("TEXT"|"text")                return rtoken::RTEXT_TK;
"*"                   return *yytext;
"."                   return *yytext;
"("                   return *yytext;
")"                   return *yytext;
"["                   return *yytext;
"]"                   return *yytext;
"+"                   return *yytext;
"-"                   return *yytext;
"|"                   return *yytext;
"%"                   return *yytext;
"?"                   return *yytext;
"^"                   return *yytext;
"@"                   return *yytext;
"\\w"                  return rtoken::RALPH_TK;
"\\W"                  return rtoken::RCAPT_TK;
"\\d"                  return rtoken::RDIGIT_TK;

"{"[1-9][0-9]*"}" {
    char buf[128];
    char rst[128];
    memset(buf, '\0', 128);
    sscanf(yytext, "%s", buf);
    int size = strlen(buf) - 2;
    memcpy(rst, &buf[1], size);
    rst[size] = '\0';

    int temp;
    sscanf(rst, "%d", &temp);
    yylval.ivalue = temp;
    return rtoken::RCNUM_TK;
}

([ !"#&',/0-9:;<=>A-Z_`a-z\{\}~]|\\[@$%\(\)*+-.?\[\]\^\|]) {
    int size = strlen(yytext);
    yylval.cvalue = yytext[size - 1];
    return rtoken::RCHAR_TK;
}

"?P<"[a-zA-Z][a-zA-Z0-9_]*">" {
    char buf1[128];
    char buf2[128];
    memset(buf1, '\0', 128);
    sscanf(yytext, "%s", buf1);
    int size = strlen(buf1) - 4;
    memcpy(buf2, &buf1[3], size);
    buf2[size] = '\0';

    yylval.svalue = strdup(buf2);
    return rtoken::RSUBMATCH_TK;
}
%%
