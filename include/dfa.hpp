#pragma once

#include "common.hpp"
#include "nfa.hpp"

namespace vlex {
class DFA {
   public:
    struct SubMatchStates {
        ST_TYPE startState;
        bool isAnyStart;
        ST_TYPE endState;
        bool isAnyEnd;

        SubMatchStates(ST_TYPE _startState, bool _isAnyStart, ST_TYPE _endState,
                       bool _isAnyEnd)
            : startState(_startState),
              isAnyStart(_isAnyStart),
              endState(_endState),
              isAnyEnd(_isAnyEnd) {}
    };

   private:
    std::vector<std::vector<ST_TYPE>> transTable;
    std::set<ST_TYPE> acceptStates;
    int numStates;

    // inherit from NFA
    std::vector<State> states;
    State initState;
    State acceptState;
    std::vector<SubMatchStates> subMatches;
    std::vector<Transition> transVec;

   public:
    DFA(std::vector<std::vector<ST_TYPE>> _transTable,
        std::vector<State> _states, State _acceptState,
        std::set<ST_TYPE> _acceptStates, int _numStates)
        : transTable(_transTable),
          states(_states),
          acceptStates(_acceptStates),
          initState(INIT_STATE),
          numStates(_numStates) {}

    DFA(std::vector<std::vector<ST_TYPE>> _transTable,
        std::set<ST_TYPE> _acceptStates,
        std::vector<SubMatchStates> _subMatches, int _numStates)
        : transTable(_transTable),
          acceptStates(_acceptStates),
          subMatches(_subMatches),
          numStates(_numStates) {}  // used in test

    inline std::vector<std::vector<ST_TYPE>> getTransTable() {
        return transTable;
    }
    inline std::set<ST_TYPE> getAcceptStates() { return acceptStates; }
    inline int getNumStates() { return numStates; }
    inline std::vector<SubMatchStates>& getSubMatchStates() {
        return subMatches;
    }
    void set_nfa(std::vector<State> _states, State _initState,
                 State _acceptState, std::vector<Transition> _transVec) {
        states = _states;
        initState = _initState;
        acceptState = _acceptState;
        transVec = _transVec;
    }
};

class DFAGenerator {
   private:
    std::map<ST_TYPE, std::vector<ST_TYPE>> epsilonTable;
    // std::map<ST_TYPE, std::vector<Transition>> NFATransMap;
    std::map<uint8_t, std::vector<Transition>> NFATransMap;
    std::stack<std::vector<ST_TYPE>> stateStack;

   public:
    // DFAGenerator() {}
    // DFA construct_dfa(NFA& nfa) {
    //     for (Transition& t : nfa.get_transVec()) {
    //         if (t.get_c() == EPSILON) {
    //             epsilonTable[t.get_start().get_id()].push_back(
    //                 t.get_end().get_id());
    //         } else {
    //             // NFATransMap[t.get_start().get_id()].push_back(t);
    //             NFATransMap[(uint8_t)t.get_c()].push_back(t);
    //         }
    //     }

    //     std::set<ST_TYPE>
    //     initState(epsilonTable[nfa.get_initState().get_id()]);
    //     stateStack.push(initState);
    //     while (!stateStack.empty()) {
    //         std::set<ST_TYPE> state = stateStack.pop();
    //         for (uint8_t c = 0; c < 127; c++) {
    //             std::set<ST_TYPE> new_state;
    //             for (Transition& t : NFATransMap[c]) {
    //                 if (state.find(t.get_start().get_id()) != state.end()) {
    //                     new_state.insert(t.get_end().get_id());
    //                 }
    //             }

    //             if (new_state.size() > 0) {
    //             }
    //         }
    //     }
    // }
};
}  // namespace vlex
