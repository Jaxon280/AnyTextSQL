#pragma once

#include "common/charcode.hpp"
#include "common/common.hpp"
#include "parser/inter/nfa.hpp"
#include "runner/table.hpp"

using namespace vlex;

void destroyNFA(NFA *nfa);
NFA *copyNFA(NFA *n);

NFA *buildNFA(char c);
NFA *buildCharsetsNFA(const uint8_t *chsets);
NFA *buildWildcardNFA();
NFA *buildDigitNFA();
NFA *buildAlphNFA();
NFA *buildCaptNFA();
NFA *buildSubmatchNFA(NFA *nfa, const char *name);
NFA *buildConcatNFA(NFA *n1, NFA *n2);
NFA *buildUnionNFA(NFA *n1, NFA *n2);
NFA *buildStarNFA(NFA *n);
NFA *buildPlusNFA(NFA *n);
NFA *buildSelectNFA(NFA *n);
NFA *buildNumNFA(NFA *n, int num);
NFA *buildParenthesis(NFA *n);

NFA *buildINT();
NFA *buildDOUBLE();

uint8_t *buildCharsets(char c);
uint8_t *buildRangeCharsets(char start, char end);
uint8_t *addCharsets(uint8_t *chsets1, uint8_t *chsets2);
uint8_t *negateCharsets(uint8_t *chsets);
std::string restoreChsetRegex(const uint8_t *chsets);
