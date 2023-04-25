#pragma once

#include "common.hpp"
#include "hashMap.hpp"
#include "scanner/interface.hpp"
#include "scanner/vfa.hpp"
#if (defined VECEXEC)
#include "queryVExecutor.hpp"
#endif
#include "bitset.hpp"
#include "parser/query.hpp"
#include "spark.hpp"

namespace vlex {
class Executor {
   public:
    Executor();
    ~Executor();
    void setFA(VectFA *vfa, SIZE_TYPE _start);
    void setQuery(QueryContext *query);
    void setSparkContext(SparkContext *sctx, QueryContext *qctx);
    void exec(DATA_TYPE *_data, SIZE_TYPE size, SIZE_TYPE start);
    int execWithSpark(DATA_TYPE *_data, SIZE_TYPE _size, SIZE_TYPE start);

    SIZE_TYPE preExec(DATA_TYPE *_data, SIZE_TYPE _size);
    void postExec();

   private:
    // VFA runner
    struct Context {
        ST_TYPE recentAcceptState;
        ST_TYPE currentState;
        SIZE_TYPE recentAcceptIndex;
    };

    void setTransTable(const std::vector<Qlabel> &qlabels, int stateSize);
    void setSubMatchTable(
        const std::vector<VectFA::SubMatchStates> &subMatchStates,
        int stateSize);
    void setVecDatas(const std::vector<Qlabel> &qlabels, int stateSize);
    void setWildCardProjection();
    void setStatements(const StatementList *stmts);
    void setSelections(QueryContext *query);
    void setAggregations(const std::vector<Key> &gKeyVec);

    void cmpestriOrd(ST_TYPE curState);
    void cmpestriAny(ST_TYPE curState);
    void cmpestriRanges(ST_TYPE curState);
    // void cmpestrmAny(ST_TYPE curState);
    // void cmpestrmRanges(ST_TYPE curState);
    void startSubMatch(int id);
    void endSubMatch(int id);
    void resetContext();

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

    void storeToDataFrame();
    void preStoreToDataFrame();

    Executor::Context ctx;
    SIMD_TEXTTYPE *SIMDDatas;
    int *SIMDSizes;
    // int *SIMDCounts;
    SIMDKind *kindTable;       // State -> Kind
    ST_TYPE *rTable;           // State -> State
    ST_TYPE **charTable;       // State * char -> State
    int *anyStartTable;        // State -> sub id or 0
    int *anyEndTable;          // State -> sub id or 0
    int *charStartTable;       // State -> sub id or 0
    int *charEndTable;         // State -> sub id or 0
    uint32_t *endPredIdTable;  // State -> Text Pred Id
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
    DATA_TYPE *prevData = NULL;
    SIZE_TYPE prevStart;
    SIZE_TYPE prevSize;
    SIZE_TYPE size;
    SIZE_TYPE i;

    // tuple and types
#if (defined VECEXEC)
    QueryVExecutor *qexec;
#else
#endif

#if (defined BENCH)
    int chunk = 1;
    double execTotal = 0.0;
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
    BitSet *textPredBits;
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
    int count = 0;

    // ORDER BY clause

    // LIMIT clause
    int limit;

    // Apache Spark
    void *baseDF;
    void *curDF;
    int rowCount = 0;
    int sizeInRow;
    int varSize = 0;
    int *varlens;
    int *varoffsets;
};
}  // namespace vlex
