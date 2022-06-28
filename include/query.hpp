#pragma once

#include "common.hpp"

namespace vlex {
typedef enum _op_type {
    EQUAL = 0,     // == (both for numerics and text)
    NEQUAL = 12,   // != (both for numerics and text)
    LESS = 17,     // < (only for numerics)
    LESSEQ = 18,   // <= (only for numerics)
    GREATEQ = 29,  // >= (only for numerics)
    GREATER = 30,  // > (only for numerics)
    REGEXP = 31,   // LIKE, REGEXP (only for text)
    ADD = 1,
    SUB = 2,
    MUL = 3,
    DIV = 4
} OpType;
typedef enum _type { DOUBLE, INT, TEXT } Type;
typedef enum _eval_type { OP, VAR, CONST } EvalType;

typedef enum _aggregate_function {
    DISTINCT,
    COUNT,
    SUM,
    AVG,
    MAX,
    MIN
} AggFuncType;

class QueryContext {
   public:
    struct OpTree {
        EvalType evalType;
        OpTree *left;
        OpTree *right;
        OpType opType;  // only for OP
        Type type;
        int varKey;        // only for VAR
        data64 constData;  // only for CONSTANT
    };

    struct Selection {
        std::vector<OpTree> predTrees;        // pred_id -> pred tree
        std::vector<Type> predTypes;          // pred_id -> pred type
        std::vector<std::vector<int>> preds;  // AND -> OR
        int numPreds;
        bool isCNF;  // CNF or DNF
    };

    struct Aggregation {
        std::vector<int> keys;  // if key is projected, it is seen as DISTINCT
        std::vector<std::pair<int, AggFuncType>> valueKeys;
    };

    struct Projection {
        std::vector<int> columns;
    };

   private:
    int limit;
    Selection sel;
    // Sort sort;
    Aggregation agg;
    // Having having;
    std::vector<int> projColumns;

    std::vector<Type> keyTypes;
    /*
    Query Execution
    1. Make fixed-size array for limit (if limit is assigned)
    2. Make mask/vector for branching in selection
    (2.5. If it is sorted, change fixed-size array to heap and update
    min/max in heap)
    3. Make map/hashtable for aggregation
    (3.5. Having)
    4. Print for projection
    */
    // LIMIT -> Selection -> Sort -> Aggregation -> Having -> Projection
   public:
    QueryContext() {}
    QueryContext(Selection _sel, std::vector<Type> _keyTypes)
        : sel(_sel), keyTypes(_keyTypes) {}
    inline Selection &getSelection() { return sel; }
    inline std::vector<Type> &getKeyTypes() { return keyTypes; }
};
}  // namespace vlex
