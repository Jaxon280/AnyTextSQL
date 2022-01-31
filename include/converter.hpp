#pragma once

#include "codegen.hpp"
#include "common.hpp"
#include "interface.hpp"
#include "pfa.hpp"

namespace vlex {
class VectFA {
    std::set<ST_TYPE> states;
    std::set<ST_TYPE> acceptStates;
    std::map<ST_TYPE, std::vector<ST_TYPE>> dfa;
    std::map<ST_TYPE, Qlabel> qlabels;

    std::vector<ST_TYPE> construct_Qs(int state_sz);

    std::vector<Qstar> construct_Qstars(std::vector<ST_TYPE> Qsource);

    std::set<ST_TYPE> construct_Qtilde(std::set<ST_TYPE> Qstar_source);

    void construct_delta_ords(std::vector<Qstar> Qstar_set, int opt_pos);

    int construct_delta_ranges(Delta *trans, std::vector<int> &chars);
    void construct_delta_anys(std::set<ST_TYPE> Qtilde);

    void construct_delta_cs(std::set<ST_TYPE> Qstar_source,
                            std::set<ST_TYPE> Qtilde);

   public:
    VectFA(ST_TYPE **fa, ST_TYPE *accepts, int stateSize, int acceptStateSize,
           char *data, int size);
    int codegen(const std::string &filename);
};
}  // namespace vlex
