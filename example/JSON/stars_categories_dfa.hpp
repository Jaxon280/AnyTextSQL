#pragma once

#include "../../include/common.hpp"

void generate_dfa(DFA_ST_TYPE ***dfa0, DFA_ST_TYPE **acceptStates0,
                  int *stateSize0, int *acceptStateSize0) {
    int stateSize = 39;
    DFA_ST_TYPE **dfa =
        (DFA_ST_TYPE **)malloc(sizeof(DFA_ST_TYPE *) * stateSize);
    for (int i = 0; i < stateSize; i++) {
        dfa[i] = (DFA_ST_TYPE *)malloc(sizeof(DFA_ST_TYPE) * 128);
    }
    int acceptStateSize = 1;
    DFA_ST_TYPE *acceptStates =
        (DFA_ST_TYPE *)malloc(sizeof(DFA_ST_TYPE) * acceptStateSize);
    acceptStates[0] = 38;

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

    for (int i = 49; i < 58; i++) {
        if (i == 49 || i == 50)
            dfa[9][i] = 10;  // 1, 2
        else if (i == 51)
            dfa[9][i] = 11;  // 3
        else
            dfa[9][i] = 12;  // 4~9
    }
    dfa[10][46] = 0;
    for (int i = 48; i < 58; i++) {
        dfa[10][i] = 12;
    }
    dfa[11][46] = 13;
    for (int i = 48; i < 58; i++) {
        dfa[11][i] = 12;
    }
    dfa[12][46] = 14;
    for (int i = 48; i < 58; i++) {
        dfa[12][i] = 12;
    }
    for (int i = 49; i < 58; i++) {
        if (i < 53)
            dfa[13][i] = 0;
        else
            dfa[13][i] = 15;
    }
    for (int i = 48; i < 58; i++) {
        dfa[14][i] = 15;
    }
    for (int i = 48; i < 58; i++) {
        dfa[15][i] = 15;
    }

    dfa[15][44] = 16;

    for (int i = 16; i < 30; i++)
        for (int j = 0; j < ASCII_SZ; j++) dfa[i][j] = 16;

    dfa[16][34] = 17;   // "
    dfa[17][99] = 18;   // c
    dfa[18][97] = 19;   // a
    dfa[19][116] = 20;  // t
    dfa[20][101] = 21;  // e
    dfa[21][103] = 22;  // g
    dfa[22][111] = 23;  // o
    dfa[23][114] = 24;  // r
    dfa[24][105] = 25;  // i
    dfa[25][101] = 26;  // e
    dfa[26][115] = 27;  // s
    dfa[27][34] = 28;   // "
    dfa[28][58] = 29;   // :
    dfa[29][34] = 30;   // "

    for (int i = 17; i < 27; i++) {
        dfa[i][34] = 17;  // "
    }
    dfa[28][34] = 17;  // "
    dfa[28][99] = 18;  // c

    for (int i = 30; i < 37; i++)
        for (int j = 0; j < ASCII_SZ; j++) {
            if (j == 34)
                dfa[i][j] = 0;
            else if (j == 80)
                dfa[i][j] = 31;  // P
            else
                dfa[i][j] = 30;
        }
    dfa[31][101] = 32;  // e
    dfa[32][114] = 33;  // r
    dfa[33][115] = 34;  // s
    dfa[34][105] = 35;  // i
    dfa[35][97] = 36;   // a
    dfa[36][110] = 37;  // n
    for (int i = 0; i < ASCII_SZ; i++) {
        if (i == 34)
            dfa[37][i] = 38;
        else
            dfa[37][i] = 37;  // except "
    }

    *dfa0 = dfa;
    *acceptStates0 = acceptStates;
    *stateSize0 = stateSize;
    *acceptStateSize0 = acceptStateSize;
}
