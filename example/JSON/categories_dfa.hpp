#pragma once

#include "../../include/common.hpp"

void generate_sample_dfa(ST_TYPE** dfa, int stateSize) {
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
        dfa[i][34] = 2;
    }
    dfa[13][34] = 2;
    dfa[13][99] = 3;

    for (int i = 0; i < 128; i++) {
        if (i == 34 || i == 82 || i == 92) continue;
        dfa[15][i] = 15;  // [^"\R]
    }
    dfa[15][34] = INIT_STATE;  // "
    dfa[15][92] = 16;          // \ //
    dfa[15][82] = 18;          // R
    for (int i = 0; i < 128; i++) {
        if (i == 92) continue;
        dfa[16][i] = 15;  // ^\ //
    }
    dfa[16][92] = 17;  // \ //
    dfa[17][46] = 15;  // .
    for (int i = 18; i <= 26; i++) {
        for (int j = 0; j < 128; j++) {
            if (j == 92) {
                dfa[i][j] = 16;  // \ //
            } else if (j == 82) {
                dfa[i][j] = 18;  // R
            } else if (j == 114) {
                dfa[i][j] = INV_STATE;
            } else {
                dfa[i][j] = 15;
            }
        }
    }
    dfa[18][101] = 19;  // e
    dfa[19][115] = 20;  // s
    dfa[20][116] = 21;  // t
    dfa[21][97] = 22;   // a
    dfa[22][117] = 23;  // u
    dfa[23][114] = 24;  // r
    dfa[24][97] = 25;   // a
    dfa[25][110] = 26;  // n
    dfa[26][116] = 27;  // t
    for (int i = 0; i < 128; i++) {
        if (i == 34 || i == 92) continue;
        dfa[27][i] = 27;
    }
    dfa[27][34] = 28;  // accept
    dfa[27][92] = 29;  // \ //
    for (int i = 0; i < 128; i++) {
        if (i == 92) continue;
        dfa[29][i] = 27;
    }
    dfa[29][92] = 30;  // \ //
    dfa[30][46] = 27;  // \ //
}
