#pragma once

#include "common.hpp"
#include "sets.hpp"

namespace vlex {
class PFA {
   private:
    // delta_ord
    std::map<ST_TYPE, std::string> patterns;
    std::map<ST_TYPE, std::vector<std::vector<int>>> count_ords;

    // delta_any
    int **count_mat;
    int *sum_curs_vec;
    int *sum_nexts_vec;
    int currentState = INIT_STATE;
    int sum_all;

    // common
    int numStates;
    std::vector<std::vector<ST_TYPE>> &transTable;
    DATA_TYPE *data;
    SIZE_TYPE size;
    SIZE_TYPE start;

   public:
    PFA(std::vector<std::vector<ST_TYPE>> &_transTable, int _numStates,
        DATA_TYPE *_data, SIZE_TYPE _size, SIZE_TYPE _start);
    void scan_ords(std::vector<vlex::Qstar> Qstars);
    std::map<ST_TYPE, int> calc_ords() const;

    void scan();
    double calc(ST_TYPE j, ST_TYPE k) const;
    void calc() const;
};
}  // namespace vlex
