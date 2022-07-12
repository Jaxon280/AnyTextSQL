#include "query_tree.h"

Node *buildClauseNode(ClauseType cltype, Node *child, Node *brother) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->type = CLAUSE_NODE;
    n->cltype = cltype;
    n->child = child;
    if (child != NULL) {
        child->brother = brother;
    }
    n->brother = NULL;
    return n;
}

Node *buildStNode(Node *child, Node *brother) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->type = ST_NODE;
    n->child = child;
    child->brother = brother;
    n->brother = NULL;
    return n;
}

Node *buildExprNode(ExprType etype, Node *child, Node *brother) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->type = EXPR_NODE;
    n->etype = etype;
    n->child = child;
    child->brother = brother;
    n->brother = NULL;
    return n;
}

Node *buildFuncNode(FuncType ftype, Node *child, Node *brother) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->type = FUNC_NODE;
    n->ftype = ftype;
    n->child = child;
    child->brother = brother;
    n->brother = NULL;
    return n;
}

Node *buildCondNode(CondType ctype, Node *child, Node *brother) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->type = COND_NODE;
    n->ctype = ctype;
    n->child = child;
    child->brother = brother;
    n->brother = NULL;
    return n;
}

Node *buildPredNode(PredType ptype, Node *child, Node *brother) {
    Node *n = (Node *)malloc(sizeof(Node));
    if (ptype == LIKE_NODE || ptype == REGEXP_NODE) {
        n->type = SPRED_NODE;
    } else {
        n->type = PRED_NODE;
    }
    n->ptype = ptype;
    n->child = child;
    child->brother = brother;
    n->brother = NULL;
    return n;
}

Node *buildTPredNode(PredType ptype, Node *child, Node *brother) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->type = TPRED_NODE;
    n->ptype = ptype;
    n->child = child;
    child->brother = brother;
    n->brother = NULL;
    return n;
}

Node *buildNullPredNode(PredType ptype, Node *child, Node *brother) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->type = NPRED_NODE;
    n->ptype = ptype;  // EQ or NEQ
    n->child = child;
    child->brother = brother;
    n->brother = NULL;
    return n;
}

Node *buildIntNode(int ivalue) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->type = INT_NODE;
    n->ivalue = ivalue;
    n->child = NULL;
    n->brother = NULL;
    return n;
}

Node *buildDoubleNode(double dvalue) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->type = DOUBLE_NODE;
    n->dvalue = dvalue;
    n->child = NULL;
    n->brother = NULL;
    return n;
}

Node *buildStringNode(char *svalue) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->type = STRING_NODE;
    n->svalue.name = svalue;
    n->svalue.size = strlen(svalue);
    n->child = NULL;
    n->brother = NULL;
    return n;
}

Node *buildIdentNode(char *identName, bool isWildCard) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->type = IDENT_NODE;
    if (isWildCard) {
        n->ident.isWildcard = true;
    } else {
        n->ident.name = identName;
        n->ident.size = strlen(identName);
        n->ident.isWildcard = false;
    }
    n->child = NULL;
    n->brother = NULL;
    return n;
}

Node *evalConstPred(PredType ptype, Node *lnode, Node *rnode) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->type = BOOL_NODE;
    if (ptype == EQ_NODE) {
        if (lnode->type == DOUBLE_NODE && rnode->type == DOUBLE_NODE) {
            n->ivalue = lnode->dvalue == rnode->dvalue;
        } else if (lnode->type == DOUBLE_NODE && rnode->type == INT_NODE) {
            n->ivalue = lnode->dvalue == (double)rnode->ivalue;
        } else if (lnode->type == INT_NODE && rnode->type == DOUBLE_NODE) {
            n->ivalue = (double)lnode->ivalue == rnode->dvalue;
        } else {
            n->ivalue = lnode->ivalue == rnode->ivalue;
        }
    } else if (ptype == NEQ_NODE) {
        if (lnode->type == DOUBLE_NODE && rnode->type == DOUBLE_NODE) {
            n->ivalue = lnode->dvalue != rnode->dvalue;
        } else if (lnode->type == DOUBLE_NODE && rnode->type == INT_NODE) {
            n->ivalue = lnode->dvalue != (double)rnode->ivalue;
        } else if (lnode->type == INT_NODE && rnode->type == DOUBLE_NODE) {
            n->ivalue = (double)lnode->ivalue != rnode->dvalue;
        } else {
            n->ivalue = lnode->ivalue != rnode->ivalue;
        }
    } else if (ptype == LESS_NODE) {
        if (lnode->type == DOUBLE_NODE && rnode->type == DOUBLE_NODE) {
            n->ivalue = lnode->dvalue < rnode->dvalue;
        } else if (lnode->type == DOUBLE_NODE && rnode->type == INT_NODE) {
            n->ivalue = lnode->dvalue < (double)rnode->ivalue;
        } else if (lnode->type == INT_NODE && rnode->type == DOUBLE_NODE) {
            n->ivalue = (double)lnode->ivalue < rnode->dvalue;
        } else {
            n->ivalue = lnode->ivalue < rnode->ivalue;
        }
    } else if (ptype == LESSEQ_NODE) {
        if (lnode->type == DOUBLE_NODE && rnode->type == DOUBLE_NODE) {
            n->ivalue = lnode->dvalue <= rnode->dvalue;
        } else if (lnode->type == DOUBLE_NODE && rnode->type == INT_NODE) {
            n->ivalue = lnode->dvalue <= (double)rnode->ivalue;
        } else if (lnode->type == INT_NODE && rnode->type == DOUBLE_NODE) {
            n->ivalue = (double)lnode->ivalue <= rnode->dvalue;
        } else {
            n->ivalue = lnode->ivalue <= rnode->ivalue;
        }
    } else if (ptype == GREATER_NODE) {
        if (lnode->type == DOUBLE_NODE && rnode->type == DOUBLE_NODE) {
            n->ivalue = lnode->dvalue > rnode->dvalue;
        } else if (lnode->type == DOUBLE_NODE && rnode->type == INT_NODE) {
            n->ivalue = lnode->dvalue > (double)rnode->ivalue;
        } else if (lnode->type == INT_NODE && rnode->type == DOUBLE_NODE) {
            n->ivalue = (double)lnode->ivalue > rnode->dvalue;
        } else {
            n->ivalue = lnode->ivalue > rnode->ivalue;
        }
    } else if (ptype == GREATEQ_NODE) {
        if (lnode->type == DOUBLE_NODE && rnode->type == DOUBLE_NODE) {
            n->ivalue = lnode->dvalue >= rnode->dvalue;
        } else if (lnode->type == DOUBLE_NODE && rnode->type == INT_NODE) {
            n->ivalue = lnode->dvalue >= (double)rnode->ivalue;
        } else if (lnode->type == INT_NODE && rnode->type == DOUBLE_NODE) {
            n->ivalue = (double)lnode->ivalue >= rnode->dvalue;
        } else {
            n->ivalue = lnode->ivalue >= rnode->ivalue;
        }
    } else {
        return NULL;
    }

    free(lnode);
    free(rnode);
    n->child = NULL;
    n->brother = NULL;

    return n;
}

