#include "runtime.hpp"

using namespace vlex;

Runtime::Runtime(Table& table, NFA* nfa, QueryContext* _query)
    : executor(new Executor()),
      ios(new ioStream(table.getFilename())),
      query(_query) {
    dfag = new DFAGenerator(nfa);
    dfag->generate(table.getKeyMap());
    vfa = new VectFA(*dfag->getDFA());
    size = ios->getSize();
    makePartitions(size);
}

void Runtime::construct(double lr) {
    // now only for VFA
#if (defined BENCH)
    double ex_time = 0.0;
    timeval lex1, lex2;
    gettimeofday(&lex1, NULL);
#endif

    SIZE_TYPE lsize = (SIZE_TYPE)size * lr;
    ios->readFile(0, lsize);
    vfa->constructVFA(*dfag->getDFA(), ios->getData(), lsize);
    executor->setVFA(vfa, 0);
    executor->setQuery(query);
    ios->seek(0);

#if (defined BENCH)
    gettimeofday(&lex2, NULL);
    ex_time =
        (lex2.tv_sec - lex1.tv_sec) + (lex2.tv_usec - lex1.tv_usec) * 0.000001;
    printf("#BENCH_VFA_construction: %lf s\n\n", ex_time);
#endif
}

void Runtime::iexec() {
    // interleave
    int i = 0;

    while (i < partitions.size() - 1) {
        SIZE_TYPE off = partitions[i], next_off = partitions[i + 1];
        i++;
        SIZE_TYPE rsize = next_off - off;
        ios->readFile(off, rsize);
        data = ios->getData();
        executor->exec(data, rsize);
    }
}

void Runtime::exec() {
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
