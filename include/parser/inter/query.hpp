#pragma once

#include "common/string-list.hpp"
#include "runner/table.hpp"

namespace vlex {

typedef enum _op_type {
    EQUAL = 0,      // == (both for numerics and text)
    NEQUAL = 12,    // != (both for numerics and text)
    ISNULL = 5,     // IS NULL
    ISNOTNULL = 6,  // IS NOT NULL
    LESS = 17,      // < (only for numerics)
    LESSEQ = 18,    // <= (only for numerics)
    GREATEQ = 29,   // >= (only for numerics)
    GREATER = 30,   // > (only for numerics)
    LIKE = 31,      // LIKE (only for text)
    REGEXP = 32,    // REGEXP (only for text)
    ADD = 1,
    SUB = 2,
    MUL = 3,
    DIV = 4
} OpType;
typedef enum _op_eval_type { OP, VAR, CONST, AGGFUNC } OpEvalType;
typedef enum _pred_eval_type { COND, PRED, CPRED } PredEvalType;
typedef enum _cond_type { DEFAULT, AND, OR, NOT } CondType;
typedef enum _hashtable_type {
    NONE_HT,
    COUNT_HT,
    VALUE_HT,
    BOTH_HT
} HashTableType;
typedef enum _aggregate_function_type {
    DISTINCT,
    COUNT,
    SUM,
    AVG,
    MAX,
    MIN
} AggFuncType;

struct OpTree {
    OpEvalType evalType;
    vlex::Type type;
    HashTableType httype;

    OpTree *left;   // only for OP
    OpTree *right;  // only for OP
    OpType opType;  // only for OP

    AggFuncType ftype;   // only for AGGFUNC
    int aggId;           // only for AGGFUNC
    std::string varKey;  // only for VAR/AGGFUNC
    int varKeyId;        // only for VAR/AGGFUNC

    int textPredId;  // only for TEXT PRED

    data64 constData;  // only for CONSTANT
};

struct PredTree {
    PredEvalType evalType;

    CondType ctype;   // only for COND
    PredTree *left;   // only for COND
    PredTree *right;  // only for COND

    OpTree *pred;  // only for PRED

    bool cvalue;  // only for CONST
};

struct Statement {
    Statement() {}
    Statement(OpTree *expr, const char *name, HashTableType httype,
              bool isWildCard)
        : expr(expr), name(name), httype(httype), isWildCard(isWildCard) {}
    OpTree *expr;
    const char *name;
    HashTableType httype;
    bool isWildCard;
};

struct StatementList {
    Statement *stmt;
    StatementList *next;
};

struct Aggregation {
    Type type;
    AggFuncType ftype;
    int keyId;
    bool isWildCard;
};

inline HashTableType intersectionHTType(const HashTableType a,
                                        const HashTableType b) {
    if (a == NONE_HT) {
        return b;
    } else if (a == COUNT_HT) {
        if (b == VALUE_HT || b == BOTH_HT) {
            return BOTH_HT;
        } else {
            return COUNT_HT;
        }
    } else if (a == VALUE_HT) {
        if (b == COUNT_HT || b == BOTH_HT) {
            return BOTH_HT;
        } else {
            return VALUE_HT;
        }
    } else {
        return BOTH_HT;
    }
}

class QueryContext {
   public:
    QueryContext() {}
    void mapping(const KeyMap &keyMap) {
        for (StatementList *s = output; s != NULL; s = s->next) {
            mapOpTree(s->stmt->expr, keyMap);
            addType(s->stmt->expr);
        }
        if (pList.size() != 0) {
            mapPredTree(pList, keyMap);
            countTextPred();
        }
        for (StringList *k = gKeys; k != NULL; k = k->next) {
            std::string s(k->str, strlen(k->str));
            gKeyVec.push_back(keyMap.at(s));
        }
        for (StringList *k = oKeys; k != NULL; k = k->next) {
            std::string s(k->str, strlen(k->str));
            oKeyVec.push_back(keyMap.at(s));
        }
    }
    inline const StatementList *getStatements() const { return output; }
    inline const StringList *getTables() const { return tablenames; }
    inline PredTree *getPredTree() { return pTree; }
    inline const std::vector<OpTree *> &getPList() const { return pList; }
    inline int getTextPredNum() const { return countTPred; }
    inline const std::vector<Key> &getGKeys() const { return gKeyVec; }
    inline const std::vector<Key> &getOKeys() const { return oKeyVec; }
    inline const std::vector<vlex::Type> &getKeyTypes() const {
        return keyTypes;
    }
    inline int getLimit() const { return limit; }
    inline bool isError() const { return errorno; }
    void assignStmts(StatementList *_stmts) { output = _stmts; }
    void assignTables(StringList *_tables) { tablenames = _tables; }
    void assignPredTree(PredTree *_ptree) {
        pTree = _ptree;
        createPList(_ptree);
    }
    void assignGroupKeys(StringList *_keys) { gKeys = _keys; }
    void assignOrderKeys(StringList *_keys) { oKeys = _keys; }
    void assignLimit(int _limit) { limit = _limit; }
    void grammarError(const char *s) {
        printf("ERROR: %s\n\n", s);
        errorno = 1;
    }

