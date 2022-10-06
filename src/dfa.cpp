#include "scanner/dfa.hpp"

namespace vlex {

DFA::DFA(int _initState, const std::vector<int>& _powsStates,
         const std::map<int, int>& _old2new,
         std::map<int, std::vector<int>>& _powTTable,
         const std::set<int>& _acceptStates,
         const std::vector<SubMatchStates>& _smses) {
    std::map<int, int> old2new_comp;
    numStates = 0;
    for (const int& s : _powsStates) {
        old2new_comp[s] = numStates;
        numStates++;
    }

    for (const auto& [os, ns] : _old2new) {
        old2new_comp[os] = old2new_comp[ns];
    }

    initState = _initState;
    for (const int& ps : _powsStates) {
        if (_acceptStates.count(ps) > 0) {
            acceptStates.insert(old2new_comp[ps]);
        }
    }

    transTable = std::vector(numStates, std::vector<DFA_ST_TYPE>(ASCII_SZ));
    int j = 0;
    for (const int& ps : _powsStates) {
        for (int i = 0; i < ASCII_SZ; i++) {
            if (ps == INV_STATE) {
                transTable[j][i] = INV_STATE;
            } else {
                transTable[j][i] = old2new_comp[_powTTable[ps][i]];
            }
        }
        j++;
    }

    for (const SubMatchStates& sms : _smses) {
        SubMatchStates new_sms;
        new_sms.id = sms.id;
        new_sms.predID = sms.predID;
        new_sms.type = sms.type;
        for (int ss : sms.startStates) {
            new_sms.startStates.push_back(old2new_comp[ss]);
        }
        for (int es : sms.endStates) {
            new_sms.endStates.push_back(old2new_comp[es]);
        }
        subMatches.push_back(new_sms);
    }
}

void DFAGenerator::setEpsilonTable(Transition* trans, int transSize,
                                   int stateSize) {
    std::vector<std::vector<int>> etTable(stateSize);
    for (int ti = 0; ti < transSize; ti++) {
        if (trans[ti].c == EPSILON) {
            etTable[trans[ti].start].push_back(trans[ti].end);
        }
    }

    for (int s = 0; s < stateSize; s++) {
        std::stack<int> sstack;
        std::vector<int> visited(stateSize, 0);

        sstack.push(s);
        while (!sstack.empty()) {
            int v = sstack.top();
            sstack.pop();

            if (visited[v] == 0) {
                visited[v] = 1;
                for (int ve : etTable[v]) {
                    if (visited[ve] == 0) {
                        sstack.push(ve);
                        epsilonTable[s].push_back(ve);
                    }
                }
            }
        }
    }
}

void DFAGenerator::setPowsetTable(Transition* trans, int transSize) {
    for (int ti = 0; ti < transSize; ti++) {
        int s = trans[ti].start;
        int e = trans[ti].end;
        int c = trans[ti].c;

        if (c == EPSILON) continue;
        powsetTransTable[s][c].insert(e);

        if (epsilonTable[e].size() > 0) {
            for (int es : epsilonTable[e]) {
                if (powsetTransTable[s][c].count(es) == 0) {
                    powsetTransTable[s][c].insert(es);
                }
            }
        }
    }
}

void DFAGenerator::initPowsetStates(const NFA* nfa) {
    std::set<int> pset;
    powsetStates.insert(std::pair<int, std::set<int>>(INIT_STATE, pset));
    powsetStates[INIT_STATE].insert(nfa->initState);
    for (int es : epsilonTable[nfa->initState]) {
        powsetStates[INIT_STATE].insert(es);
    }
    if (powsetStates[INIT_STATE].count(nfa->acceptState) > 0) {
        acceptStates.insert(INIT_STATE);
    }
}

void DFAGenerator::minimize() {
    std::stack<int> sstack;
    sstack.push(INIT_STATE);

    std::vector<int> visited(powsetStates.size() + 1, 0);
    stateVec.push_back(0);  // inv state
    default2mini[0] = 0;
    while (!sstack.empty()) {
        int s = sstack.top();
        sstack.pop();

        if (visited[s] == 1) {
            continue;
        } else {
            visited[s] = 1;
            default2mini[s] = s;
            stateVec.push_back(s);
        }

        for (const auto& [sid, ps] : powsetStates) {
            if (sid == s) continue;
            if ((acceptStates.count(s) > 0) ^ (acceptStates.count(sid) > 0)) {
                goto distinguish;
            }
            for (int i = 0; i < ASCII_SZ; i++) {
                if (transTable[s][i] != transTable[sid][i]) {
                    goto distinguish;
                }
            }

            visited[sid] = 1;
            for (int pss : ps) {
                powsetStates[s].insert(pss);
            }
            default2mini[sid] = s;
            continue;  // merge

        distinguish:
            sstack.push(sid);
        }
    }
}

void DFAGenerator::setInvStates() {
    for (auto it = transTable.begin(); it != transTable.end(); ++it) {
        if (acceptStates.find(it->first) != acceptStates.end()) {
            for (int i = 0; i < (int)it->second.size(); i++) {
                if (default2mini[it->second[i]] == INIT_STATE) {
                    it->second[i] = INV_STATE;
                }
            }
        }
    }
}

void DFAGenerator::initialize() {
    powsetStates = std::map<int, std::set<int>>();
    transTable = std::map<int, std::vector<int>>();
    default2mini = std::map<int, int>();
    acceptStates = std::set<int>();
    stateVec = std::vector<int>();
}

DFA* DFAGenerator::generate(const NFA* nfa, const KeyMap& keyMap) {
    int stateSize = nfa->stateSize;

    epsilonTable = std::vector<std::vector<int>>(stateSize);
    powsetTransTable = std::vector<std::vector<std::set<int>>>(
        stateSize, std::vector<std::set<int>>(ASCII_SZ));
    setEpsilonTable(nfa->transVec, nfa->transSize, nfa->stateSize);
    setPowsetTable(nfa->transVec, nfa->transSize);

    initPowsetStates(nfa);

    std::stack<int> stateStack;
    stateStack.push(INIT_STATE);
    int stateID = 2;
    while (!stateStack.empty()) {
        int s = stateStack.top();
        stateStack.pop();

        if (transTable.count(s) == 0) {
            transTable[s] = std::vector<int>(ASCII_SZ, INV_STATE);
        }

        int sc = 0;
        int new_s;

        while (sc < ASCII_SZ) {
            std::set<int> sset;
            for (int ss : powsetStates[s]) {
                for (int se : powsetTransTable[ss][sc]) {
                    sset.insert(se);
                }
            }

            if (sset.size() == 0) {
                transTable[s][sc] = INV_STATE;
                goto loop_end;
            }

            for (const auto& [sid, ps] : powsetStates) {
                if (sset == ps) {
                    transTable[s][sc] = sid;
                    goto loop_end;
                }
            }
            new_s = stateID;
            stateID++;
            powsetStates.insert(std::pair<int, std::set<int>>(new_s, sset));
            if (sset.count(nfa->acceptState) > 0) {
                acceptStates.insert(new_s);
            }
            transTable[s][sc] = new_s;
            stateStack.push(new_s);

        loop_end:
            sc++;
        }
    }

    minimize();
    setInvStates();

    std::vector<DFA::SubMatchStates> smses;
    for (SubMatch* s = nfa->subms; s != NULL; s = s->next) {
        DFA::SubMatchStates sms;
        sms.id = keyMap.at(s->name).id;
        sms.type = keyMap.at(s->name).type;
        sms.predID = s->predID;
        for (const auto& [sid, ps] : powsetStates) {
            if (ps.count(s->start) > 0) {
                sms.startStates.push_back(sid);
            }

            if (ps.count(s->end) > 0) {
                sms.endStates.push_back(sid);
            }
        }
        smses.push_back(sms);
    }

    DFA* dfa = new DFA(INIT_STATE, stateVec, default2mini, transTable,
                       acceptStates, smses);
    return dfa;
}

}  // namespace vlex
