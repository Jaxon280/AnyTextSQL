#pragma once

#include "common.hpp"
#include "converter.hpp"
#include "dfa.hpp"
#include "executor.hpp"
#include "ioStream.hpp"

#define PARTITION_SIZE (SIZE_TYPE)(1 << 30)  // 1024MB

namespace vlex {
class Vlex {
   public:
    Vlex(std::string& filename, DFA& _dfa);
    void construct(double lr);
    void iexec();
    void exec();

    inline void printTokens() {
        for (int i = 0; i < executor->getTokenVec().size(); i++) {
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
    Executor* executor;
    ioStream* ios;

    DATA_TYPE* data;
    SIZE_TYPE size;

    std::vector<SIZE_TYPE> partitions;

    void make_partitions(SIZE_TYPE size);
};
}  // namespace vlex
