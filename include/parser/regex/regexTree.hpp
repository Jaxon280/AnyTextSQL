#pragma once

#include "common.hpp"
#include "interface.hpp"
#include "parser/nfa.hpp"

using namespace vlex;

NFA *construct_NFA(int tsize, PatternType type);
void destroy_NFA(NFA *nfa);
NFA *copy_NFA(NFA *n);

NFA *build_NFA(char c);
NFA *build_charsets_NFA(uint8_t *chsets);
NFA *build_wildcard_NFA();
NFA *build_digit_NFA();
NFA *build_alph_NFA();
NFA *build_capt_NFA();
NFA *build_submatch_NFA(NFA *nfa, const char *name);
NFA *build_concat_NFA(NFA *n1, NFA *n2);
NFA *build_union_NFA(NFA *n1, NFA *n2);
NFA *build_star_NFA(NFA *n);
NFA *build_plus_NFA(NFA *n);
NFA *build_select_NFA(NFA *n);
NFA *build_num_NFA(NFA *n, int num);

NFA *build_INT();
NFA *build_DOUBLE();
NFA *build_TEXT();

uint8_t *build_c_charsets(char c);
uint8_t *build_range_charsets(char start, char end);
uint8_t *add_charsets(uint8_t *chsets1, uint8_t *chsets2);
uint8_t *negate_charsets(uint8_t *chsets);