   private:
    void addType(OpTree *tree) {
        if (tree != NULL) {
            addTypeOp(tree);
        }
    }

    Type addTypeOp(OpTree *tree) {
        if (tree->evalType == OP) {
            Type lt = addTypeOp(tree->left);
            Type rt = addTypeOp(tree->right);
            if (lt == INT && rt == INT) {
                tree->type = INT;
            } else if (lt == TEXT && rt == TEXT) {
                tree->type = TEXT;
            } else if ((lt == INT && rt == DOUBLE) ||
                       (lt == DOUBLE && rt == INT) ||
                       (lt == DOUBLE && rt == DOUBLE)) {
                tree->type = DOUBLE;
            } else {
                tree->type = INVALID;
            }
        }
        return tree->type;
    }

    void mapOpTree(OpTree *tree, const KeyMap &keyMap) {
        OpTree *t;
        std::queue<OpTree *> bfsQueue;
        if (tree != NULL) {
            bfsQueue.push(tree);
        }
        while (!bfsQueue.empty()) {
            t = bfsQueue.front();
            if (t->left != NULL) {
                bfsQueue.push(t->left);
            }
            if (t->right != NULL) {
                bfsQueue.push(t->right);
            }

            if (t->evalType == VAR || t->evalType == AGGFUNC) {
                if (keyMap.find(t->varKey)) {
                    const Key &key = keyMap.at(t->varKey);
                    t->varKeyId = key.id;
                    if (t->evalType == AGGFUNC && t->ftype == COUNT) {
                        t->type = INT;
                    } else {
                        t->type = key.type;
                    }
                } else {
                    return;
                }
            }
            bfsQueue.pop();
        }
    }

    void mapPredTree(std::vector<OpTree *> &pList, const KeyMap &keyMap) {
        for (OpTree *opt : pList) {
            mapOpTree(opt, keyMap);
            addType(opt);
        }
    }

    void createPList(PredTree *pt) {
        if (pt == NULL) {
            return;
        }

        PredTree *t;
        std::queue<PredTree *> bfsQueue;
        bfsQueue.push(pt);
        while (!bfsQueue.empty()) {
            t = bfsQueue.front();
            if (t->left != NULL) {
                bfsQueue.push(t->left);
            }
            if (t->right != NULL) {
                bfsQueue.push(t->right);
            }

            if (t->evalType == PRED) {
                pList.push_back(t->pred);
            }
            bfsQueue.pop();
        }
    }

    void countTextPred() {
        countTPred = 0;
        for (OpTree *opt : pList) {
            if (opt->type == TEXT) {
                countTPred++;
            }
        }
    }

    std::vector<vlex::Type> keyTypes;
    int limit;
    PredTree *pTree;
    std::vector<OpTree *> pList;
    int countTPred;
    StatementList *output;
    StringList *tablenames;
    StringList *gKeys;
    StringList *oKeys;
    std::vector<Key> gKeyVec;
    std::vector<Key> oKeyVec;
    int errorno = 0;
};

}  // namespace vlex
