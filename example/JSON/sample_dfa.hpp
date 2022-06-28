#include "dfa.hpp"

using namespace vlex;

DFA generate_stars_dfa() {
    int stateSize = 14;

    std::vector<std::vector<ST_TYPE>> trans(
        stateSize, std::vector<ST_TYPE>(256, INV_STATE));
    std::set<ST_TYPE> acceptStates;
    acceptStates.insert(13);

    for (int i = INIT_STATE; i < 9; i++)
        for (int j = 0; j < 256; j++) trans[i][j] = INIT_STATE;

    trans[INIT_STATE][34] = 2;  // "
    trans[2][115] = 3;          // s
    trans[3][116] = 4;          // t
    trans[4][97] = 5;           // a
    trans[5][114] = 6;          // r
    trans[6][115] = 7;          // s
    trans[7][34] = 8;           // "
    trans[8][58] = 9;           // :

    for (int i = 2; i < 7; i++) {
        trans[i][34] = 2;  // "
    }
    trans[8][34] = 2;   // "
    trans[8][115] = 3;  // s

    for (int i = 0; i < 256; i++) {
        if (i < 49 || i > 57) continue;
        trans[9][i] = 10;  // [1-9]
    }
    trans[10][46] = 11;
    for (int i = 0; i < 256; i++) {
        if (i < 48 || i > 57) continue;
        trans[10][i] = 10;
        trans[11][i] = 12;  // [0-9]
        trans[12][i] = 12;
    }
    trans[10][44] = 13;
    trans[12][44] = 13;

    std::vector<DFA::SubMatchStates> smsVec;
    DFA::SubMatchStates sms(9, false, 12, true);
    smsVec.push_back(sms);

    DFA dfa = DFA(trans, acceptStates, smsVec, stateSize);
    return dfa;
}

DFA generate_categories_dfa() {
    int stateSize = 29;

    std::vector<std::vector<ST_TYPE>> trans(
        stateSize, std::vector<ST_TYPE>(256, INV_STATE));
    std::set<ST_TYPE> acceptStates;
    acceptStates.insert(27);

    for (int i = INIT_STATE; i < 15; i++)
        for (int j = 0; j < 256; j++) trans[i][j] = INIT_STATE;

    trans[INIT_STATE][34] = 2;  // "
    trans[2][99] = 3;           // c
    trans[3][97] = 4;           // a
    trans[4][116] = 5;          // t
    trans[5][101] = 6;          // e
    trans[6][103] = 7;          // g
    trans[7][111] = 8;          // o
    trans[8][114] = 9;          // r
    trans[9][105] = 10;         // i
    trans[10][101] = 11;        // e
    trans[11][115] = 12;        // s
    trans[12][34] = 13;         // "
    trans[13][58] = 14;         // :
    trans[14][34] = 15;         // "

    for (int i = 2; i < 12; i++) {
        trans[i][34] = 2;  // "
    }
    trans[13][34] = 2;  // "
    trans[13][99] = 3;  // c

    for (int i = 0; i < 256; i++) {
        if (i == 34 || i == 82 || i == 92) continue;
        trans[15][i] = 15;  // [^"\R]
    }
    trans[15][34] = INIT_STATE;  // "
    trans[15][92] = 16;          // \ //
    trans[15][82] = 17;          // R

    for (int i = 0; i < 256; i++) {
        trans[16][i] = 15;
    }
    trans[16][34] = INV_STATE;

    for (int i = 17; i <= 25; i++) {
        for (int j = 0; j < 256; j++) {
            if (j == 34) {
                trans[i][j] = INIT_STATE;
            } else if (j == 92) {
                trans[i][j] = 16;  // \ //
            } else if (j == 82) {
                trans[i][j] = 17;  // R
            } else {
                trans[i][j] = 15;
            }
        }
    }
    trans[17][101] = 18;  // e
    trans[18][115] = 19;  // s
    trans[19][116] = 20;  // t
    trans[20][97] = 21;   // a
    trans[21][117] = 22;  // u
    trans[22][114] = 23;  // r
    trans[23][97] = 24;   // a
    trans[24][110] = 25;  // n
    trans[25][116] = 26;  // t
    for (int i = 0; i < 256; i++) {
        if (i == 34 || i == 92) continue;
        trans[26][i] = 26;
    }
    trans[26][34] = 27;  // accept
    trans[26][92] = 28;  // \ //

    for (int i = 0; i < 256; i++) {
        trans[28][i] = 26;
    }
    trans[28][34] = INV_STATE;

    std::vector<DFA::SubMatchStates> smsVec;
    // DFA::SubMatchStates sms(15, true, 26, true);
    // smsVec.push_back(sms);

    DFA::SubMatchStates sms(14, false, 27, false);
    smsVec.push_back(sms);

    DFA dfa = DFA(trans, acceptStates, smsVec, stateSize);
    return dfa;
}

