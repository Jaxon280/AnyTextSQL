#pragma once

#include "../../include/common.hpp"

void generate_stars_dfa(ST_TYPE ***dfa0, ST_TYPE **acceptStates0,
                        int *stateSize0, int *acceptStateSize0) {
    int stateSize = 14;
    ST_TYPE **dfa = (ST_TYPE **)malloc(sizeof(ST_TYPE *) * stateSize);
    for (int i = 0; i < stateSize; i++) {
        dfa[i] = (ST_TYPE *)malloc(sizeof(ST_TYPE) * 128);
    }
    int acceptStateSize = 3;
    ST_TYPE *acceptStates =
        (ST_TYPE *)malloc(sizeof(ST_TYPE) * acceptStateSize);
    acceptStates[0] = 10;
    acceptStates[1] = 11;
    acceptStates[2] = 13;

    for (int i = 0; i < stateSize; i++)
        for (int j = 0; j < ASCII_SZ; j++) dfa[i][j] = INV_STATE;

    for (int i = INIT_STATE; i < 9; i++)
        for (int j = 0; j < ASCII_SZ; j++) dfa[i][j] = INIT_STATE;

    dfa[INIT_STATE][34] = 2;  // "
    dfa[2][115] = 3;          // s
    dfa[3][116] = 4;          // t
    dfa[4][97] = 5;           // a
    dfa[5][114] = 6;          // r
    dfa[6][115] = 7;          // s
    dfa[7][34] = 8;           // "
    dfa[8][58] = 9;           // :

    for (int i = 2; i < 7; i++) {
        dfa[i][34] = 2;  // "
    }
    dfa[8][34] = 2;   // "
    dfa[8][115] = 3;  // s

    for (int i = 0; i < 128; i++) {
        if (i < 49 || i > 57) continue;
        dfa[9][i] = 10;  // [1-9]
    }
    dfa[9][48] = 11;

    dfa[10][46] = 12;
    dfa[11][46] = 12;
    for (int i = 0; i < 128; i++) {
        if (i < 48 || i > 57) continue;
        dfa[10][i] = 10;  // [0-9]
        dfa[12][i] = 13;
        dfa[13][i] = 13;
    }

    *dfa0 = dfa;
    *acceptStates0 = acceptStates;
    *stateSize0 = stateSize;
    *acceptStateSize0 = acceptStateSize;
}
