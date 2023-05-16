#pragma once

#include "common/common.hpp"
#include "scanner/dfa.hpp"
#include "scanner/pfa.hpp"
#include "scanner/sets.hpp"

#define ORD_LENGTH 4

namespace vlex {
struct Delta {
    ST_TYPE startState;
    std::string str;
    std::string backStr;  // for delta_ord
    std::vector<ST_TYPE> charTable;// for delta_{any, c}
    std::vector<ST_TYPE> rTable; // for delta_ord
    // int count; // for *_MASK
};

typedef enum _simd_kind { ORDERED, ANY, RANGES, CMPEQ, C, INV } SIMDKind;

struct Qlabel {
    SIMDKind kind;
    Delta *delta;

    bool isAccept = false;
};

class VectFA {
   public:
    struct SubMatchStates {
        int id;
        Type type;
        // int predUUID;
        // std::set<int> predIDs;
        std::set<DFA_ST_TYPE> startStates;
        std::set<DFA_ST_TYPE> states;

        SubMatchStates(int _id, Type _type,
                       std::set<DFA_ST_TYPE> _startStates,
                       std::set<DFA_ST_TYPE> _states)
            : id(_id),
              type(_type),
              startStates(_startStates),
              states(_states) {}
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
    std::set<ST_TYPE> SMStates;
    std::map<ST_TYPE, ST_TYPE> dfas2vfas;
};
}  // namespace vlex
