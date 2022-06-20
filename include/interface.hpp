#pragma once

#include "common.hpp"

struct Delta {
    ST_TYPE startState;
    std::string str;
    std::string backStr;  // for delta_ord
    std::vector<ST_TYPE> charTable;
    std::vector<ST_TYPE> rTable;
};

typedef enum _simd_kind { ORDERED, ANY, RANGES, CMPEQ, C, INV } SIMDKind;

struct Qlabel {
    SIMDKind kind;
    Delta *delta;

    bool isAccept = false;
};
