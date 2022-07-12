#include "dfa.hpp"

using namespace vlex;

DFA generate_reviewtext_dfa() {
    int stateSize = 22;

    std::vector<std::vector<DFA_ST_TYPE>> trans(
        stateSize, std::vector<DFA_ST_TYPE>(256, INV_STATE));
    std::set<DFA_ST_TYPE> acceptStates;
    acceptStates.insert(20);

    for (int i = 0; i < stateSize; i++)
        for (int j = 0; j < 256; j++) trans[i][j] = INV_STATE;

    for (int j = 0; j < 256; j++) trans[INIT_STATE][j] = INIT_STATE;
    trans[INIT_STATE][10] = 2;
    for (int i = 0; i < 256; i++) {
        trans[2][i] = 2;  // funny
    }
    trans[2][44] = 3;
    for (int i = 0; i < 256; i++) {
        trans[3][i] = 3;  // useful
    }
    trans[3][44] = 4;
    for (int i = 0; i < 256; i++) {
        trans[4][i] = 4;  // review_id
    }
    trans[4][44] = 5;
    trans[5][34] = 6;  // text
    for (int i = 0; i < 256; i++) {
        trans[6][i] = 6;
    }
    trans[6][34] = 21;
    trans[21][34] = 6;
    trans[6][71] = 7;  // G
    // trans[6][103] = 7; // g
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
        trans[13][i] = 13;  // business_id
    }
    trans[13][44] = 14;
    for (int i = 48; i < 58; i++) {
        trans[14][i] = 15;  // stars
    }
    for (int i = 48; i < 58; i++) {
        trans[15][i] = 15;  // stars
    }
    trans[15][46] = 16;
    for (int i = 48; i < 58; i++) {
        trans[16][i] = 17;  // stars
    }
    for (int i = 48; i < 58; i++) {
        trans[17][i] = 17;  // stars
    }
    trans[17][44] = 18;
    for (int i = 0; i < 256; i++) {
        trans[18][i] = 18;  // date
    }
    trans[18][44] = 19;
    for (int i = 0; i < 256; i++) {
        trans[19][i] = 19;  // user_id
    }
    trans[19][44] = 20;
    for (int i = 0; i < 256; i++) {
        trans[20][i] = 20;  // cool
    }
    trans[20][10] = 0;

    std::vector<DFA::SubMatchStates> smsVec;
    DFA::SubMatchStates sms1(14, false, 17, true);
    smsVec.push_back(sms1);

    DFA dfa = DFA(trans, acceptStates, smsVec, stateSize);
    return dfa;
}
