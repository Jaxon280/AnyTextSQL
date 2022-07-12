%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "query.h"
extern int yylex();
extern int yyerror();
%}
%code requires {
#include "query_tree.h"
}
%code {
Node *query_root;
}
%union
{
    Node *node;
	double dvalue;
    int ivalue;
	char *str;
};
%token <dvalue> DOUBLE_TK
%token <ivalue> INT_TK
%token <str> IDENTIFIER_TK STRING_TK '=' '*' ',' ';' '(' ')' '.' '+' '-' '/' '%'
%token SELECT_TK SUM_TK COUNT_TK AVG_TK MIN_TK MAX_TK DISTINCT_TK AS_TK FROM_TK WHERE_TK AND_TK OR_TK IS_TK NOT_TK NIL_TK BETWEEN_TK REGEXP_TK LIKE_TK EQ_TK NOTEQ_TK LESS_TK GREAT_TK LESSEQ_TK GREATEQ_TK GROUP_TK BY_TK ORDER_TK LIMIT_TK
/* HAVING TRUE FALSE */
%type <node> expression cexpression select_expression statement statements identifier identifiers number conds cond ccond string_pred untyped_pred pred null_pred cpred integer string line from_clauses selection_clauses agg_clauses sort_clauses limit_clauses

%left AND_TK OR_TK
%left EQ_TK NOTEQ_TK LESS_TK LESSEQ_TK GREAT_TK GREATEQ_TK IS_TK LIKE_TK REGEXP_TK
%left '+''-'
%left '*''/'

%%

/* line: SELECT statements FROM identifiers WHERE conds GROUP BY identifiers ORDER BY identifiers LIMIT NUMBER */

line: SELECT_TK statements from_clauses {query_root = buildClauseNode(PROJ_NODE, $3, $2);};

from_clauses: FROM_TK identifiers selection_clauses {$$ = buildClauseNode(FROM_NODE, $3, $2);};

selection_clauses: agg_clauses {$$ = $1;}
    | WHERE_TK conds agg_clauses {$$ = buildClauseNode(SEL_NODE, $3, $2);}
    ;

agg_clauses: sort_clauses {$$ = $1;}
    | GROUP_TK BY_TK identifiers sort_clauses {$$ = buildClauseNode(AGG_NODE, $4, $3);}
    ;

sort_clauses: limit_clauses {$$ = $1;}
    | ORDER_TK BY_TK identifiers limit_clauses {$$ = buildClauseNode(SORT_NODE, $4, $3);}
    ;

limit_clauses: ';' {$$ = buildClauseNode(NULL_NODE, NULL, NULL);}
    | LIMIT_TK integer ';' {$$ = buildClauseNode(LIM_NODE, $2, NULL);}
    ;

pred: expression EQ_TK cexpression {$$ = buildPredNode(EQ_NODE, $1, $3);}
    | cexpression EQ_TK expression {$$ = buildPredNode(EQ_NODE, $1, $3);}
    | expression NOTEQ_TK cexpression {$$ = buildPredNode(NEQ_NODE, $1, $3);}
    | cexpression NOTEQ_TK expression {$$ = buildPredNode(NEQ_NODE, $1, $3);}
    | expression LESSEQ_TK expression {$$ = buildPredNode(LESSEQ_NODE, $1, $3);}
    | expression LESSEQ_TK cexpression {$$ = buildPredNode(LESSEQ_NODE, $1, $3);}
    | cexpression LESSEQ_TK expression {$$ = buildPredNode(LESSEQ_NODE, $1, $3);}
    | expression LESS_TK expression {$$ = buildPredNode(LESS_NODE, $1, $3);}
    | expression LESS_TK cexpression {$$ = buildPredNode(LESS_NODE, $1, $3);}
    | cexpression LESS_TK expression {$$ = buildPredNode(LESS_NODE, $1, $3);}
    | expression GREATEQ_TK expression {$$ = buildPredNode(GREATEQ_NODE, $1, $3);}
    | expression GREATEQ_TK cexpression {$$ = buildPredNode(GREATEQ_NODE, $1, $3);}
    | cexpression GREATEQ_TK expression {$$ = buildPredNode(GREATEQ_NODE, $1, $3);}
    | expression GREAT_TK expression {$$ = buildPredNode(GREATER_NODE, $1, $3);}
    | expression GREAT_TK cexpression {$$ = buildPredNode(GREATER_NODE, $1, $3);}
    | cexpression GREAT_TK expression {$$ = buildPredNode(GREATER_NODE, $1, $3);}
    | expression BETWEEN_TK cexpression AND_TK cexpression {$$ = buildCondNode(AND_NODE, buildPredNode(LESSEQ_NODE, $3, $1), buildPredNode(GREATEQ_NODE, $5, $1));}
    ;

string_pred: identifier LIKE_TK string {$$ = buildPredNode(LIKE_NODE, $1, $3);}
    | identifier REGEXP_TK string {$$ = buildPredNode(REGEXP_NODE, $1, $3);}
    /* | identifier IN '[' strings ']' {$$ = buildPredNode();} */
    /* | identifier NOT IN '[' strings ']' {$$ = buildPredNode();} */
    ;

null_pred: identifier IS_TK NIL_TK {$$ = buildNullPredNode(EQ_NODE, $1, NULL);}
    | identifier IS_TK NOT_TK NIL_TK {$$ = buildNullPredNode(NEQ_NODE, $1, NULL);}
    ;

untyped_pred: expression EQ_TK expression {$$ = buildTPredNode(EQ_NODE, $1, $3);}
    | expression NOTEQ_TK expression {$$ = buildTPredNode(NEQ_NODE, $1, $3);}
    ;

