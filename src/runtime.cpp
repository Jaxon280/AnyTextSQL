#include "runtime.hpp"

namespace vlex {

Runtime::Runtime(const Table& _table)
    : executor(new Executor()),
      ios(new ioStream(table.getFilename())),
      dfag(new DFAGenerator()),
      table(_table) {
    size = ios->getSize();
    makePartitions(size);
}

// Runtime::Runtime(const Table& table, NFA** keyNFAs, QueryContext* _query)
//     : executor(new Executor()),
//       ios(new ioStream(table.getFilename())),
//       query(_query),
//       dfag(new DFAGenerator(nfa)) {}

void Runtime::constructDFA(const NFA* nfa) {
    dfag->initialize();
    dfa = dfag->generate(nfa, table.getKeyMap());
}

void Runtime::constructDFA(const NFA** keyNFAs) {
    // int keySize = table.getKeySize();
    // DFA** keyDFAs = new DFA*[keySize];
    // for (int ki = 0; ki < keySize; ki++) {
    //     dfag->initialize();
    //     keyDFAs[ki] = dfag->generate(keyNFAs[ki], table.getKeyMap());
    // }
    // dfa = mergeDFAs(keyDFAs, keySize);
}

void Runtime::constructVFA(double lr) {
    // now only for VFA
#if (defined BENCH)
    double ex_time = 0.0;
    timeval lex1, lex2;
    gettimeofday(&lex1, NULL);
#endif
    vfa = new VectFA(*dfa);

    SIZE_TYPE lsize = (SIZE_TYPE)size * lr;
    ios->readFile(0, lsize);
    vfa->constructVFA(*dfa, ios->getData(), lsize);
    executor->setVFA(vfa, 0);
    ios->seek(0);

#if (defined BENCH)
    gettimeofday(&lex2, NULL);
    ex_time =
        (lex2.tv_sec - lex1.tv_sec) + (lex2.tv_usec - lex1.tv_usec) * 0.000001;
    printf("#BENCH_VFA_construction: %lf s\n\n", ex_time);
#endif
}

void Runtime::iexec(QueryContext* query) {
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

void Runtime::exec(QueryContext* query) {
    executor->setQuery(query);
    ios->readFile(0, size);
    data = ios->getData();
    executor->exec(data, size);
}

void Runtime::makePartitions(SIZE_TYPE size) {
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

}  // namespace vlex
