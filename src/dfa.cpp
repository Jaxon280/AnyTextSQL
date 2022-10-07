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

DFA* DFAMerger::merge(const DFA* rDFA, const DFA* sDFA) {
    mapDFAStates(rDFA, sDFA);

    int ssize = rDFA->getNumStates();
    const DFA::StateSet& racceptStates = rDFA->getAcceptStates();
    const DFA::SubMatches& rsubms = rDFA->getSubMatches();

    createTransMap(rDFA, sDFA, ssize, racceptStates);

    int numStates = transMap.size();
    std::vector<int> old2new = createStateMap(ssize);
    DFA::TransTable transTable = createTransTable(numStates, old2new, ssize);
    DFA::StateSet acceptStates =
        createAcceptStates(racceptStates, old2new, ssize);
    DFA::SubMatches subMatches = createSubMatches(rsubms, old2new, ssize);

    DFA* dfa = new DFA(transTable, acceptStates, subMatches, numStates);
    delete rDFA;
    delete sDFA;
    return dfa;
}

void DFAMerger::createTransMap(const DFA* rDFA, const DFA* sDFA, int ssize,
                               const DFA::StateSet& racceptStates) {
    std::vector<int> visited(ssize * 2 + 1);
    const DFA::TransTable& rtrans = rDFA->getTransTable();
    const DFA::TransTable& strans = sDFA->getTransTable();
    DFA_ST_TYPE initState = rDFA->getInitState();

    std::stack<DFA_ST_TYPE> stateStack;
    if (racceptStates.find(initState) != racceptStates.end()) {
        stateStack.push(initState + ssize);
    } else {
        stateStack.push(initState);
    }
    stateStack.push(0);

    while (!stateStack.empty()) {
        DFA_ST_TYPE s = stateStack.top();
        stateStack.pop();

        std::vector<DFA_ST_TYPE> trans(ASCII_SZ);
        if (s > ssize) {
            int ss = s - ssize;
            for (int i = 0; i < ASCII_SZ; i++) {
                if (strans[rsMap[ss]][i] == 0) {
                    trans[i] = 0;
                } else {
                    trans[i] = srMap[strans[rsMap[ss]][i]] + ssize;
                }
                if (visited[trans[i]] == 0) {
                    stateStack.push(trans[i]);
                    visited[trans[i]] = 1;
                }
            }
        } else if (s == ssize) {
            continue;
        } else {
            for (int i = 0; i < ASCII_SZ; i++) {
                if (racceptStates.find(rtrans[s][i]) != racceptStates.end()) {
                    trans[i] = rtrans[s][i] + ssize;
                } else {
                    trans[i] = rtrans[s][i];
                }
                if (visited[trans[i]] == 0) {
                    stateStack.push(trans[i]);
                    visited[trans[i]] = 1;
                }
            }
        }
        transMap.insert(std::pair<int, std::vector<int>>(s, trans));
    }
}

DFA::StateSet DFAMerger::createAcceptStates(const DFA::StateSet& racceptStates,
                                            const std::vector<int>& old2new,
                                            int ssize) {
    DFA::StateSet states;
    for (auto it = transMap.cbegin(); it != transMap.cend(); ++it) {
        if (it->first >= ssize &&
            racceptStates.find(it->first - ssize) != racceptStates.end()) {
            states.insert(old2new[it->first]);
        }
    }
    return states;
}

DFA::SubMatches DFAMerger::createSubMatches(const DFA::SubMatches& rsubms,
                                            const std::vector<int>& old2new,
                                            int ssize) {
    DFA::SubMatches subms;
    for (const DFA::SubMatchStates& rsms : rsubms) {
        std::vector<DFA_ST_TYPE> startStates;
        std::vector<DFA_ST_TYPE> endStates;
        for (DFA_ST_TYPE ss : rsms.startStates) {
            if (old2new[ss] == 0) {
                if (old2new[ss + ssize] == 0) {
                } else {
                    startStates.push_back(old2new[ss + ssize]);
                }
            } else {
                startStates.push_back(old2new[ss]);
            }
        }
        for (DFA_ST_TYPE ss : rsms.endStates) {
            if (old2new[ss] == 0) {
                if (old2new[ss + ssize] == 0) {
                } else {
                    endStates.push_back(old2new[ss + ssize]);
                }
            } else {
                endStates.push_back(old2new[ss]);
            }
        }

        subms.push_back(DFA::SubMatchStates(rsms.id, rsms.type, rsms.predID,
                                            startStates, endStates));
    }
    return subms;
}

DFA::TransTable DFAMerger::createTransTable(int numStates,
                                            const std::vector<int>& old2new,
                                            int ssize) {
    DFA::TransTable transTable;

    for (auto it = transMap.cbegin(); it != transMap.cend(); ++it) {
        std::vector<int> trans;
        for (int s : it->second) {
            trans.push_back(old2new[s]);
        }
        transTable.push_back(trans);
    }

    return transTable;
}

std::vector<int> DFAMerger::createStateMap(int ssize) {
    std::vector<int> map(ssize * 2);
    map[INV_STATE] = INV_STATE, map[INIT_STATE] = INIT_STATE;
    int i = 2;
    for (auto it = transMap.cbegin(); it != transMap.cend(); ++it) {
        if (it->first != INV_STATE && it->first != INIT_STATE) {
            map[it->first] = i;
            i++;
        }
    }
    return map;
}

void DFAMerger::mapDFAStates(const DFA* rDFA, const DFA* sDFA) {
    const DFA::TransTable& rtrans = rDFA->getTransTable();
    const DFA::TransTable& strans = sDFA->getTransTable();

    std::vector<int> visited(rDFA->getNumStates());

    srMap.insert(
        std::pair<int, int>(sDFA->getInitState(), rDFA->getInitState()));
    std::stack<int> stateStack;
    stateStack.push(sDFA->getInitState());
    while (!stateStack.empty()) {
        int s = stateStack.top();
        int rs = srMap[s];
        stateStack.pop();
        for (int i = 0; i < ASCII_SZ; i++) {
            if (strans[s][i] != 0) {
                if (visited[rtrans[rs][i]] == 0) {
                    visited[rtrans[rs][i]] = 1;
                    stateStack.push(strans[s][i]);
                    srMap.insert(
                        std::pair<int, int>(strans[s][i], rtrans[rs][i]));
                }
            }
        }
    }

    for (const auto& [s, r] : srMap) {
        rsMap.insert(std::pair<int, int>(r, s));
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
