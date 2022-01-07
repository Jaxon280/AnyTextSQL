#pragma once

#include "common.hpp"

struct Delta {
    ST_TYPE startState;
    std::string str;
    std::vector<ST_TYPE> char_table;
    std::vector<ST_TYPE> r_table;
    // SIMD_KIND inst;
};

typedef enum _simd_kind { ORDERED, ANY, RANGES, CMPEQ, C, INV } SIMDKind;

struct Qlabel {
    SIMDKind kind;
    ST_TYPE state;
    Delta *delta;

    int c_length = 0;
    bool is_sink = false;  // "i += r + |x|"
    bool is_inc = false;   // "i++"
    bool is_r = false;     // "i += r"

    bool is_accept = false;
};
