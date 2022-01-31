#pragma once

#include "../../include/common.hpp"

void generate_categories_dfa(ST_TYPE ***dfa0, ST_TYPE **acceptStates0,
                             int *stateSize0, int *acceptStateSize0) {
    int stateSize = 29;
    ST_TYPE **dfa = (ST_TYPE **)malloc(sizeof(ST_TYPE *) * stateSize);
    for (int i = 0; i < stateSize; i++) {
        dfa[i] = (ST_TYPE *)malloc(sizeof(ST_TYPE) * 128);
    }
    int acceptStateSize = 1;
    ST_TYPE *acceptStates =
        (ST_TYPE *)malloc(sizeof(ST_TYPE) * acceptStateSize);
    acceptStates[0] = 27;

    for (int i = 0; i < stateSize; i++)
        for (int j = 0; j < ASCII_SZ; j++) dfa[i][j] = INV_STATE;

    for (int i = INIT_STATE; i < 15; i++)
        for (int j = 0; j < ASCII_SZ; j++) dfa[i][j] = INIT_STATE;

    dfa[INIT_STATE][34] = 2;  // "
    dfa[2][99] = 3;           // c
    dfa[3][97] = 4;           // a
    dfa[4][116] = 5;          // t
    dfa[5][101] = 6;          // e
    dfa[6][103] = 7;          // g
    dfa[7][111] = 8;          // o
    dfa[8][114] = 9;          // r
    dfa[9][105] = 10;         // i
    dfa[10][101] = 11;        // e
    dfa[11][115] = 12;        // s
    dfa[12][34] = 13;         // "
    dfa[13][58] = 14;         // :
    dfa[14][34] = 15;         // "

    for (int i = 2; i < 12; i++) {
        dfa[i][34] = 2;  // "
    }
    dfa[13][34] = 2;  // "
    dfa[13][99] = 3;  // c

    for (int i = 0; i < 128; i++) {
        if (i == 34 || i == 82 || i == 92) continue;
        dfa[15][i] = 15;  // [^"\R]
    }
    dfa[15][34] = INIT_STATE;  // "
    dfa[15][92] = 16;          // \ //
    dfa[15][82] = 17;          // R

    for (int i = 0; i < 128; i++) {
        dfa[16][i] = 15;
    }
    dfa[16][34] = INV_STATE;

    for (int i = 17; i <= 25; i++) {
        for (int j = 0; j < 128; j++) {
            if (j == 34) {
                dfa[i][j] = INIT_STATE;
            } else if (j == 92) {
                dfa[i][j] = 16;  // \ //
            } else if (j == 82) {
                dfa[i][j] = 17;  // R
            } else {
                dfa[i][j] = 15;
            }
        }
    }
    dfa[17][101] = 18;  // e
    dfa[18][115] = 19;  // s
    dfa[19][116] = 20;  // t
    dfa[20][97] = 21;   // a
    dfa[21][117] = 22;  // u
    dfa[22][114] = 23;  // r
    dfa[23][97] = 24;   // a
    dfa[24][110] = 25;  // n
    dfa[25][116] = 26;  // t
    for (int i = 0; i < 128; i++) {
        if (i == 34 || i == 92) continue;
        dfa[26][i] = 26;
    }
    dfa[26][34] = 27;  // accept
    dfa[26][92] = 28;  // \ //

    for (int i = 0; i < 128; i++) {
        dfa[28][i] = 26;
    }
    dfa[28][34] = INV_STATE;

    *dfa0 = dfa;
    *acceptStates0 = acceptStates;
    *stateSize0 = stateSize;
    *acceptStateSize0 = acceptStateSize;
}
