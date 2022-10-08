#include "parser/query/queryNode.hpp"

StatementList *buildWildCard(bool isCount) {
    StatementList *stmtList = new StatementList;
    stmtList->next = NULL;
    Statement *stmt = new Statement;
    stmt->isWildCard = true;
    if (isCount) {
        stmt->httype = COUNT_HT;
    } else {
        stmt->httype = NONE_HT;
    }
    stmtList->stmt = stmt;
    return stmtList;
}

StatementList *buildStatements(StatementList *stmts, Statement *stmt) {
    StatementList *stmtList = new StatementList;
    stmtList->stmt = stmt;
    stmtList->next = stmts;
    return stmtList;
}

Statement *buildStatement(OpTree *left, const char *right) {
    Statement *stmt = new Statement;
    stmt->expr = left;
    if (right == NULL) {
        if (left->evalType == VAR) {
            stmt->name = left->varKey.c_str();
        }
    } else {
        stmt->name = right;
    }
    stmt->httype = left->httype;
    stmt->isWildCard = false;
    return stmt;
}

StringList *buildIdents(StringList *idents, const char *ident) {
    StringList *idList = new StringList;
    idList->str = ident;
    idList->next = idents;
    return idList;
}

OpTree *buildOp(OpType op, OpTree *left, OpTree *right) {
    OpTree *opt = new OpTree;
    opt->evalType = OP;
    opt->left = left;
    opt->right = right;
    opt->httype = intersectionHTType(left->httype, right->httype);
    opt->opType = op;
    return opt;
}

OpTree *buildAggFunc(AggFuncType ftype, const char *ident) {
    OpTree *opt = new OpTree;
    opt->left = NULL, opt->right = NULL;
    opt->evalType = AGGFUNC;
    opt->ftype = ftype;
    if (ftype == COUNT) {
        opt->httype = COUNT_HT;
    } else if (ftype == AVG) {
        opt->httype = BOTH_HT;
    } else {
        opt->httype = VALUE_HT;
    }
    opt->varKey = std::string(ident, strlen(ident));
    return opt;
}

OpTree *buildVar(const char *ident) {
    OpTree *opt = new OpTree;
    opt->left = NULL, opt->right = NULL;
    opt->evalType = VAR;
    opt->httype = NONE_HT;
    opt->varKey = std::string(ident, strlen(ident));
    return opt;
}

OpTree *buildConstString(const char *svalue) {
    OpTree *opt = new OpTree;
    opt->left = NULL, opt->right = NULL;
    opt->evalType = CONST;
    opt->httype = NONE_HT;
    opt->constData.p = (uint8_t *)svalue;
    opt->type = TEXT;
    return opt;
}

OpTree *buildSignedNumber(OpTree *opt, bool isNeg) {
    if (isNeg) {
        if (opt->type == INT) {
            opt->constData.i *= -1;
        } else if (opt->type == DOUBLE) {
            opt->constData.d *= -1.0;
        }
    }
    return opt;
}

OpTree *buildConstInt(int ivalue) {
    OpTree *opt = new OpTree;
    opt->left = NULL, opt->right = NULL;
    opt->evalType = CONST;
    opt->httype = NONE_HT;
    opt->constData.i = ivalue;
    opt->type = INT;
    return opt;
}

OpTree *buildConstDouble(double dvalue) {
    OpTree *opt = new OpTree;
    opt->left = NULL, opt->right = NULL;
    opt->evalType = CONST;
    opt->constData.d = dvalue;
    opt->type = DOUBLE;
    return opt;
}

PredTree *buildCond(CondType ctype, PredTree *left, PredTree *right) {
    // todo: eval const pred
    PredTree *prt = new PredTree;
    prt->evalType = COND;
    prt->left = left;
    prt->right = right;
    prt->ctype = ctype;
    return prt;
}

PredTree *buildCCond(CondType ctype, PredTree *cond, OpTree *ccond) {
    if (ctype == DEFAULT && cond == NULL) {
        PredTree *prt = new PredTree;
        prt->evalType = CPRED;
        prt->left = NULL, prt->right = NULL;
        prt->cvalue = ccond->constData.i ? true : false;
        return prt;
    } else if (ctype == AND) {
        if (ccond->constData.i) {
            delete ccond;
            return cond;
        } else {
            PredTree *prt = new PredTree;
            prt->left = NULL, prt->right = NULL;
            prt->evalType = CPRED;
            prt->cvalue = false;
            delete cond, delete ccond;
            return prt;
        }
    } else if (ctype == OR) {
        if (ccond->constData.i) {
            PredTree *prt = new PredTree;
            prt->left = NULL, prt->right = NULL;
            prt->evalType = CPRED;
            prt->cvalue = true;
            delete cond, delete ccond;
            return prt;
        } else {
            delete ccond;
            return cond;
        }
    }
    return cond;
}