cpred: cexpression EQ_TK cexpression {$$ = evalConstPred(EQ_NODE, $1, $3);}
    | cexpression NOTEQ_TK cexpression {$$ = evalConstPred(NEQ_NODE, $1, $3);}
    | cexpression LESSEQ_TK cexpression {$$ = evalConstPred(LESSEQ_NODE, $1, $3);}
    | cexpression LESS_TK cexpression {$$ = evalConstPred(LESS_NODE, $1, $3);}
    | cexpression GREATEQ_TK cexpression {$$ = evalConstPred(GREATEQ_NODE, $1, $3);}
    | cexpression GREAT_TK cexpression {$$ = evalConstPred(GREATER_NODE, $1, $3);}
    | cexpression BETWEEN_TK cexpression AND_TK cexpression {$$ = evalConstCond(AND_NODE, evalConstPred(LESSEQ_NODE, $3, $1), evalConstPred(GREATEQ_NODE, $5, $1));}
    ;

ccond: '(' ccond ')' {$$ = $2;}
    | ccond AND_TK ccond {$$ = evalConstCond(AND_NODE, $1, $3);}
    | ccond OR_TK ccond {$$ = evalConstCond(OR_NODE, $1, $3);}
    | cpred {$$ = $1;}
    ;

cond: '(' cond ')' {$$ = $2;}
    | cond AND_TK cond {$$ = buildCondNode(AND_NODE, $1, $3);}
    | cond OR_TK cond {$$ = buildCondNode(OR_NODE, $1, $3);}
    | cond AND_TK ccond {$$ = buildCondNode(AND_NODE, $1, $3);}
    | cond OR_TK ccond {$$ = buildCondNode(OR_NODE, $1, $3);}
    | ccond AND_TK cond {$$ = buildCondNode(AND_NODE, $1, $3);}
    | ccond OR_TK cond {$$ = buildCondNode(OR_NODE, $1, $3);}
    | pred {$$ = $1;}
    | string_pred {$$ = $1;}
    | untyped_pred {$$ = $1;}
    | null_pred {$$ = $1;}
    ;

conds: cond {$$ = $1;}
    | ccond {$$ = $1;}
    ;


cexpression: '(' cexpression ')' {$$ = $2;}
    | cexpression '+' cexpression {$$ = evalConstExpr(ADD_NODE, $1, $3);}
    | cexpression '-' cexpression {$$ = evalConstExpr(SUB_NODE, $1, $3);}
    | cexpression '*' cexpression {$$ = evalConstExpr(MUL_NODE, $1, $3);}
    | cexpression '/' cexpression {$$ = evalConstExpr(DIV_NODE, $1, $3);}
    | number {$$ = $1;}
    ;

expression: '(' expression ')' {$$ = $2;}
    | expression '+' expression {$$ = buildExprNode(ADD_NODE, $1, $3);}
    | expression '-' expression {$$ = buildExprNode(SUB_NODE, $1, $3);}
    | expression '*' expression {$$ = buildExprNode(MUL_NODE, $1, $3);}
    | expression '/' expression {$$ = buildExprNode(DIV_NODE, $1, $3);}
    | expression '+' cexpression {$$ = buildExprNode(ADD_NODE, $1, $3);}
    | expression '-' cexpression {$$ = buildExprNode(SUB_NODE, $1, $3);}
    | expression '*' cexpression {$$ = buildExprNode(MUL_NODE, $1, $3);}
    | expression '/' cexpression {$$ = buildExprNode(DIV_NODE, $1, $3);}
    | cexpression '+' expression {$$ = buildExprNode(ADD_NODE, $1, $3);}
    | cexpression '-' expression {$$ = buildExprNode(SUB_NODE, $1, $3);}
    | cexpression '*' expression {$$ = buildExprNode(MUL_NODE, $1, $3);}
    | cexpression '/' expression {$$ = buildExprNode(DIV_NODE, $1, $3);}
    | identifier {$$ = $1;}
    ;

select_expression: expression
    | SUM_TK '(' identifier ')' {$$ = buildFuncNode(SUM_NODE, $3, NULL);}
    | COUNT_TK '(' identifier ')' {$$ = buildFuncNode(COUNT_NODE, $3, NULL);}
    | AVG_TK '(' identifier ')' {$$ = buildFuncNode(AVG_NODE, $3, NULL);}
    | MIN_TK '(' identifier ')' {$$ = buildFuncNode(MIN_NODE, $3, NULL);}
    | MAX_TK '(' identifier ')' {$$ = buildFuncNode(MAX_NODE, $3, NULL);}
    | DISTINCT_TK '(' identifier ')' {$$ = buildFuncNode(DISTINCT_NODE, $3, NULL);}
    ;

statement: select_expression AS_TK identifier {$$ = buildExprNode(AS_NODE, $1, $3);}
    | select_expression {$$ = $1;}
    ;

statements: statement {$$ = buildStNode($1, NULL);}
    | statements ',' statement {$$ = buildStNode($1, $3);}
    ;

identifiers: identifier {$$ = buildStNode($1, NULL);}
    | identifiers ',' identifier {$$ = buildStNode($1, $3);}
    ;

identifier: '*' {$$ = buildIdentNode(NULL, true);}
    | IDENTIFIER_TK {$$ = buildIdentNode($1, false);}
    ;

number: INT_TK {$$ = buildIntNode($1);}
    | DOUBLE_TK {$$ = buildDoubleNode($1);}
    ;

integer: INT_TK {$$ = buildIntNode($1);}
    ;

string: STRING_TK {$$ = buildStringNode($1);}
    ;

%%
int yyerror(char *s){
  printf("ERROR: %s\n\n",s);
  /*printf("\tERROR IN LINE %4d\n", yylval+1);
  */
	return *s;
}
