#pragma once

#include "common.hpp"
#include "executor/executor.hpp"
#include "general/ios.hpp"
#include "interface.hpp"
#include "parser/nfa.hpp"
#include "parser/query.hpp"
#include "scanner/dfa.hpp"
#include "scanner/vfa.hpp"

#define PARTITION_SIZE (SIZE_TYPE)(1 << 31)  // 1024MB

namespace vlex {
class Runtime {
   public:
    Runtime(const Table& table);
    void constructDFA(const NFA** keyNFAs);
    void constructDFA(const NFA* nfa);
    void constructVFA(double lr);
    void iexec(QueryContext* query);
    void exec(QueryContext* query);

   private:
    void makePartitions(SIZE_TYPE size);

    Executor* executor;
    ioStream* ios;
    DFAGenerator* dfag;
    DFA* dfa;
    VectFA* vfa;
    const Table& table;

    DATA_TYPE* data;
    SIZE_TYPE size;

    std::vector<SIZE_TYPE> partitions;
};
}  // namespace vlex
