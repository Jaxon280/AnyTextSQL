%{
#include <stdio.h>
#include <string.h>
#include "y.tab.h"
#include "tree.h"
int lineno=0;
%}
%option noyywrap
%option noinput nounput
%%
("select"|"SELECT")   return SELECT;
("sum"|"SUM")         return SUM;
("count"|"COUNT")     return COUNT;
("avg"|"AVG")         return AVG;
("min"|"MIN")         return MIN;
("max"|"MAX")         return MAX;
("distinct"|"DISTINCT")  return DISTINCT;
("as"|"AS")           return AS;
("from"|"FROM")       return FROM;
"*"                   return *yytext;
","                   return *yytext;
";"                   return *yytext;
"."                   return *yytext;
"("                   return *yytext;
")"                   return *yytext;
"="                   return *yytext;
"+"                   return *yytext;
"-"                   return *yytext;
"/"                   return *yytext;
"%"                   return *yytext;
("where"|"WHERE")     return WHERE;
("and"|"AND")         return AND;
("or"|"OR")           return OR;
("not"|"NOT")         return NOT;
("is"|"IS")           return IS;
("null"|"NULL")       return NIL;
("between"|"BETWEEN") return BETWEEN;
("regexp"|"REGEXP")   return REGEXP;
("like"|"LIKE")       return LIKE;
"=="                  return EQ;
"!="                  return NOTEQ;
"<"                   return LESS;
">"                   return GREAT;
"<="                  return LESSEQ;
">="                  return GREATEQ;
("group"|"GROUP")     return GROUP;
("by"|"BY")           return BY;
("order"|"ORDER")     return ORDER;
("limit"|"LIMIT")     return LIMIT;

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.str = strdup(yytext);
    return IDENTIFIER;
}
[+-]?(0|[1-9][0-9]*)(\.[0-9]+)([Ee][+-]?(0|[1-9][0-9]*))? {
    double temp;
    sscanf(yytext, "%lf", &temp);
    yylval.dvalue = temp;
    return DOUBLE;
}
[+-]?(0|[1-9][0-9]*) {
    int temp;
    sscanf(yytext, "%d", &temp);
    yylval.ivalue = temp;
    return INT;
}

'(\\.|[^\n'\\])*' {
    yylval.str = strdup(yytext);
    return STRING;
}

\n			{lineno++; return 0;}

[ \t\r]+                /* ignore whitespace */
%%
