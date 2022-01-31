#pragma once

#include "common.hpp"
#include "sets.hpp"

namespace vlex {
class PFA {
   private:
    int ***count_ord;
    int **sum_curs_ord;   // sum by row
    int **sum_nexts_ord;  // sum by column
    int *currentStates;
    // std::string pattern = "\"categories\":\"";
    std::string pattern;
    int n_patterns;
    int n_states;

    int **count_mat;
    int *sum_curs_vec;
    int *sum_nexts_vec;
    int currentState = INIT_STATE;

    int sum_all;

    int numStates;
    ST_TYPE **dfa;

    ST_TYPE ***ord_dfa;
    char *data;
    int size;
    int i;

   public:
    PFA(ST_TYPE **dfa, int numStates, char *data, int size, int start);
    void construct_ordPFA(std::vector<vlex::Qstar> Qstars, int n_patterns0,
                          int length);
    void scan_ord(double lr);
    int calc_ord();
    void scan(double lr);
    void calc();
};
}  // namespace vlex
