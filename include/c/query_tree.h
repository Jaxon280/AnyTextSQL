#pragma once

#include "stdbool.h"
#include "stdlib.h"
#include "string.h"

typedef enum node_type {
    CLAUSE_NODE,
    ST_NODE,
    EXPR_NODE,
    FUNC_NODE,
    COND_NODE,
    PRED_NODE,
    SPRED_NODE,
    TPRED_NODE,
    NPRED_NODE,
    INT_NODE,
    BOOL_NODE,
    DOUBLE_NODE,
    STRING_NODE,
    IDENT_NODE
} NodeType;

typedef enum _clause_type {
    PROJ_NODE,
    FROM_NODE,
    SEL_NODE,
    AGG_NODE,
    SORT_NODE,
    LIM_NODE,
    NULL_NODE
} ClauseType;

typedef enum expr_type {
    ADD_NODE,
    SUB_NODE,
    MUL_NODE,
    DIV_NODE,
    MOD_NODE,
    AS_NODE  // other type?
} ExprType;

typedef enum func_type {
    AVG_NODE,
    SUM_NODE,
    COUNT_NODE,
    MIN_NODE,
    MAX_NODE,
    DISTINCT_NODE
} FuncType;

typedef enum cond_type {
    AND_NODE,
    OR_NODE,
} CondType;

typedef enum pred_type {
    EQ_NODE,
    NEQ_NODE,
    LESSEQ_NODE,
    LESS_NODE,
    GREATER_NODE,
    GREATEQ_NODE,
    REGEXP_NODE,
    LIKE_NODE
} PredType;

typedef struct ident {
    char *name;
    struct ident *child;
    int size;
    bool isWildcard;
} Ident;

typedef struct string {
    char *name;
    int size;
} String;

typedef struct node {
    NodeType type;

    ClauseType cltype;
    ExprType etype;
    FuncType ftype;
    CondType ctype;
    PredType ptype;
    int ivalue;
    double dvalue;
    String svalue;
    Ident ident;

    struct node *child;
    struct node *brother;
} Node;

Node *buildClauseNode(ClauseType cltype, Node *child, Node *brother);

Node *buildStNode(Node *child, Node *brother);
Node *buildExprNode(ExprType etype, Node *child, Node *brother);
Node *buildFuncNode(FuncType ftype, Node *child, Node *brother);
Node *buildCondNode(CondType ctype, Node *child, Node *brother);
Node *buildPredNode(PredType ptype, Node *child, Node *brother);
Node *buildTPredNode(PredType ptype, Node *child, Node *brother);
Node *buildNullPredNode(PredType ptype, Node *child, Node *brother);

Node *buildIntNode(int ivalue);
Node *buildDoubleNode(double dvalue);
Node *buildStringNode(char *svalue);
Node *buildIdentNode(char *identName, bool isWildCard);

Node *evalConstExpr(ExprType etype, Node *lnode, Node *rnode);
Node *evalConstCond(CondType ctype, Node *lnode, Node *rnode);
Node *evalConstPred(PredType ptype, Node *lnode, Node *rnode);