PredTree *buildPred(OpType op, OpTree *left, OpTree *right) {
    PredTree *prt = new PredTree;
    prt->left = NULL, prt->right = NULL;
    prt->evalType = PRED;

    OpTree *opt = buildOp(op, left, right);
    prt->pred = opt;

    return prt;
}

OpTree *evalConstPred(OpType opt, OpTree *left, OpTree *right) {
    OpTree *op = new OpTree;
    op->left = NULL, op->right = NULL;
    op->evalType = CONST;
    op->httype = NONE_HT;
    op->type = INT;
    int li, ri;
    double ld, rd;

    if (left->type == DOUBLE && right->type == DOUBLE) {
        ld = left->constData.d, rd = right->constData.d;
    } else if (left->type == DOUBLE && right->type == INT) {
        ld = left->constData.d, rd = (double)right->constData.i;
    } else if (left->type == INT && right->type == DOUBLE) {
        ld = (double)left->constData.i, rd = right->constData.d;
    } else {
        li = left->constData.i, ri = right->constData.i;
    }

    if (left->type == INT && right->type == INT) {
        if (opt == EQUAL) {
            op->constData.i = li == ri;
        } else if (opt == NEQUAL) {
            op->constData.i = li != ri;
        } else if (opt == LESS) {
            op->constData.i = li < ri;
        } else if (opt == LESSEQ) {
            op->constData.i = li <= ri;
        } else if (opt == GREATER) {
            op->constData.i = li > ri;
        } else if (opt == GREATEQ) {
            op->constData.i = li >= ri;
        }
    } else {
        if (opt == EQUAL) {
            op->constData.i = ld == rd;
        } else if (opt == NEQUAL) {
            op->constData.i = ld != rd;
        } else if (opt == LESS) {
            op->constData.i = ld < rd;
        } else if (opt == LESSEQ) {
            op->constData.i = ld <= rd;
        } else if (opt == GREATER) {
            op->constData.i = ld > rd;
        } else if (opt == GREATEQ) {
            op->constData.i = ld >= rd;
        }
    }
    delete left, delete right;
    return op;
}

OpTree *evalConstCond(CondType ctype, OpTree *left, OpTree *right) {
    OpTree *op = new OpTree;
    op->left = NULL, op->right = NULL;
    op->evalType = CONST;
    op->httype = NONE_HT;
    op->type = INT;
    if (ctype == AND) {
        op->constData.i = left->constData.i && right->constData.i;
    } else if (ctype == OR) {
        op->constData.i = left->constData.i || right->constData.i;
    }
    delete left, delete right;
    return op;
}

OpTree *evalConstExpr(OpType opt, OpTree *left, OpTree *right) {
    OpTree *op = new OpTree;
    op->left = NULL, op->right = NULL;
    op->evalType = CONST;
    op->httype = NONE_HT;
    int li, ri;
    double ld, rd;

    if (left->type == DOUBLE && right->type == DOUBLE) {
        ld = left->constData.d, rd = right->constData.d;
    } else if (left->type == DOUBLE && right->type == INT) {
        ld = left->constData.d, rd = (double)right->constData.i;
    } else if (left->type == INT && right->type == DOUBLE) {
        ld = (double)left->constData.i, rd = right->constData.d;
    } else {
        li = left->constData.i, ri = right->constData.i;
    }

    if (left->type == INT && right->type == INT) {
        op->type = INT;
        if (opt == ADD) {
            op->constData.i = li + ri;
        } else if (opt == SUB) {
            op->constData.i = li - ri;
        } else if (opt == MUL) {
            op->constData.i = li * ri;
        } else if (opt == DIV) {
            op->constData.i = (int)(li / ri);
        }
    } else {
        op->type = DOUBLE;
        if (opt == ADD) {
            op->constData.d = ld + rd;
        } else if (opt == SUB) {
            op->constData.d = ld - rd;
        } else if (opt == MUL) {
            op->constData.d = ld * rd;
        } else if (opt == DIV) {
            op->constData.d = ld / rd;
        }
    }
    delete left, delete right;
    return op;
}
