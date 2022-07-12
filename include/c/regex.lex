%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "regex.h"
#include "regex_tree.h"
int lineno=0;
%}
%option noyywrap
%option noinput nounput
%%
"INT"                 return INT;
"DOUBLE"              return DOUBLE;
"TEXT"                return TEXT;
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
"\\w"                  return ALPH;
"\\W"                  return CAPT;
"\\d"                  return DIGIT;

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
    return CNUM;
}

([ !"#&',/0-9:;<=>A-Z_`a-z\{\}~]|\\[@$%\(\)*+-.?\[\]\^\|]) {
    int size = strlen(yytext);
    yylval.cvalue = yytext[size - 1];
    return CHAR;
}

"?P<"[a-zA-Z][a-zA-Z0-9_]*">" {
    char buf1[128];
    char buf2[128];
    memset(buf1, '\0', 128);
    sscanf(yytext, "%s", buf1);
    int size = strlen(buf1) - 4;
    memcpy(buf2, &buf1[3], size);
    buf2[size] = '\0';

    yylval.str = strdup(buf2);
    return SUBMATCH;
}

\n			{lineno++; return 0;}
%%
