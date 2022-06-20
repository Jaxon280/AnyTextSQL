#pragma once

#include "common.hpp"
#include "converter.hpp"
#include "interface.hpp"

namespace vlex {
class Token {
    std::vector<DATA_TYPE> str;
    int tokenType = 0;

   public:
    void set_literals(DATA_TYPE *data, SIZE_TYPE start, SIZE_TYPE size) {
        DATA_TYPE buf[size + 1];
        buf[size] = (DATA_TYPE)0;
        memcpy(buf, &data[start], size);
        str = std::vector<DATA_TYPE>(buf, buf + size + 1);
    }
    std::vector<DATA_TYPE> get_literals() { return str; };
};

class Executor {
   private:
    struct Context {
        ST_TYPE recentAcceptState;
        ST_TYPE currentState;
        SIZE_TYPE recentAcceptIndex;
        SIZE_TYPE tokenStartIndex;
    };

    Executor::Context ctx;
    std::vector<Token> tokenVec;
    SIMD_TYPE *SIMDDatas;
    int *SIMDSizes;
    SIMDKind *kindTable;  // State -> Kind
    ST_TYPE *rTable;      // State -> State
    ST_TYPE **charTable;  // State * char -> State
    std::set<ST_TYPE> acceptStates;

    DATA_TYPE *data;
    SIZE_TYPE size;
    SIZE_TYPE i;

    inline void cmpestri_ord(ST_TYPE cur_state);
    inline void cmpestri_any(ST_TYPE cur_state);
    inline void cmpestri_ranges(ST_TYPE cur_state);
    void generateToken(std::vector<Token> &token_vec, ST_TYPE state,
                       DATA_TYPE *data, SIZE_TYPE start, SIZE_TYPE end);

   public:
    Executor();
    Executor(VectFA *vfa, SIZE_TYPE _start);
    void setVFA(VectFA *vfa, SIZE_TYPE _start);
    void exec(DATA_TYPE *_data, SIZE_TYPE size);
    inline std::vector<Token> getTokenVec() { return tokenVec; }
};
}  // namespace vlex
