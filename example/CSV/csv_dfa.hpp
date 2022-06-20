#pragma once

#include "../../include/common.hpp"

void generate_dfa(ST_TYPE ***dfa0, ST_TYPE **acceptStates0, int *stateSize0,
                  int *acceptStateSize0) {
    int stateSize = 26;
    ST_TYPE **dfa = (ST_TYPE **)malloc(sizeof(ST_TYPE *) * stateSize);
    for (int i = 0; i < stateSize; i++) {
        dfa[i] = (ST_TYPE *)malloc(sizeof(ST_TYPE) * 256);
    }
    int acceptStateSize = 1;
    ST_TYPE *acceptStates =
        (ST_TYPE *)malloc(sizeof(ST_TYPE) * acceptStateSize);
    acceptStates[0] = 17;

    for (int i = 0; i < stateSize; i++)
        for (int j = 1; j < 256; j++) dfa[i][j] = INV_STATE;

    dfa[INIT_STATE][10] = 2;
    for (int i = 1; i < 256; i++) {
        dfa[2][i] = 2;
    }
    dfa[2][44] = 3;
    for (int i = 1; i < 256; i++) {
        dfa[3][i] = 3;
    }
    dfa[3][44] = 4;
    for (int i = 1; i < 256; i++) {
        dfa[4][i] = 4;
    }
    dfa[4][44] = 5;
    dfa[5][34] = 6;
    for (int i = 1; i < 256; i++) {
        dfa[6][i] = 6;
    }
    dfa[6][71] = 7;
    for (int i = 7; i <= 10; i++) {
        for (int j = 1; j < 256; j++) {
            dfa[i][j] = 6;
        }
    }
    dfa[7][114] = 8;
    dfa[8][101] = 9;
    dfa[9][97] = 10;
    dfa[10][116] = 11;
    for (int i = 1; i < 256; i++) {
        dfa[11][i] = 11;
    }
    dfa[11][34] = 12;
    dfa[12][34] = 11;
    dfa[12][44] = 13;
    for (int i = 1; i < 256; i++) {
        dfa[13][i] = 13;
    }
    dfa[13][44] = 14;
    for (int i = 1; i < 256; i++) {
        dfa[14][i] = 14;
    }
    dfa[14][44] = 15;
    for (int i = 1; i < 256; i++) {
        dfa[15][i] = 15;
    }
    dfa[15][44] = 16;
    for (int i = 1; i < 256; i++) {
        dfa[16][i] = 16;
    }
    dfa[16][44] = 17;
    for (int i = 1; i < 256; i++) {
        dfa[17][i] = 17;
    }
    dfa[17][10] = 0;

    dfa[6][34] = 19;
    dfa[19][34] = 6;
    dfa[19][44] = 20;
    for (int i = 1; i < 256; i++) {
        dfa[20][i] = 20;
    }
    dfa[20][44] = 21;
    for (int i = 1; i < 256; i++) {
        dfa[21][i] = 21;
    }
    dfa[21][44] = 22;
    for (int i = 1; i < 256; i++) {
        dfa[22][i] = 22;
    }
    dfa[22][44] = 23;
    for (int i = 1; i < 256; i++) {
        dfa[23][i] = 23;
    }
    dfa[23][44] = 24;
    for (int i = 1; i < 256; i++) {
        dfa[24][i] = 24;
    }
    dfa[24][10] = 0;
    dfa[24][34] = 0;
    dfa[24][44] = 0;

    *dfa0 = dfa;
    *acceptStates0 = acceptStates;
    *stateSize0 = stateSize;
    *acceptStateSize0 = acceptStateSize;
}
