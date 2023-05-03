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
    struct SubMatchStates {
        int id;
        Type type;
        int predUUID;
        std::set<int> predIDs;
        std::set<DFA_ST_TYPE> charStartStates;
        std::set<DFA_ST_TYPE> charEndStates;
        std::set<DFA_ST_TYPE> anyStartStates;
        std::set<DFA_ST_TYPE> anyEndStates;

        SubMatchStates(int _id, Type _type, int _predUUID,
                       std::set<int> _predIDs,
                       std::set<DFA_ST_TYPE> _charStartStates,
                       std::set<DFA_ST_TYPE> _charEndStates,
                       std::set<DFA_ST_TYPE> _anyStartStates,
                       std::set<DFA_ST_TYPE> _anyEndStates)
            : id(_id),
              type(_type),
              predUUID(_predUUID),
              predIDs(_predIDs),
              charStartStates(_charStartStates),
              charEndStates(_charEndStates),
              anyStartStates(_anyStartStates),
              anyEndStates(_anyEndStates) {}
        SubMatchStates() {}
    };
    VectFA(const DFA &dfa);
    VectFA(DFA &dfa, DATA_TYPE *_data, SIZE_TYPE _size);
    void constructVFA(DFA &dfa, DATA_TYPE *data, SIZE_TYPE _size);

    inline const std::set<ST_TYPE> getStates() const { return states; }
    inline const std::set<ST_TYPE> getAcceptStates() const {
        return acceptStates;
    }
    inline const std::vector<Qlabel> &getQlabels() const { return qlabels; }
    inline const std::vector<VectFA::SubMatchStates> &getSubMatches() const {
        return subMatches;
    }
    inline const std::map<ST_TYPE, ST_TYPE> &getStateMap() const {
        return dfas2vfas;
    }
#if (defined CODEGEN)
    int codegen(const std::string &filename);
#endif

   private:
    std::vector<ST_TYPE> constructQs() const;
    std::vector<Qstar> constructQstars() const;
    std::set<ST_TYPE> constructQtilde(const std::set<ST_TYPE> &QstarSource);
    // std::vector<std::list<ST_TYPE>> constructQtildeChain(std::set<ST_TYPE> &Qtilde);
    void constructDeltaOrds(const std::vector<Qstar> &Qstar_set,
                            std::map<ST_TYPE, int> opt_poses);
    int constructDeltaRanges(Delta *trans, const std::vector<int> &chars);
    void constructDeltaAnys(std::set<ST_TYPE> &Qtilde, const PFA &pfa);
    // void constructDeltaAnyMasks(std::vector<std::list<ST_TYPE>> &QtildeChains);
    void constructDeltaCs(const std::set<ST_TYPE> &QstarSource,
                          const std::set<ST_TYPE> &Qtilde);
    void mapStateDFA2VFA();
    void constructSubmatches(const std::vector<DFA::SubMatchStates> &SMSs);

    std::map<ST_TYPE, std::vector<ST_TYPE>> Qtilde;

    std::vector<std::vector<ST_TYPE>> transTable;
    std::set<ST_TYPE> states;
    std::set<ST_TYPE> acceptStates;
    std::vector<Qlabel> qlabels;
    std::vector<VectFA::SubMatchStates> subMatches;
    std::set<ST_TYPE> charSMStates;
    std::map<ST_TYPE, ST_TYPE> dfas2vfas;
};
}  // namespace vlex
