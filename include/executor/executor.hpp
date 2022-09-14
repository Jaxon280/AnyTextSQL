#pragma once

#include "common.hpp"
#include "hashMap.hpp"
#include "scanner/converter.hpp"
#include "scanner/interface.hpp"
#if (defined VECEXEC)
#include "opEvaluator.hpp"
#include "predEvaluator.hpp"
#endif
#include "parser/query.hpp"

namespace vlex {
class Executor {
   public:
    Executor();
    Executor(VectFA *vfa, QueryContext *query, SIZE_TYPE _start);
    ~Executor();
    void setVFA(VectFA *vfa, SIZE_TYPE _start);
    void setQuery(QueryContext *query);
    void exec(DATA_TYPE *_data, SIZE_TYPE size);

   private:
    // VFA runner
    struct Context {
        ST_TYPE recentAcceptState;
        ST_TYPE currentState;
        SIZE_TYPE recentAcceptIndex;
        SIZE_TYPE tokenStartIndex;
    };

    Executor::Context ctx;
    SIMD_TEXTTYPE *SIMDDatas;
    int *SIMDSizes;
    SIMDKind *kindTable;  // State -> Kind
    ST_TYPE *rTable;      // State -> State
    ST_TYPE **charTable;  // State * char -> State
    int *anyStartTable;   // State -> sub id or 0
    int *anyEndTable;     // State -> sub id or 0
    int *charStartTable;  // State -> sub id or 0
    int *charEndTable;    // State -> sub id or 0
    std::set<ST_TYPE> acceptStates;

    // Submatch
    struct SubMatchNode {
        int id;
        SIZE_TYPE index;
    };

    struct SubMatch {
        SIZE_TYPE start;
        SIZE_TYPE end;
    };

    std::stack<SubMatchNode> startStack;
    SubMatchNode *end;
    SubMatch *subMatches;  // id -> SubMatch
    int subMatchSize;

    // Data
    DATA_TYPE *data;
    SIZE_TYPE size;
    SIZE_TYPE i;

    // tuple and types
#if (defined VECEXEC)
    SIZE_TYPE tid = 0;    // tuple index for vectorized execution
    SIZE_TYPE **indexes;  // key * id -> index
    SIZE_TYPE **sizes;    // key * id -> size

    data64 **bufArray;  // key * tupleid -> 64bit data

    uint8_t **predMasks;  // pred_id * tupleid -> mask
    uint16_t *mask;

    int *tupleIds;
    int *selectionVector;  // array of selected tupleid
    int selVecSize = 0;
#else
    data64 *tuple;     // key -> 64bit data (used only for tuple-at-once)
    SIZE_TYPE *tsize;  // key -> size (used only for tuple-at-once)
#endif
    Type *keyTypes;

    // SELECT clause
    std::list<Statement> stmtList;
    HashTableType httype = NONE_HT;

    // WHERE clause
#if (defined VECEXEC)
    bool isCNF;
    int numPreds;
    int predANDsize;                          // num of AND preds
    int *predORsize;                          // AND -> num of OR preds
    int **preds;                              // described in CNF. AND * OR ->
    pred_id QueryContext::OpTree *predTrees;  // pred_id -> tree Type
    *predTypes;                               // pred_id -> pred type
#else
    PredTree *ptree;
#endif

    // Aggregate functions and GROUP BY clause
    std::vector<Key> gKeyVec;
    std::vector<Aggregation> aggContext;  // id -> agg
    AggregationValueMap *aggMap;
    AggregationCountMap *aggCountMap;
    data64 *agg;
    int *aggCount;

    // ORDER BY clause

    // LIMIT clause
    int limit = 0;

    inline void cmpestri_ord(ST_TYPE cur_state);
    inline void cmpestri_any(ST_TYPE cur_state);
    inline void cmpestri_ranges(ST_TYPE cur_state);
    inline void startSubMatch(int id);
    inline void endSubMatch(int id);
    inline void resetContext();

#if (defined VECEXEC)
    void materialize(int tid);
    void vmaterialize();
    void vevalPreds();
    void veval();
    inline void vselection();
    void vprojection();
    void vaggregation0();
    void vaggregation1();
    inline void vaggregation();
    inline void queryVExec();
#else
    void printColumnNames();
    void queryStartExec();

    void materialize();
    template <typename Value>
    Value evalFunc1Op(OpTree *tree, data64 *vht, int *cht);
    template <typename Value>
    Value evalOp(OpTree *tree);
    bool evalPred(OpTree *tree);
    bool evalCond(PredTree *ptree);
    bool selection();
    void projection();
    void aggregation0();
    void aggregation1();
    void aggregation();
    void queryExec();
#endif

    void printAggregation0();
    void printAggregation1();
    void queryEndExec();
};
}  // namespace vlex
