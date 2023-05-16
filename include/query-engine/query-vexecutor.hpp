#pragma once

#include "common/common.hpp"
#include "query-engine/pred-evaluator.hpp"

namespace vlex {
class QueryVExecutor {
   private:
    SIZE_TYPE tid = 0;    // tuple index for vectorized execution
    SIZE_TYPE **indexes;  // key * id -> index
    SIZE_TYPE **sizes;    // key * id -> size

    data64 **bufArray;  // key * tupleid -> 64bit data

    uint8_t **predMasks;  // pred_id * tupleid -> mask
    uint16_t *mask;

    int *tupleIds;
    int *selectionVector;  // array of selected tupleid
    int selVecSize = 0;

    bool isCNF;
    int numPreds;
    int predANDsize;    // num of AND preds
    int *predORsize;    // AND -> num of OR preds
    int **preds;        // described in CNF. AND * OR ->
    OpTree *predTrees;  // pred_id -> tree Type
    Type *predTypes;    // pred_id -> pred type

    void vmaterialize();
    void vevalPreds();
    void veval();
    inline void vselection();
    void vprojection();
    void vaggregation0();
    void vaggregation1();
    inline void vaggregation();
    inline void queryVExec();

   public:
    QueryVExecutor() {}
    void materialize(int tid);
    inline void queryVExec();
};
}  // namespace vlex
