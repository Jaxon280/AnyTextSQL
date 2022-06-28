#include "query.hpp"

using namespace vlex;

QueryContext generate_businessid_query() {
    QueryContext::Selection sel;
    sel.isCNF = true;
    sel.numPreds = 1;
    sel.preds = {{0}};
    sel.predTypes = {TEXT};
    QueryContext::OpTree optree1;
    optree1.type = DOUBLE;
    optree1.evalType = OP;
    optree1.opType = NEQUAL;
    optree1.left = new QueryContext::OpTree;
    optree1.left->evalType = VAR;
    optree1.left->varKey = 0;
    optree1.right = new QueryContext::OpTree;
    optree1.right->evalType = VAR;
    optree1.right->varKey = 0;

    sel.predTrees = {optree1};

    std::vector<Type> types = {TEXT};

    QueryContext query(sel, types);
    return query;
}

QueryContext generate_stars_query_complicate() {
    QueryContext::Selection sel;
    sel.isCNF = true;
    sel.numPreds = 6;
    sel.preds = {{0, 1}, {2, 3}, {4, 5}};
    sel.predTypes = {DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE, DOUBLE};
    QueryContext::OpTree optree1;
    optree1.type = DOUBLE;
    optree1.evalType = OP;
    optree1.opType = LESSEQ;
    optree1.left = new QueryContext::OpTree;
    optree1.left->evalType = VAR;
    optree1.left->varKey = 0;
    optree1.right = new QueryContext::OpTree;
    optree1.right->evalType = VAR;
    optree1.right->varKey = 0;

    QueryContext::OpTree optree2;
    optree2.type = DOUBLE;
    optree2.evalType = OP;
    optree2.opType = GREATEQ;
    optree2.left = new QueryContext::OpTree;
    optree2.left->evalType = VAR;
    optree2.left->varKey = 0;
    optree2.right = new QueryContext::OpTree;
    optree2.right->evalType = CONST;
    optree2.right->constData.d = 4.5;

    QueryContext::OpTree optree3;
    optree3.type = DOUBLE;
    optree3.evalType = OP;
    optree3.opType = LESSEQ;
    optree3.left = new QueryContext::OpTree;
    optree3.left->evalType = VAR;
    optree3.left->varKey = 0;
    optree3.right = new QueryContext::OpTree;
    optree3.right->evalType = CONST;
    optree3.right->constData.d = 0.3;

    QueryContext::OpTree optree4;
    optree4.type = DOUBLE;
    optree4.evalType = OP;
    optree4.opType = GREATEQ;
    optree4.left = new QueryContext::OpTree;
    optree4.left->evalType = VAR;
    optree4.left->varKey = 0;
    optree4.right = new QueryContext::OpTree;
    optree4.right->evalType = CONST;
    optree4.right->constData.d = 4.7;

    QueryContext::OpTree optree5;
    optree5.type = DOUBLE;
    optree5.evalType = OP;
    optree5.opType = LESSEQ;
    optree5.left = new QueryContext::OpTree;
    optree5.left->evalType = VAR;
    optree5.left->varKey = 0;
    optree5.right = new QueryContext::OpTree;
    optree5.right->evalType = CONST;
    optree5.right->constData.d = 0.1;

    QueryContext::OpTree optree6;
    optree6.type = DOUBLE;
    optree6.evalType = OP;
    optree6.opType = GREATEQ;
    optree6.left = new QueryContext::OpTree;
    optree6.left->evalType = VAR;
    optree6.left->varKey = 0;
    optree6.right = new QueryContext::OpTree;
    optree6.right->evalType = CONST;
    optree6.right->constData.d = 4.9;

    sel.predTrees = {optree1, optree2, optree3, optree4, optree5, optree6};

    std::vector<Type> types = {DOUBLE};

    QueryContext query(sel, types);
    return query;
}

QueryContext generate_stars_query() {
    QueryContext::Selection sel;
    sel.isCNF = true;
    sel.numPreds = 2;
    sel.preds = {{0, 1}};
    sel.predTypes = {DOUBLE, DOUBLE};

    QueryContext::OpTree optree1;
    optree1.type = DOUBLE;
    optree1.evalType = OP;
    optree1.opType = EQUAL;
    optree1.left = new QueryContext::OpTree;
    optree1.left->evalType = VAR;
    optree1.left->varKey = 0;
    optree1.right = new QueryContext::OpTree;
    optree1.right->evalType = CONST;
    optree1.right->constData.d = 0.1;

    QueryContext::OpTree optree2;
    optree2.type = DOUBLE;
    optree2.evalType = OP;
    optree2.opType = GREATER;
    optree2.left = new QueryContext::OpTree;
    optree2.left->evalType = VAR;
    optree2.left->varKey = 0;

    optree2.right = new QueryContext::OpTree;
    optree2.right->evalType = OP;
    optree2.right->opType = MUL;
    optree2.right->type = DOUBLE;
    optree2.right->left = new QueryContext::OpTree;
    optree2.right->right = new QueryContext::OpTree;
    optree2.right->left->evalType = VAR;
    optree2.right->left->varKey = 0;
    optree2.right->right->evalType = CONST;
    optree2.right->right->constData.d = 0.5;

    // optree2.right->left = new QueryContext::OpTree;
    // optree2.right->right = new QueryContext::OpTree;
    // optree2.right->left->left = new QueryContext::OpTree;
    // optree2.right->left->right = new QueryContext::OpTree;
    // optree2.right->left->evalType = OP;
    // optree2.right->left->opType = ADD;
    // optree2.right->left->type = DOUBLE;
    // optree2.right->left->left->evalType = VAR;
    // optree2.right->left->left->varKey = 0;
    // optree2.right->left->right->evalType = CONST;
    // optree2.right->left->right->constData.d = 0.5;
    // optree2.right->right->evalType = CONST;
    // optree2.right->right->constData.d = 0.5;

    sel.predTrees = {optree1, optree2};

    std::vector<Type> types = {DOUBLE};

    QueryContext query(sel, types);
    return query;
}
