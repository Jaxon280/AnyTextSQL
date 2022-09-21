#pragma once

#include "common.hpp"
#if (defined CODEGEN)
#include "scanner/codegen.hpp"
#endif
#include "scanner/dfa.hpp"
#include "scanner/interface.hpp"
#include "scanner/pfa.hpp"

#define ORD_LENGTH 4

namespace vlex {
class VectFA {
   public:
    VectFA(const DFA &dfa);
    VectFA(DFA &dfa, DATA_TYPE *_data, SIZE_TYPE _size);
    void constructVFA(DFA &dfa, DATA_TYPE *data, SIZE_TYPE _size);

    inline const std::set<ST_TYPE> getStates() const { return states; }
    inline const std::set<ST_TYPE> getAcceptStates() const {
        return acceptStates;
    }
    inline const std::vector<Qlabel> &getQlabels() const { return qlabels; }
    inline const std::vector<DFA::SubMatchStates> &getSubMatches() const {
        return subMatches;
    }
#if (defined CODEGEN)
    int codegen(const std::string &filename);
#endif

   private:
    std::vector<ST_TYPE> constructQs() const;
    std::vector<Qstar> constructQstars() const;
    std::set<ST_TYPE> constructQtilde(const std::set<ST_TYPE> &QstarSource);
    void constructDeltaOrds(const std::vector<Qstar> &Qstar_set,
                            std::map<ST_TYPE, int> opt_poses);
    int constructDeltaRanges(Delta *trans, const std::vector<int> &chars);
    void constructDeltaAnys(std::set<ST_TYPE> &Qtilde, const PFA &pfa);
    void constructDeltaCs(const std::set<ST_TYPE> &QstarSource,
                          const std::set<ST_TYPE> &Qtilde);

    std::vector<std::vector<ST_TYPE>> transTable;
    std::set<ST_TYPE> states;
    std::set<ST_TYPE> acceptStates;
    std::vector<Qlabel> qlabels;
    std::vector<DFA::SubMatchStates> subMatches;
    std::set<ST_TYPE> charSMStates;
    std::set<ST_TYPE> anySMStates;
    std::map<ST_TYPE, ST_TYPE> old2new;
};
}  // namespace vlex
