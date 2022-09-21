#pragma once

#include "common.hpp"
#include "sets.hpp"

namespace vlex {
class PFA {
   public:
    PFA(std::vector<std::vector<ST_TYPE>> &_transTable, int _numStates,
        DATA_TYPE *_data, SIZE_TYPE _size, SIZE_TYPE _start);
    void scanSubString(const std::vector<vlex::Qstar> &Qstars);
    std::map<ST_TYPE, int> calcSubString() const;

    void scan();
    double calc(ST_TYPE j, ST_TYPE k) const;
    void calc() const;

   private:
    // delta_ord
    std::map<ST_TYPE, std::string> patterns;
    std::map<ST_TYPE, std::vector<std::vector<int>>> countOrds;

    // delta_any
    int **countMat;
    int *sumCursVec;
    int *sumNextsVec;
    int currentState = INIT_STATE;
    int sumAll;

    // common
    std::vector<std::vector<ST_TYPE>> &transTable;
    int numStates;
    DATA_TYPE *data;
    SIZE_TYPE size;
    SIZE_TYPE start;
};
}  // namespace vlex
