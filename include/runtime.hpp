#pragma once

#include "common.hpp"
#include "executor/executor.hpp"
#include "general/ioStream.hpp"
#include "interface.hpp"
#include "parser/nfa.hpp"
#include "parser/query.hpp"
#include "scanner/converter.hpp"
#include "scanner/dfa.hpp"

#define PARTITION_SIZE (SIZE_TYPE)(1 << 31)  // 1024MB

namespace vlex {
class Runtime {
   public:
    Runtime(Table& table, NFA* nfa, QueryContext* _query);
    void construct(double lr);
    void iexec();
    void exec();

   private:
    DFAGenerator* dfag;
    VectFA* vfa;
    QueryContext* query;
    Executor* executor;
    ioStream* ios;

    DATA_TYPE* data;
    SIZE_TYPE size;

    std::vector<SIZE_TYPE> partitions;

    void makePartitions(SIZE_TYPE size);
};
}  // namespace vlex
