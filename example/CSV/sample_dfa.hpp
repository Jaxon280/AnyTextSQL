#include "dfa.hpp"

using namespace vlex;

DFA generate_reviewtext_dfa() {
    int stateSize = 26;

    std::vector<std::vector<ST_TYPE>> trans(
        stateSize, std::vector<ST_TYPE>(256, INV_STATE));
    std::set<ST_TYPE> acceptStates;
    acceptStates.insert(17);

    for (int i = 0; i < stateSize; i++)
        for (int j = 0; j < 256; j++) trans[i][j] = INV_STATE;

    for (int j = 0; j < 256; j++) trans[INIT_STATE][j] = INIT_STATE;
    trans[INIT_STATE][10] = 2;
    for (int i = 0; i < 256; i++) {
        trans[2][i] = 2;
    }
    trans[2][44] = 3;
    for (int i = 0; i < 256; i++) {
        trans[3][i] = 3;
    }
    trans[3][44] = 4;
    for (int i = 0; i < 256; i++) {
        trans[4][i] = 4;
    }
    trans[4][44] = 5;
    trans[5][34] = 6;
    for (int i = 0; i < 256; i++) {
        trans[6][i] = 6;
    }
    trans[6][71] = 7;  // G
    for (int i = 7; i <= 10; i++) {
        for (int j = 0; j < 256; j++) {
            trans[i][j] = 6;
        }
    }
    trans[7][114] = 8;    // r
    trans[8][101] = 9;    // e
    trans[9][97] = 10;    // a
    trans[10][116] = 11;  // t
    for (int i = 0; i < 256; i++) {
        trans[11][i] = 11;
    }
    trans[11][34] = 12;
    trans[12][34] = 11;
    trans[12][44] = 13;
    for (int i = 0; i < 256; i++) {
        trans[13][i] = 13;
    }
    trans[13][44] = 14;
    for (int i = 0; i < 256; i++) {
        trans[14][i] = 14;
    }
    trans[14][44] = 15;
    for (int i = 0; i < 256; i++) {
        trans[15][i] = 15;
    }
    trans[15][44] = 16;
    for (int i = 0; i < 256; i++) {
        trans[16][i] = 16;
    }
    trans[16][44] = 17;
    for (int i = 0; i < 256; i++) {
        trans[17][i] = 17;
    }
    trans[17][10] = 0;

    trans[6][34] = 19;
    trans[19][34] = 6;
    trans[19][44] = 20;
    for (int i = 0; i < 256; i++) {
        trans[20][i] = 20;
    }
    trans[20][44] = 21;
    for (int i = 0; i < 256; i++) {
        trans[21][i] = 21;
    }
    trans[21][44] = 22;
    for (int i = 0; i < 256; i++) {
        trans[22][i] = 22;
    }
    trans[22][44] = 23;
    for (int i = 0; i < 256; i++) {
        trans[23][i] = 23;
    }
    trans[23][44] = 24;
    for (int i = 0; i < 256; i++) {
        trans[24][i] = 24;
    }
    trans[24][10] = 0;
    trans[24][34] = 0;
    trans[24][44] = 0;

    DFA dfa = DFA(trans, acceptStates, stateSize);
    return dfa;
}
