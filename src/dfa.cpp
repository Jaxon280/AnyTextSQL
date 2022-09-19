#include "scanner/dfa.hpp"

using namespace vlex;

DFA::DFA(int _initState, std::vector<int>& _powsStates,
         std::map<int, int>& _old2new,
         std::map<int, std::vector<int>>& _powTTable,
         std::set<int>& _acceptStates, std::vector<SubMatchStates> _smses) {
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

    for (SubMatchStates& sms : _smses) {
        SubMatchStates new_sms = sms;
        new_sms.startState = old2new_comp[sms.startState];
        new_sms.endState = old2new_comp[sms.endState];
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

void DFAGenerator::initPowsetStates(int initState) {
    std::set<int> pset;
    powsetStates.insert(std::pair<int, std::set<int>>(initState, pset));
    powsetStates[initState].insert(nfa->initState);
    for (int es : epsilonTable[nfa->initState]) {
        powsetStates[initState].insert(es);
    }
    if (powsetStates[initState].count(nfa->acceptState) > 0) {
        acceptStates.insert(initState);
    }
}

void DFAGenerator::minimize(std::vector<int>& submatchStates) {
    std::stack<int> sstack;
    sstack.push(INIT_STATE);

    std::vector<int> visited(powsetStates.size() + 1, 0);
    stateVec.push_back(0);  // inv state
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
            for (int smss : submatchStates) {
                if (ps.find(smss) != ps.end()) {
                    goto distinguish;
                }
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

void DFAGenerator::generate(KeyMap* keyMap) {
    setEpsilonTable(nfa->transVec, nfa->transSize, nfa->stateSize);
    setPowsetTable(nfa->transVec, nfa->transSize);

    int initState = INIT_STATE;
    initPowsetStates(initState);

    stateStack.push(initState);
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

    std::vector<int> smsVec;
    for (SubMatch* s = nfa->subms; s != NULL; s = s->next) {
        smsVec.push_back(s->start);
        smsVec.push_back(s->end);
    }
    minimize(smsVec);

    for (SubMatch* s = nfa->subms; s != NULL; s = s->next) {
        DFA::SubMatchStates sms;
        sms.id = keyMap->at(s->name).id;
        sms.type = keyMap->at(s->name).type;
        sms.predID = s->predID;
        for (int ss : stateVec) {
            if (powsetStates[ss].count(s->start) > 0) {
                sms.startState = ss;
                sms.isAnyStart = s->isAnyStart;
                // todo: type and subm name
            }

            if (powsetStates[ss].count(s->end) > 0) {
                sms.endState = ss;
                sms.isAnyEnd = s->isAnyEnd;
            }
        }
        smses.push_back(sms);
    }

    dfa = new DFA(initState, stateVec, default2mini, transTable, acceptStates,
                  smses);
    // std::cout << "Num States: " << dfa->getNumStates() << std::endl;
    // std::vector<std::vector<DFA_ST_TYPE>>& ttable = dfa->getTransTable();
    // for (int ti = 1; ti < ttable.size(); ti++) {
    //     for (int tc = 0; tc < ASCII_SZ; tc++) {
    //         if (ttable[ti][tc] != 0) {
    //             std::cout << "cur state: " << ti << " -- " << (char)tc
    //                       << " --> " << (int)ttable[ti][tc] << std::endl;
    //         }
    //     }
    // }
    // for (DFA_ST_TYPE s : dfa->getAcceptStates()) {
    //     std::cout << "Accept States: " << (int)s << std::endl;
    // }
    // for (DFA::SubMatchStates& s : dfa->getSubMatches()) {
    //     std::cout << (int)s.startState << (int)s.isAnyStart << " "
    //               << (int)s.endState << (int)s.isAnyEnd << std::endl;
    // }
}
