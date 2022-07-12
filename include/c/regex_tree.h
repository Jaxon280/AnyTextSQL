#pragma once

#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define EPSILON -1
#define ASCII_SZ 128

typedef enum pattern_type { INT_PT, DOUBLE_PT, TEXT_PT } PatternType;

typedef struct submatch {
    int start;
    int end;
    bool isAnyStart;
    bool isAnyEnd;
    char *name;
    struct submatch *next;
    PatternType type;
} SubMatch;  // linked list

typedef struct transition {
    int start;
    int end;
    int c;  // 0x00-0xff and epsilon (-1)
} Transition;

typedef struct nfa {
    SubMatch *subms;
    Transition *transVec;
    int transSize;
    int initState;
    int acceptState;
    int stateSize;
    bool isAnyStart;
    bool isAnyEnd;
    PatternType type;
} NFA;

NFA *construct_NFA(int tsize, PatternType type);
void destroy_NFA(NFA *nfa);
NFA *copy_NFA(NFA *n);

NFA *build_NFA(char c);
NFA *build_charsets_NFA(uint8_t *chsets);
NFA *build_wildcard_NFA();
NFA *build_digit_NFA();
NFA *build_alph_NFA();
NFA *build_capt_NFA();
NFA *build_submatch_NFA(NFA *nfa, char *name);
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