Node *evalConstCond(CondType ctype, Node *lnode, Node *rnode) {
    Node *n = (Node *)malloc(sizeof(Node));
    n->type = BOOL_NODE;
    if (ctype == AND_NODE) {
        if (lnode->ivalue && rnode->ivalue) {
            n->ivalue = 1;
        } else {
            n->ivalue = 0;
        }
    } else if (ctype == OR_NODE) {
        if (lnode->ivalue || rnode->ivalue) {
            n->ivalue = 1;
        } else {
            n->ivalue = 0;
        }
    }
    free(lnode);
    free(rnode);
    return n;
}

Node *evalConstExpr(ExprType etype, Node *lnode, Node *rnode) {
    Node *n = (Node *)malloc(sizeof(Node));

    if (etype == ADD_NODE) {
        if (lnode->type == DOUBLE_NODE && rnode->type == DOUBLE_NODE) {
            n->type = DOUBLE_NODE;
            n->dvalue = lnode->dvalue + rnode->dvalue;
        } else if (lnode->type == DOUBLE_NODE && rnode->type == INT_NODE) {
            n->type = DOUBLE_NODE;
            n->dvalue = lnode->dvalue + (double)rnode->ivalue;
        } else if (lnode->type == INT_NODE && rnode->type == DOUBLE_NODE) {
            n->type = DOUBLE_NODE;
            n->dvalue = (double)lnode->ivalue + rnode->dvalue;
        } else {
            n->type = INT_NODE;
            n->ivalue = lnode->ivalue + rnode->ivalue;
        }
    } else if (etype == SUB_NODE) {
        if (lnode->type == DOUBLE_NODE && rnode->type == DOUBLE_NODE) {
            n->type = DOUBLE_NODE;
            n->dvalue = lnode->dvalue - rnode->dvalue;
        } else if (lnode->type == DOUBLE_NODE && rnode->type == INT_NODE) {
            n->type = DOUBLE_NODE;
            n->dvalue = lnode->dvalue - (double)rnode->ivalue;
        } else if (lnode->type == INT_NODE && rnode->type == DOUBLE_NODE) {
            n->type = DOUBLE_NODE;
            n->dvalue = (double)lnode->ivalue - rnode->dvalue;
        } else {
            n->type = INT_NODE;
            n->ivalue = lnode->ivalue - rnode->ivalue;
        }
    } else if (etype == MUL_NODE) {
        if (lnode->type == DOUBLE_NODE && rnode->type == DOUBLE_NODE) {
            n->type = DOUBLE_NODE;
            n->dvalue = lnode->dvalue * rnode->dvalue;
        } else if (lnode->type == DOUBLE_NODE && rnode->type == INT_NODE) {
            n->type = DOUBLE_NODE;
            n->dvalue = lnode->dvalue * (double)rnode->ivalue;
        } else if (lnode->type == INT_NODE && rnode->type == DOUBLE_NODE) {
            n->type = DOUBLE_NODE;
            n->dvalue = (double)lnode->ivalue * rnode->dvalue;
        } else {
            n->type = INT_NODE;
            n->ivalue = lnode->ivalue * rnode->ivalue;
        }
    } else if (etype == DIV_NODE) {
        if (lnode->type == DOUBLE_NODE && rnode->type == DOUBLE_NODE) {
            n->type = DOUBLE_NODE;
            n->dvalue = lnode->dvalue / rnode->dvalue;
        } else if (lnode->type == DOUBLE_NODE && rnode->type == INT_NODE) {
            n->type = DOUBLE_NODE;
            n->dvalue = lnode->dvalue / (double)rnode->ivalue;
        } else if (lnode->type == INT_NODE && rnode->type == DOUBLE_NODE) {
            n->type = DOUBLE_NODE;
            n->dvalue = (double)lnode->ivalue / rnode->dvalue;
        } else {
            n->type = DOUBLE_NODE;
            n->dvalue = lnode->ivalue / rnode->ivalue;
        }
    } else {
        return NULL;
    }

    free(lnode);
    free(rnode);
    n->child = NULL;
    n->brother = NULL;

    return n;
}
