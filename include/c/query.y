%{
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "y.tab.h"
extern int yylex();
extern int yyerror();
%}
%code requires {
#include "tree.h"
}
%union
{
    Node *node;
	double dvalue;
    int ivalue;
	char *str;
};
%token <dvalue> DOUBLE
%token <ivalue> INT
%token <str> IDENTIFIER STRING '=' '*' ',' ';' '(' ')' '.' '+' '-' '/' '%'
%token SELECT SUM COUNT AVG MIN MAX DISTINCT AS FROM WHERE AND OR IS NOT NIL BETWEEN REGEXP LIKE EQ NOTEQ LESS GREAT LESSEQ GREATEQ GROUP BY ORDER LIMIT
/* HAVING TRUE FALSE */
%type <node> expression cexpression select_expression statement statements identifier identifiers number conds cond ccond string_pred untyped_pred pred null_pred cpred integer string line from_clauses selection_clauses agg_clauses sort_clauses limit_clauses

%left AND OR
%left EQ NOTEQ LESS LESSEQ GREAT GREATEQ IS LIKE REGEXP
%left '+''-'
%left '*''/'

%%

/* line: SELECT statements FROM identifiers WHERE conds GROUP BY identifiers ORDER BY identifiers LIMIT NUMBER */

line: SELECT statements from_clauses {$$ = buildClauseNode(PROJ_NODE, $3, $2);};

from_clauses: FROM identifiers selection_clauses {$$ = buildClauseNode(FROM_NODE, $3, $2);};

selection_clauses: agg_clauses {$$ = $1;}
    | WHERE conds agg_clauses {$$ = buildClauseNode(SEL_NODE, $3, $2);}
    ;

agg_clauses: sort_clauses {$$ = $1;}
    | GROUP BY identifiers sort_clauses {$$ = buildClauseNode(AGG_NODE, $4, $3);}
    ;

sort_clauses: limit_clauses {$$ = $1;}
    | ORDER BY identifiers limit_clauses {$$ = buildClauseNode(SORT_NODE, $4, $3);}
    ;

limit_clauses: ';' {$$ = buildClauseNode(NULL_NODE, NULL, NULL);}
    | LIMIT integer ';' {$$ = buildClauseNode(LIM_NODE, $2, NULL);}
    ;

pred: expression EQ cexpression {$$ = buildPredNode(EQ_NODE, $1, $3);}
    | cexpression EQ expression {$$ = buildPredNode(EQ_NODE, $1, $3);}
    | expression NOTEQ cexpression {$$ = buildPredNode(NEQ_NODE, $1, $3);}
    | cexpression NOTEQ expression {$$ = buildPredNode(NEQ_NODE, $1, $3);}
    | expression LESSEQ expression {$$ = buildPredNode(LESSEQ_NODE, $1, $3);}
    | expression LESSEQ cexpression {$$ = buildPredNode(LESSEQ_NODE, $1, $3);}
    | cexpression LESSEQ expression {$$ = buildPredNode(LESSEQ_NODE, $1, $3);}
    | expression LESS expression {$$ = buildPredNode(LESS_NODE, $1, $3);}
    | expression LESS cexpression {$$ = buildPredNode(LESS_NODE, $1, $3);}
    | cexpression LESS expression {$$ = buildPredNode(LESS_NODE, $1, $3);}
    | expression GREATEQ expression {$$ = buildPredNode(GREATEQ_NODE, $1, $3);}
    | expression GREATEQ cexpression {$$ = buildPredNode(GREATEQ_NODE, $1, $3);}
    | cexpression GREATEQ expression {$$ = buildPredNode(GREATEQ_NODE, $1, $3);}
    | expression GREAT expression {$$ = buildPredNode(GREATER_NODE, $1, $3);}
    | expression GREAT cexpression {$$ = buildPredNode(GREATER_NODE, $1, $3);}
    | cexpression GREAT expression {$$ = buildPredNode(GREATER_NODE, $1, $3);}
    | expression BETWEEN cexpression AND cexpression {$$ = buildCondNode(AND_NODE, buildPredNode(LESSEQ_NODE, $3, $1), buildPredNode(GREATEQ_NODE, $5, $1));}
    ;

