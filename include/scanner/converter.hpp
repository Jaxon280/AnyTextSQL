#pragma once

#include "common.hpp"
#include "scanner/codegen.hpp"
#include "scanner/dfa.hpp"
#include "scanner/interface.hpp"
#include "scanner/pfa.hpp"

#define ORD_LENGTH 4

namespace vlex {
class VectFA {
    std::vector<std::vector<ST_TYPE>> transTable;
    std::set<ST_TYPE> states;
    std::set<ST_TYPE> acceptStates;
    std::vector<Qlabel> qlabels;
    std::vector<DFA::SubMatchStates> subMatches;
    std::set<ST_TYPE> charSMStates;
    std::set<ST_TYPE> anySMStates;
    std::map<ST_TYPE, ST_TYPE> old2new;

    std::vector<ST_TYPE> construct_Qs();

    std::vector<Qstar> construct_Qstars(std::vector<ST_TYPE> Qsource);

    std::set<ST_TYPE> construct_Qtilde(const std::set<ST_TYPE> &Qstar_source);

    void construct_delta_ords(const std::vector<Qstar> &Qstar_set,
                              std::map<ST_TYPE, int> opt_poses);

    int construct_delta_ranges(Delta *trans, const std::vector<int> &chars);
    void construct_delta_anys(std::set<ST_TYPE> &Qtilde, const PFA &pfa);
    void construct_delta_cs(const std::set<ST_TYPE> &Qstar_source,
                            const std::set<ST_TYPE> &Qtilde);

   public:
    VectFA(DFA &dfa);
    VectFA(DFA &dfa, DATA_TYPE *_data, SIZE_TYPE _size);
    void constructVFA(DFA &dfa, DATA_TYPE *data, SIZE_TYPE _size);

    inline std::set<ST_TYPE> getStates() { return states; }
    inline std::set<ST_TYPE> getAcceptStates() { return acceptStates; }
    inline std::vector<Qlabel> &getQlabels() { return qlabels; }
    inline std::vector<DFA::SubMatchStates> &getSubMatches() {
        return subMatches;
    }
    int codegen(const std::string &filename);
};
}  // namespace vlex
