#pragma once

#include <string>

#include "types.hpp"

namespace vlex {
#define EPSILON -1
#define ASCII_SZ 128

struct SubMatch {
    int start;
    int end;
    int predID;
    const char *name;
    PatternType type;
    std::string regex;
    SubMatch *next;
    SubMatch() {}
    SubMatch(int start, int end, int predID, const char *name, PatternType type,
             std::string regex)
        : start(start),
          end(end),
          predID(predID),
          name(name),
          type(type),
          regex(regex) {}
    SubMatch(SubMatch *sms)
        : start(sms->start),
          end(sms->end),
          predID(sms->predID),
          name(sms->name),
          type(sms->type),
          regex(sms->regex) {}
};  // linked list

struct Transition {
    int start;
    int end;
    int c;  // 0x00-0xff and epsilon (-1)
    Transition() {}
    Transition(int start, int end, int c) : start(start), end(end), c(c) {}
};

struct NFA {
    SubMatch *subms;
    Transition *transVec;
    int transSize;
    int initState;
    int acceptState;
    int stateSize;
    PatternType type;
    std::string regex;
    NFA() {}
    NFA(SubMatch *subms, Transition *transVec, int transSize, int initState,
        int acceptState, int stateSize, PatternType type, std::string regex)
        : subms(subms),
          transVec(transVec),
          transSize(transSize),
          initState(initState),
          acceptState(acceptState),
          stateSize(stateSize),
          type(type),
          regex(regex) {}
};
}  // namespace vlex
