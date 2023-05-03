#pragma once

#include "common.hpp"

namespace vlex {
struct Delta {
    ST_TYPE startState;
    std::string str;
    std::string backStr;  // for delta_ord
    std::vector<ST_TYPE> charTable;// for delta_{any, c}
    std::vector<ST_TYPE> rTable; // for delta_ord
    // int count; // for *_MASK
};

typedef enum _simd_kind { ORDERED, ANY, RANGES, CMPEQ, C, INV } SIMDKind;

struct Qlabel {
    SIMDKind kind;
    Delta *delta;

    bool isAccept = false;
};
}  // namespace vlex