string_pred: identifier LIKE string {$$ = buildPredNode(LIKE_NODE, $1, $3);}
    | identifier REGEXP string {$$ = buildPredNode(REGEXP_NODE, $1, $3);}
    /* | identifier IN '[' strings ']' {$$ = buildPredNode();} */
    /* | identifier NOT IN '[' strings ']' {$$ = buildPredNode();} */
    ;

null_pred: identifier IS NIL {$$ = buildNullPredNode(EQ_NODE, $1, NULL);}
    | identifier IS NOT NIL {$$ = buildNullPredNode(NEQ_NODE, $1, NULL);}
    ;

untyped_pred: expression EQ expression {$$ = buildTPredNode(EQ_NODE, $1, $3);}
    | expression NOTEQ expression {$$ = buildTPredNode(NEQ_NODE, $1, $3);}
    ;

cpred: cexpression EQ cexpression {$$ = evalConstPred(EQ_NODE, $1, $3);}
    | cexpression NOTEQ cexpression {$$ = evalConstPred(NEQ_NODE, $1, $3);}
    | cexpression LESSEQ cexpression {$$ = evalConstPred(LESSEQ_NODE, $1, $3);}
    | cexpression LESS cexpression {$$ = evalConstPred(LESS_NODE, $1, $3);}
    | cexpression GREATEQ cexpression {$$ = evalConstPred(GREATEQ_NODE, $1, $3);}
    | cexpression GREAT cexpression {$$ = evalConstPred(GREATER_NODE, $1, $3);}
    | cexpression BETWEEN cexpression AND cexpression {$$ = evalConstCond(AND_NODE, evalConstPred(LESSEQ_NODE, $3, $1), evalConstPred(GREATEQ_NODE, $5, $1));}
    ;

ccond: '(' ccond ')' {$$ = $2;}
    | ccond AND ccond {$$ = evalConstCond(AND_NODE, $1, $3);}
    | ccond OR ccond {$$ = evalConstCond(OR_NODE, $1, $3);}
    | cpred {$$ = $1;}
    ;

cond: '(' cond ')' {$$ = $2;}
    | cond AND cond {$$ = buildCondNode(AND_NODE, $1, $3);}
    | cond OR cond {$$ = buildCondNode(OR_NODE, $1, $3);}
    | cond AND ccond {$$ = buildCondNode(AND_NODE, $1, $3);}
    | cond OR ccond {$$ = buildCondNode(OR_NODE, $1, $3);}
    | ccond AND cond {$$ = buildCondNode(AND_NODE, $1, $3);}
    | ccond OR cond {$$ = buildCondNode(OR_NODE, $1, $3);}
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
    | SUM '(' identifier ')' {$$ = buildFuncNode(SUM_NODE, $3, NULL);}
    | COUNT '(' identifier ')' {$$ = buildFuncNode(COUNT_NODE, $3, NULL);}
    | AVG '(' identifier ')' {$$ = buildFuncNode(AVG_NODE, $3, NULL);}
    | MIN '(' identifier ')' {$$ = buildFuncNode(MIN_NODE, $3, NULL);}
    | MAX '(' identifier ')' {$$ = buildFuncNode(MAX_NODE, $3, NULL);}
    | DISTINCT '(' identifier ')' {$$ = buildFuncNode(DISTINCT_NODE, $3, NULL);}
    ;

statement: select_expression AS identifier {$$ = buildExprNode(AS_NODE, $1, $3);}
    | select_expression {$$ = $1;}
    ;

statements: statement {$$ = buildStNode($1, NULL);}
    | statements ',' statement {$$ = buildStNode($1, $3);}
    ;

identifiers: identifier {$$ = buildStNode($1, NULL);}
    | identifiers ',' identifier {$$ = buildStNode($1, $3);}
    ;

identifier: '*' {$$ = buildIdentNode(NULL, true);}
    | IDENTIFIER {$$ = buildIdentNode($1, false);}
    ;

number: INT {$$ = buildIntNode($1);}
    | DOUBLE {$$ = buildDoubleNode($1);}
    ;

integer: INT {$$ = buildIntNode($1);}
    ;

string: STRING {$$ = buildStringNode($1);}
    ;

%%
int yyerror(char *s){
  printf("ERROR: %s\n\n",s);
  /*printf("\tERROR IN LINE %4d\n", yylval+1);
  */
	return *s;
}
int main() {
    while (true) {
        printf("\n> ");
        if (yyparse()) {
            fprintf(stderr, "Error\n");
            return 1;
        }
    }
    return 0;
}
