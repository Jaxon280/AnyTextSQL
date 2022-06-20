#pragma once

#include "common.hpp"
#include "nfa.hpp"

namespace vlex {
struct SubMatch {
    ST_TYPE start;
    ST_TYPE end;
    std::string name;
    SubMatch(ST_TYPE _start, ST_TYPE _end, std::string _name)
        : start(_start), end(_end), name(_name) {}
};

class DFA {
   private:
    std::vector<std::vector<ST_TYPE>> transTable;
    std::set<ST_TYPE> acceptStates;
    int numStates;

    // inherit from NFA
    std::vector<State> states;
    State initState;
    State acceptState;
    std::vector<SubMatch> subMatches;
    std::vector<Transition> transVec;

   public:
    DFA(std::vector<std::vector<ST_TYPE>> _transTable,
        std::vector<State> _states, State _acceptState,
        std::set<ST_TYPE> _acceptStates, std::vector<SubMatch> _subMatches,
        int _numStates)
        : transTable(_transTable),
          states(_states),
          acceptStates(_acceptStates),
          initState(INIT_STATE),
          subMatches(_subMatches),
          numStates(_numStates) {}

    DFA(std::vector<std::vector<ST_TYPE>> _transTable,
        std::set<ST_TYPE> _acceptStates, int _numStates)
        : transTable(_transTable),
          acceptStates(_acceptStates),
          numStates(_numStates) {}

    inline std::vector<std::vector<ST_TYPE>> getTransTable() {
        return transTable;
    }
    inline std::set<ST_TYPE> getAcceptStates() { return acceptStates; }
    inline int getNumStates() { return numStates; }
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
    std::vector<SubMatch> subMatches;
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
