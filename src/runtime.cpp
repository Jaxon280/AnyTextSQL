#include "runtime.hpp"

namespace vlex {

RuntimeBase::RuntimeBase(const Table& _table)
    : executor(new Executor()),
      ios(new ioStream(_table.getFilename())),
      dfag(new DFAGenerator()),
      table(_table) {
    size = ios->getSize();
    makePartitions(size);
}

void RuntimeBase::makePartitions(SIZE_TYPE size) {
    SIZE_TYPE psize = size / PARTITION_SIZE;
    SIZE_TYPE i = 0;
    SIZE_TYPE offset = 0;
    while (i < psize) {
        partitions.push_back(offset);
        if (i >= psize) {
            offset += size - offset;
        } else {
            offset += PARTITION_SIZE;
        }
        i++;
    }
    partitions.push_back(size - 1);
}

DFA* RuntimeBase::mergeDFAs(const DFA* rDFA, const DFA* sDFA) {
    int ssize = rDFA->getNumStates();
    std::vector<int> visited(ssize * 2 + 1);
    const DFA::TransTable& rtrans = rDFA->getTransTable();
    const DFA::TransTable& strans = sDFA->getTransTable();
    const DFA::StateSet& racceptStates = rDFA->getAcceptStates();
    const DFA::SubMatches& rsubms = rDFA->getSubMatches();
    DFA_ST_TYPE initState = rDFA->getInitState();

    std::stack<DFA_ST_TYPE> stateStack;
    std::map<int, std::vector<int>> transMap;
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
                if (strans[ss][i] == 0) {
                    trans[i] = 0;
                } else {
                    trans[i] = strans[ss][i] + ssize;
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

    int numStates = transMap.size();
    std::vector<int> old2new = createStateMap(transMap, ssize);
    DFA::TransTable transTable =
        createTransTable(transMap, numStates, old2new, ssize);
    DFA::StateSet acceptStates =
        createAcceptStates(transMap, racceptStates, old2new, ssize);
    DFA::SubMatches subMatches = createSubMatches(rsubms, old2new, ssize);

    DFA* dfa = new DFA(transTable, acceptStates, subMatches, numStates);
    return dfa;
}

DFA::SubMatches RuntimeBase::createSubMatches(const DFA::SubMatches& rsubms,
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
                if (old2new[ss + ssize] != 0) {
                    endStates.push_back(old2new[ss + ssize]);
                } else {
                    endStates.push_back(old2new[ss]);
                }
            }
        }

        subms.push_back(DFA::SubMatchStates(rsms.id, rsms.type, rsms.predID,
                                            startStates, endStates));
    }
    return subms;
}

DFA::StateSet RuntimeBase::createAcceptStates(
    const std::map<int, std::vector<int>>& transMap,
    const DFA::StateSet& racceptStates, const std::vector<int>& old2new,
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

DFA::TransTable RuntimeBase::createTransTable(
    const std::map<int, std::vector<int>>& transMap, int numStates,
    const std::vector<int>& old2new, int ssize) {
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

std::vector<int> RuntimeBase::createStateMap(
    const std::map<int, std::vector<int>>& transMap, int ssize) {
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

void RuntimeExpression::constructDFA(const NFA* nfa, const NFA* regexNFA) {
    dfag->initialize();
    DFA* stickyDFA = dfag->generate(nfa, table.getKeyMap());
    dfag->initialize();
    DFA* regexDFA = dfag->generate(regexNFA, table.getKeyMap());
    dfa = mergeDFAs(regexDFA, stickyDFA);
}

void RuntimeExpression::constructVFA(double lr) {
#if (defined BENCH)
    double ex_time = 0.0;
    timeval lex1, lex2;
    gettimeofday(&lex1, NULL);
#endif
    vfa = new VectFA(*dfa);  // todo: VFA Generator

    SIZE_TYPE lsize = (SIZE_TYPE)size * lr;
    ios->readFile(0, lsize);
    vfa->constructVFA(*dfa, ios->getData(), lsize);
    executor->setFA(vfa, 0);
    ios->seek(0);

#if (defined BENCH)
    gettimeofday(&lex2, NULL);
    ex_time =
        (lex2.tv_sec - lex1.tv_sec) + (lex2.tv_usec - lex1.tv_usec) * 0.000001;
    printf("#BENCH_VFA_construction: %lf s\n\n", ex_time);
#endif
}

void RuntimeExpression::iexec(QueryContext* query) {
    // interleave
    executor->setQuery(query);

    int i = 0;
    while (i < (int)partitions.size() - 1) {
        SIZE_TYPE off = partitions[i], next_off = partitions[i + 1];
        i++;
        SIZE_TYPE rsize = next_off - off;
        ios->readFile(off, rsize);
        data = ios->getData();
        executor->exec(data, rsize);
    }
}

void RuntimeExpression::exec(QueryContext* query) {
    executor->setQuery(query);
    ios->readFile(0, size);
    data = ios->getData();
    executor->exec(data, size);
}

void RuntimeKeys::constructDFAs(const NFA** keyNFAs, const NFA** keyRegexNFAs) {
    // int keySize = table.getKeySize();
    // DFA** keyDFAs = new DFA*[keySize];
    // for (int ki = 0; ki < keySize; ki++) {
    //     dfag->initialize();
    //     keyDFAs[ki] = dfag->generate(keyNFAs[ki], table.getKeyMap());
    // }
    // dfa = mergeDFAs(keyDFAs, keySize);
}

void RuntimeKeys::constructVFAs(double lr) {
    // #if (defined BENCH)
    //     double ex_time = 0.0;
    //     timeval lex1, lex2;
    //     gettimeofday(&lex1, NULL);
    // #endif
    //     vfa = new VectFA(*dfa);

    //     SIZE_TYPE lsize = (SIZE_TYPE)size * lr;
    //     ios->readFile(0, lsize);
    //     vfa->constructVFA(*dfa, ios->getData(), lsize);
    //     executor->setVFA(vfa, 0);
    //     ios->seek(0);

    // #if (defined BENCH)
    //     gettimeofday(&lex2, NULL);
    //     ex_time =
    //         (lex2.tv_sec - lex1.tv_sec) + (lex2.tv_usec - lex1.tv_usec) *
    //         0.000001;
    //     printf("#BENCH_VFA_construction: %lf s\n\n", ex_time);
    // #endif
}

void RuntimeKeys::iexec(QueryContext* query) {
    // interleave
    // executor->setQuery(query);

    // int i = 0;
    // while (i < (int)partitions.size() - 1) {
    //     SIZE_TYPE off = partitions[i], next_off = partitions[i + 1];
    //     i++;
    //     SIZE_TYPE rsize = next_off - off;
    //     ios->readFile(off, rsize);
    //     data = ios->getData();
    //     executor->exec(data, rsize);
    // }
}

void RuntimeKeys::exec(QueryContext* query) {
    // executor->setQuery(query);
    // ios->readFile(0, size);
    // data = ios->getData();
    // executor->exec(data, size);
}

}  // namespace vlex
