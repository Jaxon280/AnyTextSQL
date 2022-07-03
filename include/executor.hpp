#pragma once

#include "common.hpp"
#include "converter.hpp"
#include "evaluator.hpp"
#include "interface.hpp"
#include "query.hpp"

namespace vlex {
class Token {
    std::vector<DATA_TYPE> str;
    int tokenType = 0;

   public:
    void set_literals(DATA_TYPE *data, SIZE_TYPE start, SIZE_TYPE size) {
        DATA_TYPE buf[size + 1];
        buf[size] = (DATA_TYPE)0;
        memcpy(buf, &data[start], size);
        str = std::vector<DATA_TYPE>(buf, buf + size + 1);
    }
    std::vector<DATA_TYPE> get_literals() { return str; };
};

class Executor {
   private:
    struct Context {
        ST_TYPE recentAcceptState;
        ST_TYPE currentState;
        SIZE_TYPE recentAcceptIndex;
        SIZE_TYPE tokenStartIndex;
    };

    struct SubMatchNode {
        int id;
        SIZE_TYPE index;
    };

    struct SubMatch {
        SIZE_TYPE start;
        SIZE_TYPE end;
    };

    Executor::Context ctx;
    std::vector<Token> tokenVec;
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

    std::stack<SubMatchNode> startStack;
    SubMatchNode *end;
    SubMatch *subMatches;  // id -> SubMatch
    int subMatchSize;

    DATA_TYPE *data;
    SIZE_TYPE size;
    SIZE_TYPE i;

    SIZE_TYPE tid = 0;    // tuple index for vectorized execution
    SIZE_TYPE **indexes;  // key * id -> index
    SIZE_TYPE **sizes;    // key * id -> size

    data64 **bufArray;  // key * tupleid -> 64bit data
    Type *keyTypes;

    int numPreds;
    int predANDsize;                  // num of AND preds
    int *predORsize;                  // AND -> num of OR preds
    int **preds;                      // described in CNF. AND * OR -> pred_id
    SIMD_256iTYPE **predMasks;        // pred_id * tupleid -> mask
    QueryContext::OpTree *predTrees;  // pred_id -> tree
    Type *predTypes;                  // pred_id -> pred type

    int *selectionVector;  // array of selected tupleid
    int selVecSize;

    int numProjKeys;  // selected keys are 0, 1, ..., n - 1

    QueryContext::Aggregation *aggContext;
    std::unordered_map<std::string, data64> aggMap;
    std::unordered_map<std::string, int64_t> aggCountMap;
    data64 agg;
    int64_t aggCount = 0;

    int limit = 0;

    inline void cmpestri_ord(ST_TYPE cur_state);
    inline void cmpestri_any(ST_TYPE cur_state);
    inline void cmpestri_ranges(ST_TYPE cur_state);
    void generateToken(std::vector<Token> &token_vec, DATA_TYPE *data,
                       SIZE_TYPE start, SIZE_TYPE end);
    inline void startSubMatch(int id);
    inline void endSubMatch(int id);
    inline void resetContext();

    void vmaterialize();
    void vevalPreds();
    void veval();
    void vselection();
    void projection();
    void aggregation0();
    void aggregation1();
    void aggregation();
    void queryVExec();
    void materialize(int tsize);
    void selection(int tsize);
    void queryExec(int tsize);
    void printAggregation0();
    void printAggregation1();
    void queryEndExec();

   public:
    Executor();
    Executor(VectFA *vfa, QueryContext *query, SIZE_TYPE _start);
    void setVFA(VectFA *vfa, SIZE_TYPE _start);
    void setQuery(QueryContext *query);
    void exec(DATA_TYPE *_data, SIZE_TYPE size);
    inline std::vector<Token> getTokenVec() { return tokenVec; }
};
}  // namespace vlex