DFA generate_businessid_dfa() {
    int stateSize = 18;

    std::vector<std::vector<ST_TYPE>> trans(
        stateSize, std::vector<ST_TYPE>(256, INV_STATE));
    std::set<ST_TYPE> acceptStates;
    acceptStates.insert(17);

    for (int i = INIT_STATE; i < 16; i++)
        for (int j = 0; j < 256; j++) trans[i][j] = INIT_STATE;

    trans[INIT_STATE][34] = 2;  // "
    trans[2][98] = 3;           // b
    trans[3][117] = 4;          // u
    trans[4][115] = 5;          // s
    trans[5][105] = 6;          // i
    trans[6][110] = 7;          // n
    trans[7][101] = 8;          // e
    trans[8][115] = 9;          // s
    trans[9][115] = 10;         // s
    trans[10][95] = 11;         // _
    trans[11][105] = 12;        // i
    trans[12][100] = 13;        // d
    trans[13][34] = 14;         // "
    trans[14][58] = 15;         // :
    trans[15][34] = 16;         // "

    for (int i = 2; i < 13; i++) {
        trans[i][34] = 2;  // "
    }
    trans[14][34] = 2;  // "
    trans[14][98] = 3;  // b

    for (int i = 0; i < 256; i++) {
        if (i == 34 || i == 82) continue;
        trans[16][i] = 16;  // [^"\R]
    }
    trans[16][34] = 17;  // "

    std::vector<DFA::SubMatchStates> smsVec;
    // DFA::SubMatchStates sms(15, true, 26, true);
    // smsVec.push_back(sms);

    DFA::SubMatchStates sms(16, true, 16, true);
    smsVec.push_back(sms);

    DFA dfa = DFA(trans, acceptStates, smsVec, stateSize);
    return dfa;
}

DFA generate_categories_stars_dfa() {
    int stateSize = 39;

    std::vector<std::vector<ST_TYPE>> trans(
        stateSize, std::vector<ST_TYPE>(256, INV_STATE));
    std::set<ST_TYPE> acceptStates;
    acceptStates.insert(38);

    for (int i = INIT_STATE; i < 9; i++)
        for (int j = 0; j < 256; j++) trans[i][j] = INIT_STATE;

    trans[INIT_STATE][34] = 2;  // "
    trans[2][115] = 3;          // s
    trans[3][116] = 4;          // t
    trans[4][97] = 5;           // a
    trans[5][114] = 6;          // r
    trans[6][115] = 7;          // s
    trans[7][34] = 8;           // "
    trans[8][58] = 9;           // :

    for (int i = 2; i < 7; i++) {
        trans[i][34] = 2;  // "
    }
    trans[8][34] = 2;   // "
    trans[8][115] = 3;  // s

    for (int i = 49; i < 58; i++) {
        if (i == 49 || i == 50)
            trans[9][i] = 10;  // 1, 2
        else if (i == 51)
            trans[9][i] = 11;  // 3
        else
            trans[9][i] = 12;  // 4~9
    }
    trans[10][46] = 0;
    for (int i = 48; i < 58; i++) {
        trans[10][i] = 12;
    }
    trans[11][46] = 13;
    for (int i = 48; i < 58; i++) {
        trans[11][i] = 12;
    }
    trans[12][46] = 14;
    for (int i = 48; i < 58; i++) {
        trans[12][i] = 12;
    }
    for (int i = 49; i < 58; i++) {
        if (i < 53)
            trans[13][i] = 0;
        else
            trans[13][i] = 15;
    }
    for (int i = 48; i < 58; i++) {
        trans[14][i] = 15;
    }
    for (int i = 48; i < 58; i++) {
        trans[15][i] = 15;
    }

    trans[15][44] = 16;

    for (int i = 16; i < 30; i++)
        for (int j = 0; j < 256; j++) trans[i][j] = 16;

    trans[16][34] = 17;   // "
    trans[17][99] = 18;   // c
    trans[18][97] = 19;   // a
    trans[19][116] = 20;  // t
    trans[20][101] = 21;  // e
    trans[21][103] = 22;  // g
    trans[22][111] = 23;  // o
    trans[23][114] = 24;  // r
    trans[24][105] = 25;  // i
    trans[25][101] = 26;  // e
    trans[26][115] = 27;  // s
    trans[27][34] = 28;   // "
    trans[28][58] = 29;   // :
    trans[29][34] = 30;   // "

    for (int i = 17; i < 27; i++) {
        trans[i][34] = 17;  // "
    }
    trans[28][34] = 17;  // "
    trans[28][99] = 18;  // c

    for (int i = 30; i < 37; i++)
        for (int j = 0; j < 256; j++) {
            if (j == 34)
                trans[i][j] = 0;
            else if (j == 80)
                trans[i][j] = 31;  // P
            else
                trans[i][j] = 30;
        }
    trans[31][101] = 32;  // e
    trans[32][114] = 33;  // r
    trans[33][115] = 34;  // s
    trans[34][105] = 35;  // i
    trans[35][97] = 36;   // a
    trans[36][110] = 37;  // n
    for (int i = 0; i < 256; i++) {
        if (i == 34)
            trans[37][i] = 38;
        else
            trans[37][i] = 37;  // except "
    }

    std::vector<DFA::SubMatchStates> smsVec;
    DFA::SubMatchStates sms1(9, false, 15, false);
    smsVec.push_back(sms1);
    DFA::SubMatchStates sms2(29, false, 38, false);
    smsVec.push_back(sms2);

    DFA dfa = DFA(trans, acceptStates, smsVec, stateSize);
    return dfa;
}
