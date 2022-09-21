#pragma once

#include "common.hpp"
#include "hashMap.hpp"
#include "scanner/converter.hpp"
#include "scanner/interface.hpp"
#if (defined VECEXEC)
#include "queryVExecutor.hpp"
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

    void setTransTable(const std::vector<Qlabel> &qlabels, int stateSize);
    void setSubMatchTable(
        const std::vector<DFA::SubMatchStates> &subMatchStates, int stateSize);
    void setVecDatas(const std::vector<Qlabel> &qlabels, int stateSize);
    void setStatements(const StatementList *stmts);
    void setSelections(QueryContext *query);
    void setAggregations(const std::vector<Key> &gKeyVec);

    inline void cmpestriOrd(ST_TYPE cur_state);
    inline void cmpestriAny(ST_TYPE cur_state);
    inline void cmpestriRanges(ST_TYPE cur_state);
    inline void startSubMatch(int id);
    inline void endSubMatch(int id);
    inline void resetContext();

    void printColumnNames() const;
    void queryStartExec() const;

    void materialize();
    template <typename Value>
    Value evalFunc1Op(const OpTree *tree, const data64 *vht,
                      const int *cht) const;
    template <typename Value>
    Value evalOp(const OpTree *tree) const;
    bool evalPred(const OpTree *tree) const;
    bool evalCond(const PredTree *ptree) const;
    bool selection();
    void projection();
    void aggregation0();
    void aggregation1();
    void aggregation();
    void queryExec();

    void printAggregation0() const;
    void printAggregation1() const;
    void queryEndExec() const;

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
    int *endPredIdTable;  // State -> Text Pred Id
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
    QueryVExecutor *qexec;
#else
#endif

    data64 *tuple;     // key -> 64bit data (used only for tuple-at-once)
    SIZE_TYPE *tsize;  // key -> size (used only for tuple-at-once)
    Type *keyTypes;

    // SELECT clause
    Statement *stmtList;
    int stmtSize;
    HashTableType httype = NONE_HT;

    // WHERE clause
    PredTree *ptree;
    int *textPredResults;
    int textPredNum;
    // Aggregate functions and GROUP BY clause
    int gKeySize;
    Key *gKeys;
    int aggSize;
    Aggregation *aggContexts;  // id -> agg
    AggregationValueMap *aggMap;
    AggregationCountMap *aggCountMap;
    data64 *agg;
    int *aggCount;

    // ORDER BY clause

    // LIMIT clause
    int limit = 0;
};
}  // namespace vlex
