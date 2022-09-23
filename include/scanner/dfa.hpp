#pragma once

#include "../../interface.hpp"
#include "common.hpp"
#include "parser/nfa.hpp"

namespace vlex {
class DFA {
   public:
    struct SubMatchStates {
        int id;
        Type type;
        int predID;
        std::vector<DFA_ST_TYPE> startStates;
        std::vector<DFA_ST_TYPE> endStates;

        SubMatchStates(int _id, Type _type, int _predID,
                       std::vector<DFA_ST_TYPE> _startStates,
                       std::vector<DFA_ST_TYPE> _endStates)
            : id(_id),
              type(_type),
              predID(_predID),
              startStates(_startStates),
              endStates(_endStates) {}
        SubMatchStates() {}
    };
    using TransTable = std::vector<std::vector<DFA_ST_TYPE>>;
    using SubMatches = std::vector<SubMatchStates>;
    using StateSet = std::set<DFA_ST_TYPE>;

    DFA() {}
    DFA(int _initState, const std::vector<int>& _powsStates,
        const std::map<int, int>& _old2new,
        std::map<int, std::vector<int>>& _powTTable,
        const std::set<int>& _acceptStates,
        const std::vector<SubMatchStates>& _smses);
    DFA(TransTable _transTable, StateSet _acceptStates, SubMatches _subMatches,
        int _numStates)
        : transTable(_transTable),
          acceptStates(_acceptStates),
          subMatches(_subMatches),
          numStates(_numStates) {}  // used in test
    ~DFA() {}

    inline const TransTable& getTransTable() const { return transTable; }
    inline const SubMatches& getSubMatches() const { return subMatches; }
    inline const StateSet& getAcceptStates() const { return acceptStates; }
    inline int getNumStates() const { return numStates; }

   private:
    TransTable transTable;
    StateSet acceptStates;
    SubMatches subMatches;
    int numStates;
    int initState;
};

class DFAGenerator {
   public:
    DFAGenerator(NFA* _nfa) : nfa(_nfa) {
        int stateSize = _nfa->stateSize;

        epsilonTable = std::vector<std::vector<int>>(stateSize);
        powsetTransTable = std::vector<std::vector<std::set<int>>>(
            stateSize, std::vector<std::set<int>>(ASCII_SZ));
    }
    ~DFAGenerator() {
        if (dfa) delete dfa;
    }
    DFA* generate(const KeyMap& keyMap);
    DFA* getDFA() const { return dfa; }

   private:
    void setEpsilonTable(Transition* trans, int transSize, int stateSize);
    void setPowsetTable(Transition* trans, int transSize);
    void initPowsetStates(int initState);
    void setInvStates();
    void minimize();

    std::vector<std::vector<int>> epsilonTable;  // NFA state -> NFA states
    std::vector<std::vector<std::set<int>>>
        powsetTransTable;  // NFA state -> NFA powset state table (128)
    std::stack<int> stateStack;

    NFA* nfa;

    std::map<int, std::set<int>>
        powsetStates;  // DFA state -> NFA powset states
    std::map<int, std::vector<int>> transTable;  // state -> NFA powset state
    std::vector<DFA::SubMatchStates> smses;
    std::map<int, int> default2mini;
    std::set<int> acceptStates;
    DFA* dfa;
    std::vector<int> stateVec;  // minimized DFA states
};
}  // namespace vlex
