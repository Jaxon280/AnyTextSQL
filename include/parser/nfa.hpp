#pragma once

#include "types.hpp"

namespace vlex {
#define EPSILON -1
#define ASCII_SZ 128

typedef struct submatch {
    int start;
    int end;
    bool isAnyStart;
    bool isAnyEnd;
    char *name;
    struct submatch *next;
    PatternType type;
} SubMatch;  // linked list

struct Transition {
    int start;
    int end;
    int c;  // 0x00-0xff and epsilon (-1)
};

struct NFA {
    SubMatch *subms;
    Transition *transVec;
    int transSize;
    int initState;
    int acceptState;
    int stateSize;
    bool isAnyStart;
    bool isAnyEnd;
    PatternType type;
};
}  // namespace vlex
