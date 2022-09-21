#pragma once

#include "common.hpp"
#include "executor/executor.hpp"
#include "general/ios.hpp"
#include "interface.hpp"
#include "parser/nfa.hpp"
#include "parser/query.hpp"
#include "scanner/converter.hpp"
#include "scanner/dfa.hpp"

#define PARTITION_SIZE (SIZE_TYPE)(1 << 31)  // 1024MB

namespace vlex {
class Runtime {
   public:
    Runtime(const Table& table, NFA* nfa, QueryContext* _query);
    void construct(double lr);
    void iexec();
    void exec();

   private:
    void makePartitions(SIZE_TYPE size);

    Executor* executor;
    ioStream* ios;
    QueryContext* query;
    DFAGenerator* dfag;
    VectFA* vfa;

    DATA_TYPE* data;
    SIZE_TYPE size;

    std::vector<SIZE_TYPE> partitions;
};
}  // namespace vlex
