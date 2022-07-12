#pragma once

#include "c/regex_tree.h"
#include "common.hpp"

namespace vlex {
class DFA {
   public:
    struct SubMatchStates {
        DFA_ST_TYPE startState;
        bool isAnyStart;
        DFA_ST_TYPE endState;
        bool isAnyEnd;

        SubMatchStates(DFA_ST_TYPE _startState, bool _isAnyStart,
                       DFA_ST_TYPE _endState, bool _isAnyEnd)
            : startState(_startState),
              isAnyStart(_isAnyStart),
              endState(_endState),
              isAnyEnd(_isAnyEnd) {}
        SubMatchStates() {}
    };
    using TransTable = std::vector<std::vector<DFA_ST_TYPE>>;
    using SubMatches = std::vector<SubMatchStates>;
    using StateSet = std::set<DFA_ST_TYPE>;

   private:
    TransTable transTable;
    StateSet acceptStates;
    SubMatches subMatches;
    int numStates;
    int initState;

   public:
    DFA() {}
    DFA(int _initState, std::vector<int>& _powsStates,
        std::map<int, int>& _old2new,
        std::map<int, std::vector<int>>& _powTTable,
        std::set<int>& _acceptStates, std::vector<SubMatchStates> _smses);
    DFA(TransTable _transTable, StateSet _acceptStates, SubMatches _subMatches,
        int _numStates)
        : transTable(_transTable),
          acceptStates(_acceptStates),
          subMatches(_subMatches),
          numStates(_numStates) {}  // used in test
    ~DFA() {}

    inline TransTable& getTransTable() { return transTable; }
    inline SubMatches& getSubMatches() { return subMatches; }
    inline StateSet& getAcceptStates() { return acceptStates; }
    inline int getNumStates() { return numStates; }
};

class DFAGenerator {
   private:
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

    void setEpsilonTable(Transition* trans, int transSize, int stateSize);
    void setPowsetTable(Transition* trans, int transSize);
    void initPowsetStates(int initState);
    void minimize();

   public:
    DFAGenerator(NFA* _nfa) : nfa(_nfa) {
        int stateSize = _nfa->stateSize;

        epsilonTable = std::vector<std::vector<int>>(stateSize);
        powsetTransTable = std::vector<std::vector<std::set<int>>>(
            stateSize, std::vector<std::set<int>>(ASCII_SZ));
    }
    ~DFAGenerator() { delete dfa; }
    void generate();
    DFA* getDFA() { return dfa; }
};
}  // namespace vlex
