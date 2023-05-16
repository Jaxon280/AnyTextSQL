%{
#include "common/common.hpp"
#include "parser/query/query-parser.hpp"
#include "parser/query/query-node.hpp"

#undef yywrap
#define yywrap() 1

using qtoken = query_yytokentype;

#define yyterminate()   return qtoken::EOF_TK
%}

%option noyywrap batch nounput
%option never-interactive
%option nounistd
%option prefix="query_yy"
%%

("select"|"SELECT")   return qtoken::SELECT_TK;
("sum"|"SUM")         return qtoken::SUM_TK;
("count"|"COUNT")     return qtoken::COUNT_TK;
("avg"|"AVG")         return qtoken::AVG_TK;
("min"|"MIN")         return qtoken::MIN_TK;
("max"|"MAX")         return qtoken::MAX_TK;
("distinct"|"DISTINCT")  return qtoken::DISTINCT_TK;
("as"|"AS")           return qtoken::AS_TK;
("from"|"FROM")       return qtoken::FROM_TK;
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
("where"|"WHERE")     return qtoken::WHERE_TK;
("and"|"AND")         return qtoken::AND_TK;
("or"|"OR")           return qtoken::OR_TK;
("not"|"NOT")         return qtoken::NOT_TK;
("is"|"IS")           return qtoken::IS_TK;
("null"|"NULL")       return qtoken::NIL_TK;
("between"|"BETWEEN") return qtoken::BETWEEN_TK;
("regexp"|"REGEXP")   return qtoken::REGEXP_TK;
("like"|"LIKE")       return qtoken::LIKE_TK;
"=="                  return qtoken::EQ_TK;
"!="                  return qtoken::NOTEQ_TK;
"<"                   return qtoken::LESS_TK;
">"                   return qtoken::GREAT_TK;
"<="                  return qtoken::LESSEQ_TK;
">="                  return qtoken::GREATEQ_TK;
("group"|"GROUP")     return qtoken::GROUP_TK;
("by"|"BY")           return qtoken::BY_TK;
("order"|"ORDER")     return qtoken::ORDER_TK;
("limit"|"LIMIT")     return qtoken::LIMIT_TK;

[a-zA-Z_][a-zA-Z0-9_]* {
    yylval.svalue = strdup(yytext);
    return qtoken::IDENTIFIER_TK;
}
(0|[1-9][0-9]*)(\.[0-9]+)([Ee][+-]?(0|[1-9][0-9]*))? {
    double temp;
    sscanf(yytext, "%lf", &temp);
    yylval.dvalue = temp;
    return qtoken::DOUBLE_TK;
}
(0|[1-9][0-9]*) {
    int temp;
    sscanf(yytext, "%d", &temp);
    yylval.ivalue = temp;
    return qtoken::INT_TK;
}

'(\\.|[^\n'\\])*' {
    yylval.svalue = strdup(yytext);
    return qtoken::STRING_TK;
}


[ \t\r]+ {}               /* ignore whitespace */
%%
