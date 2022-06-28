#pragma once

#include "common.hpp"
#include "converter.hpp"
#include "dfa.hpp"
#include "executor.hpp"
#include "ioStream.hpp"
#include "query.hpp"

#define PARTITION_SIZE (SIZE_TYPE)(1 << 30)  // 1024MB

namespace vlex {
class Vlex {
   public:
    Vlex(std::string& filename, DFA& _dfa, QueryContext* _query);
    void construct(double lr);
    void iexec();
    void exec();

    inline void printTokens(int size) {
        int psize;
        if (executor->getTokenVec().size() < size) {
            psize = executor->getTokenVec().size();
        } else {
            psize = size;
        }
        for (int i = 0; i < psize; i++) {
            std::cout << "Token [" << i + 1 << "/" << psize << "]" << std::endl;
            std::string s;
            for (DATA_TYPE d : executor->getTokenVec()[i].get_literals()) {
                s.push_back(d);
            }
            std::cout << s << std::endl;
        }
    }

    inline void printCount() {
        std::cout << executor->getTokenVec().size() << std::endl;
    }

   private:
    DFA& dfa;

    VectFA* vfa;
    QueryContext* query;
    Executor* executor;
    ioStream* ios;

    DATA_TYPE* data;
    SIZE_TYPE size;

    std::vector<SIZE_TYPE> partitions;

    void make_partitions(SIZE_TYPE size);
};
}  // namespace vlex
