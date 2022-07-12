%{
#include <stdio.h>
#include <string.h>
#include "query.h"
#include "query_tree.h"
int lineno=0;
%}
%option noyywrap
%option noinput nounput
%%
("select"|"SELECT")   return SELECT_TK;
("sum"|"SUM")         return SUM_TK;
("count"|"COUNT")     return COUNT_TK;
("avg"|"AVG")         return AVG_TK;
("min"|"MIN")         return MIN_TK;
("max"|"MAX")         return MAX_TK;
("distinct"|"DISTINCT")  return DISTINCT_TK;
("as"|"AS")           return AS_TK;
("from"|"FROM")       return FROM_TK;
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
("where"|"WHERE")     return WHERE_TK;
("and"|"AND")         return AND_TK;
("or"|"OR")           return OR_TK;
("not"|"NOT")         return NOT_TK;
("is"|"IS")           return IS_TK;
("null"|"NULL")       return NIL_TK;
("between"|"BETWEEN") return BETWEEN_TK;
("regexp"|"REGEXP")   return REGEXP_TK;
("like"|"LIKE")       return LIKE_TK;
"=="                  return EQ_TK;
"!="                  return NOTEQ_TK;
"<"                   return LESS_TK;
">"                   return GREAT_TK;
"<="                  return LESSEQ_TK;
">="                  return GREATEQ_TK;
("group"|"GROUP")     return GROUP_TK;
("by"|"BY")           return BY_TK;
("order"|"ORDER")     return ORDER_TK;
("limit"|"LIMIT")     return LIMIT_TK;

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.str = strdup(yytext);
    return IDENTIFIER_TK;
}
[+-]?(0|[1-9][0-9]*)(\.[0-9]+)([Ee][+-]?(0|[1-9][0-9]*))? {
    double temp;
    sscanf(yytext, "%lf", &temp);
    yylval.dvalue = temp;
    return DOUBLE_TK;
}
[+-]?(0|[1-9][0-9]*) {
    int temp;
    sscanf(yytext, "%d", &temp);
    yylval.ivalue = temp;
    return INT_TK;
}

'(\\.|[^\n'\\])*' {
    yylval.str = strdup(yytext);
    return STRING_TK;
}

\n			{lineno++; return 0;}

[ \t\r]+                /* ignore whitespace */
%%
