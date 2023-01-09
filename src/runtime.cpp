#include "runtime.hpp"

namespace vlex {

RuntimeBase::RuntimeBase(const Table& _table)
    : executor(new Executor()),
      ios(new ioStream(_table.getFilename())),
      dfag(new DFAGenerator()),
      dfam(new DFAMerger()),
      table(_table) {
    size = ios->getSize();
    makePartitions(size);
}

void RuntimeBase::constructVFA(double lr) {
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

void RuntimeBase::iexec(QueryContext* query) {
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

void RuntimeBase::exec(QueryContext* query) {
    executor->setQuery(query);
    ios->readFile(0, size);
    data = ios->getData();
    executor->exec(data, size);
}

void RuntimeBase::execWithSpark(QueryContext *ctx, SparkContext *sctx) {
    executor->setSparkContext(sctx);
    ios->readFile(0, size);
    data = ios->getData();
    sctx->count = executor->execWithSpark(data, size);
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

void RuntimeExpression::constructDFA(const NFA* nfa, const NFA* regexNFA) {
    dfag->initialize();
    DFA* stickyDFA = dfag->generate(nfa, table.getKeyMap());
    dfag->initialize();
    DFA* regexDFA = dfag->generate(regexNFA, table.getKeyMap());
    dfam->initialize();
    dfa = dfam->merge(regexDFA, stickyDFA);
}

void RuntimeKeys::constructDFA(NFA** keyNFAs, NFA** keyRegexNFAs) {
    int keySize = table.getKeySize();
    DFA** keyDFAs = new DFA*[keySize];
    for (int i = 0; i < keySize; i++) {
        dfag->initialize();
        DFA* sDFA = dfag->generate(keyNFAs[i], table.getKeyMap());
        dfag->initialize();
        DFA* rDFA = dfag->generate(keyRegexNFAs[i], table.getKeyMap());
        dfam->initialize();
        keyDFAs[i] = dfam->merge(rDFA, sDFA);
    }

    mergeKeys(keyDFAs, keySize);
}

std::vector<std::map<int, int>> RuntimeKeys::createStateMaps(DFA** keyDFAs,
                                                             int keySize) {
    std::vector<std::map<int, int>> keyMaps(keySize);
    int n = 0;
    for (int i = 0; i < keySize; i++) {
        for (int s = 1; s < keyDFAs[i]->getNumStates(); s++) {
            keyMaps[i].insert(std::pair<int, int>(s, n + s));
        }
        n += keyDFAs[i]->getNumStates() - 1;
    }
    return keyMaps;
}

void RuntimeKeys::mergeKeys(DFA** keyDFAs, int keySize) {
    std::vector<std::map<int, int>> stateMaps =
        createStateMaps(keyDFAs, keySize);
    int numStates = 1;  // includes inv state
    for (const std::map<int, int>& smap : stateMaps) {
        numStates += smap.size();
    }
    DFA::TransTable transTable(numStates);
    transTable[0] = std::vector<DFA_ST_TYPE>(ASCII_SZ);
    int nextInitState;
    for (int i = 0; i < keySize; i++) {
        const DFA::TransTable& tt = keyDFAs[i]->getTransTable();
        if (i < keySize - 1) {
            nextInitState = stateMaps[i + 1][keyDFAs[i + 1]->getInitState()];
        } else {
            nextInitState = INV_STATE;
        }

        for (int s = 1; s < keyDFAs[i]->getNumStates(); s++) {
            int ss = stateMaps[i][s];
            transTable[ss].resize(ASCII_SZ);
            for (int c = 0; c < ASCII_SZ; c++) {
                if (tt[s][c] == INV_STATE) {
                    transTable[ss][c] = nextInitState;
                } else {
                    transTable[ss][c] = stateMaps[i][tt[s][c]];
                }
            }
        }
    }

    DFA::StateSet acceptStates;
    for (DFA_ST_TYPE s : keyDFAs[keySize - 1]->getAcceptStates()) {
        acceptStates.insert(stateMaps[keySize - 1][s]);
    }

    DFA::SubMatches subMatches;
    for (int i = 0; i < keySize; i++) {
        for (const DFA::SubMatchStates& sms : keyDFAs[i]->getSubMatches()) {
            DFA::SubMatchStates smss;
            smss.id = sms.id, smss.predUUID = sms.predUUID,
            smss.predIDs = sms.predIDs, smss.type = sms.type;
            for (int s : sms.startStates) {
                smss.startStates.insert(stateMaps[i][s]);
            }
            for (int s : sms.endStates) {
                smss.endStates.insert(stateMaps[i][s]);
            }
            subMatches.push_back(smss);
        }
    }

    dfa = new DFA(transTable, acceptStates, subMatches, numStates);
    for (int i = 0; i < keySize; i++) {
        delete keyDFAs[i];
    }
}

}  // namespace vlex
