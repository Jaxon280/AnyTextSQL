#pragma once

#include "common.hpp"

struct Delta {
    ST_TYPE startState;
    std::string str;
    std::vector<ST_TYPE> char_table;
    // std::map<char, ST_TYPE> char_table;
    std::vector<ST_TYPE> r_table;
    // SIMD_KIND inst;
};

typedef enum _simd_kind { ORDERED, ANY, RANGES, CMPEQ, C, INV } SIMDKind;

struct Qlabel {
    SIMDKind kind;
    ST_TYPE state;
    Delta *delta;

    bool is_accept = false;
};
