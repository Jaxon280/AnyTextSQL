#include "parser/regex/regexTree.hpp"

NFA *constructNFA(int tsize, PatternType type) {
    NFA *nfa = new NFA;
    nfa->subms = NULL;
    nfa->transVec = new Transition[tsize];
    nfa->type = type;
    return nfa;
}

void destroyNFA(NFA *nfa) {
    free(nfa->transVec);
    free(nfa);
}

NFA *copyNFA(NFA *n) {
    NFA *nfa = constructNFA(n->transSize, n->type);
    nfa->initState = n->initState;
    nfa->acceptState = n->acceptState;
    nfa->stateSize = n->stateSize;
    nfa->transSize = n->transSize;
    for (int i = 0; i < nfa->transSize; i++) {
        nfa->transVec[i].start = n->transVec[i].start;
        nfa->transVec[i].end = n->transVec[i].end;
        nfa->transVec[i].c = n->transVec[i].c;
    }
    nfa->subms = n->subms;  // assume NULL
    nfa->isAnyStart = n->isAnyStart;
    nfa->isAnyEnd = n->isAnyEnd;
    return nfa;
}

NFA *buildRegexPattern(NFA *nfa) {
    NFA *nfa1 = buildWildcardNFA();
    NFA *nfa2 = buildStarNFA(nfa1);
    NFA *pattern = buildConcatNFA(nfa2, nfa);
    return pattern;
}

NFA *buildNFA(char c) {
    NFA *nfa = constructNFA(1, TEXT_PT);
    nfa->initState = 0;
    nfa->acceptState = 1;
    nfa->stateSize = 2;

    nfa->transSize = 1;
    nfa->transVec[0].start = 0;
    nfa->transVec[0].end = 1;
    nfa->transVec[0].c = (int)c;
    nfa->isAnyStart = false;
    nfa->isAnyEnd = false;
    return nfa;
}

NFA *buildCharsetsNFA(const uint8_t *chsets) {
    int tsize = 0;
    for (int i = 0; i < ASCII_SZ; i++) {
        if (chsets[i] == 1) {
            tsize++;
        }
    }

    NFA *nfa = constructNFA(tsize, TEXT_PT);
    nfa->initState = 0;
    nfa->acceptState = 1;
    nfa->stateSize = 2;
    nfa->transSize = tsize;
    nfa->transVec = new Transition[tsize];
    int ti = 0;
    for (int i = 0; i < ASCII_SZ; i++) {
        if (chsets[i] == 1) {
            nfa->transVec[ti].start = 0;
            nfa->transVec[ti].end = 1;
            nfa->transVec[ti].c = (int)i;
            ti++;
        }
    }
    nfa->isAnyStart = false;
    nfa->isAnyEnd = false;
    return nfa;
}

NFA *buildWildcardNFA() {
    NFA *nfa = constructNFA(ASCII_SZ, TEXT_PT);
    nfa->stateSize = 2;
    nfa->initState = 0;
    nfa->acceptState = 1;
    nfa->transSize = ASCII_SZ;
    for (int i = 0; i < ASCII_SZ; i++) {
        nfa->transVec[i].start = 0;
        nfa->transVec[i].end = 1;
        nfa->transVec[i].c = i;
    }
    nfa->isAnyStart = false;
    nfa->isAnyEnd = false;
    return nfa;
}

NFA *buildDigitNFA() {
    NFA *nfa = constructNFA(10, INT_PT);
    nfa->stateSize = 2;
    nfa->initState = 0;
    nfa->acceptState = 1;
    nfa->transSize = 10;
    for (int i = 0; i < nfa->transSize; i++) {
        nfa->transVec[i].start = 0;
        nfa->transVec[i].end = 1;
        nfa->transVec[i].c = i + 48;
    }
    nfa->isAnyStart = false;
    nfa->isAnyEnd = false;
    return nfa;
}

NFA *buildAlphNFA() {
    NFA *nfa = constructNFA(26, TEXT_PT);
    nfa->stateSize = 2;
    nfa->initState = 0;
    nfa->acceptState = 1;
    nfa->transSize = 26;
    for (int i = 0; i < nfa->transSize; i++) {
        nfa->transVec[i].start = 0;
        nfa->transVec[i].end = 1;
        nfa->transVec[i].c = i + 97;
    }
    nfa->isAnyStart = false;
    nfa->isAnyEnd = false;
    return nfa;
}

NFA *buildCaptNFA() {
    NFA *nfa = constructNFA(26, TEXT_PT);
    nfa->stateSize = 2;
    nfa->initState = 0;
    nfa->acceptState = 1;
    nfa->transSize = 26;
    for (int i = 0; i < nfa->transSize; i++) {
        nfa->transVec[i].start = 0;
        nfa->transVec[i].end = 1;
        nfa->transVec[i].c = i + 65;
    }
    nfa->isAnyStart = false;
    nfa->isAnyEnd = false;
    return nfa;
}

NFA *buildINT() {
    uint8_t *chset1 = buildCharsets('+');
    uint8_t *chset2 = buildCharsets('-');
    uint8_t *chset3 = addCharsets(chset1, chset2);
    NFA *nfa1 = buildCharsetsNFA(chset3);
    NFA *nfa2 = buildSelectNFA(nfa1);

    NFA *nfa3 = buildNFA('0');
    uint8_t *chset4 = buildRangeCharsets('1', '9');
    NFA *nfa4 = buildCharsetsNFA(chset4);
    uint8_t *chset5 = buildRangeCharsets('0', '9');
    NFA *nfa5 = buildCharsetsNFA(chset5);
    NFA *nfa6 = buildStarNFA(nfa5);
    NFA *nfa7 = buildConcatNFA(nfa4, nfa6);
    NFA *nfa8 = buildUnionNFA(nfa3, nfa7);

    NFA *nfa = buildConcatNFA(nfa2, nfa8);
    nfa->type = INT_PT;
    return nfa;
}

NFA *buildDOUBLE() {
    uint8_t *chset1 = buildCharsets('+');
    uint8_t *chset2 = buildCharsets('-');
    uint8_t *chset3 = addCharsets(chset1, chset2);
    NFA *nfa1 = buildCharsetsNFA(chset3);
    NFA *nfa2 = buildSelectNFA(nfa1);

    NFA *nfa3 = buildNFA('0');
    uint8_t *chset4 = buildRangeCharsets('1', '9');
    NFA *nfa4 = buildCharsetsNFA(chset4);
    uint8_t *chset5 = buildRangeCharsets('0', '9');
    NFA *nfa5 = buildCharsetsNFA(chset5);
    NFA *nfa6 = buildStarNFA(nfa5);
    NFA *nfa7 = buildConcatNFA(nfa4, nfa6);
    NFA *nfa8 = buildUnionNFA(nfa3, nfa7);

    NFA *nfa9 = buildConcatNFA(nfa2, nfa8);

    NFA *nfa10 = buildNFA('.');
    uint8_t *chset6 = buildRangeCharsets('0', '9');
    NFA *nfa11 = buildCharsetsNFA(chset6);
    NFA *nfa12 = buildPlusNFA(nfa11);
    NFA *nfa13 = buildConcatNFA(nfa10, nfa12);

    uint8_t *chset7 = buildCharsets('E');
    uint8_t *chset8 = buildCharsets('e');
    uint8_t *chset9 = addCharsets(chset7, chset8);
    NFA *nfa14 = buildCharsetsNFA(chset9);
    uint8_t *chset10 = buildCharsets('+');
    uint8_t *chset11 = buildCharsets('-');
    uint8_t *chset12 = addCharsets(chset10, chset11);
    NFA *nfa15 = buildCharsetsNFA(chset12);
    NFA *nfa16 = buildSelectNFA(nfa15);
    NFA *nfa17 = buildNFA('0');
    uint8_t *chset13 = buildRangeCharsets('1', '9');
    NFA *nfa18 = buildCharsetsNFA(chset13);
    uint8_t *chset14 = buildRangeCharsets('0', '9');
    NFA *nfa19 = buildCharsetsNFA(chset14);
    NFA *nfa20 = buildStarNFA(nfa19);
    NFA *nfa21 = buildConcatNFA(nfa18, nfa20);
    NFA *nfa22 = buildUnionNFA(nfa17, nfa21);
    NFA *nfa23 = buildConcatNFA(nfa14, nfa16);
    NFA *nfa24 = buildConcatNFA(nfa23, nfa22);
    NFA *nfa25 = buildSelectNFA(nfa24);

    NFA *nfa26 = buildConcatNFA(nfa13, nfa25);

    NFA *nfa = buildConcatNFA(nfa9, nfa26);
    nfa->type = DOUBLE_PT;

    return nfa;
}

NFA *buildTEXT() {
    NFA *nfa1 = buildNFA('\'');

    NFA *nfa2 = buildNFA('\\');
    NFA *nfa3 = buildNFA('.');
    NFA *nfa4 = buildConcatNFA(nfa2, nfa3);

    uint8_t *chset1 = buildCharsets('\n');
    uint8_t *chset2 = buildCharsets('\'');
    uint8_t *chset3 = addCharsets(chset1, chset2);
    uint8_t *chset4 = buildCharsets('\\');
    uint8_t *chset5 = addCharsets(chset3, chset4);
    uint8_t *chset6 = negateCharsets(chset5);
    NFA *nfa5 = buildCharsetsNFA(chset6);

    NFA *nfa6 = buildUnionNFA(nfa4, nfa5);
    NFA *nfa7 = buildStarNFA(nfa6);

    NFA *nfa8 = buildNFA('\'');
    NFA *nfa9 = buildConcatNFA(nfa1, nfa7);
    NFA *nfa = buildConcatNFA(nfa9, nfa8);
    nfa->type = TEXT_PT;

    return nfa;
}

NFA *buildSubmatchNFA(NFA *nfa, const char *name) {
    SubMatch *new_sub = new SubMatch;
    new_sub->start = nfa->initState;
    new_sub->end = nfa->acceptState;
    new_sub->isAnyStart = nfa->isAnyStart;
    new_sub->isAnyEnd = nfa->isAnyEnd;
    new_sub->name = strdup(name);
    new_sub->next = nfa->subms;
    new_sub->type = nfa->type;
    new_sub->predID = 0;  // initial value
    nfa->subms = new_sub;
    return nfa;
}

SubMatch *copySubmatch(SubMatch *sm) {
    SubMatch *sm_new = new SubMatch;
    sm_new->start = sm->start;
    sm_new->end = sm->end;
    sm_new->isAnyStart = sm->isAnyStart;
    sm_new->isAnyEnd = sm->isAnyEnd;
    sm_new->name = sm->name;
    sm_new->type = sm->type;
    sm_new->predID = sm->predID;
    return sm_new;
}

NFA *buildConcatNFA(NFA *n1, NFA *n2) {
    int n1tsize = n1->transSize, n2tsize = n2->transSize,
        n1ssize = n1->stateSize, n2ssize = n2->stateSize;

    n1->transSize += n2tsize + 1;
    n1->stateSize += n2ssize;
    n1->transVec =
        (Transition *)realloc(n1->transVec, sizeof(Transition) * n1->transSize);

    for (int i = 0; i < n2tsize; i++) {
        n1->transVec[n1tsize + i].start = n1ssize + n2->transVec[i].start;
        n1->transVec[n1tsize + i].end = n1ssize + n2->transVec[i].end;
        n1->transVec[n1tsize + i].c = n2->transVec[i].c;
    }

    SubMatch *s = n2->subms;
    SubMatch *s_next;
    while (s != NULL) {
        s->start += n1ssize;
        s->end += n1ssize;
        s_next = s->next;
        s->next = n1->subms;
        n1->subms = s;
        s = s_next;
    }

    n1->transVec[n1->transSize - 1].start = n1->acceptState;
    n1->transVec[n1->transSize - 1].end = n1ssize + n2->initState;
    n1->transVec[n1->transSize - 1].c = EPSILON;

    n1->acceptState = n1ssize + n2->acceptState;
    n1->isAnyStart = n1->isAnyStart;
    n1->isAnyEnd = n2->isAnyEnd;
    if (n1->type == TEXT_PT || n2->type == TEXT_PT ||
        (n1->type == DOUBLE_PT && n2->type == DOUBLE_PT)) {
        n1->type = TEXT_PT;
    } else if (n1->type == INT_PT && n2->type == INT_PT) {
        n1->type = INT_PT;
    } else {
        n1->type = DOUBLE_PT;
    }
    destroyNFA(n2);
    return n1;
}

NFA *buildUnionNFA(NFA *n1, NFA *n2) {
    int n1tsize = n1->transSize, n2tsize = n2->transSize,
        n1ssize = n1->stateSize, n2ssize = n2->stateSize;

    NFA *nfa = constructNFA(n1tsize + n2tsize + 4, TEXT_PT);
    nfa->transSize = n1tsize + n2tsize + 4;
    nfa->stateSize = n1ssize + n2ssize + 2;
    nfa->initState = 0;
    nfa->acceptState = nfa->stateSize - 1;
    nfa->transVec[0].start = nfa->initState;
    nfa->transVec[0].end = n1->initState + 1;
    nfa->transVec[0].c = EPSILON;
    nfa->transVec[1].start = n1->acceptState + 1;
    nfa->transVec[1].end = nfa->acceptState;
    nfa->transVec[1].c = EPSILON;
    nfa->transVec[2].start = nfa->initState;
    nfa->transVec[2].end = n1ssize + n2->initState + 1;
    nfa->transVec[2].c = EPSILON;
    nfa->transVec[3].start = n1ssize + n2->acceptState + 1;
    nfa->transVec[3].end = nfa->acceptState;
    nfa->transVec[3].c = EPSILON;

    for (int i = 0; i < n1tsize; i++) {
        nfa->transVec[i + 4].start = n1->transVec[i].start + 1;
        nfa->transVec[i + 4].end = n1->transVec[i].end + 1;
        nfa->transVec[i + 4].c = n1->transVec[i].c;
    }

    for (int i = 0; i < n2tsize; i++) {
        nfa->transVec[i + n1tsize + 4].start =
            n2->transVec[i].start + n1ssize + 1;
        nfa->transVec[i + n1tsize + 4].end = n2->transVec[i].end + n1ssize + 1;
        nfa->transVec[i + n1tsize + 4].c = n2->transVec[i].c;
    }

    destroyNFA(n1);
    destroyNFA(n2);
    return nfa;
}

NFA *buildStarNFA(NFA *n) {
    int ntsize = n->transSize, nssize = n->stateSize;
    n->transSize += 4;
    n->stateSize += 2;
    int initState = nssize, acceptState = nssize + 1;
    n->transVec =
        (Transition *)realloc(n->transVec, sizeof(Transition) * n->transSize);
    n->transVec[ntsize].start = initState;
    n->transVec[ntsize].end = n->initState;
    n->transVec[ntsize].c = EPSILON;
    n->transVec[ntsize + 1].start = n->acceptState;
    n->transVec[ntsize + 1].end = n->initState;
    n->transVec[ntsize + 1].c = EPSILON;
    n->transVec[ntsize + 2].start = n->acceptState;
    n->transVec[ntsize + 2].end = acceptState;
    n->transVec[ntsize + 2].c = EPSILON;
    n->transVec[ntsize + 3].start = initState;
    n->transVec[ntsize + 3].end = acceptState;
    n->transVec[ntsize + 3].c = EPSILON;
    n->initState = initState;
    n->acceptState = acceptState;
    n->isAnyStart = true;
    n->isAnyEnd = true;

    if (n->type == DOUBLE_PT) {
        n->type = TEXT_PT;
    }
    return n;
}

NFA *buildPlusNFA(NFA *n) {
    NFA *n1 = copyNFA(n);
    NFA *nr = buildStarNFA(n1);
    NFA *npr = buildConcatNFA(n, nr);
    return npr;
}

NFA *buildSelectNFA(NFA *n) {
    int ntsize = n->transSize, nssize = n->stateSize;
    n->transSize += 3;
    n->stateSize += 2;
    int initState = nssize, acceptState = nssize + 1;
    n->transVec =
        (Transition *)realloc(n->transVec, sizeof(Transition) * n->transSize);
    n->transVec[ntsize].start = initState;
    n->transVec[ntsize].end = n->initState;
    n->transVec[ntsize].c = EPSILON;
    n->transVec[ntsize + 1].start = n->acceptState;
    n->transVec[ntsize + 1].end = acceptState;
    n->transVec[ntsize + 1].c = EPSILON;
    n->transVec[ntsize + 2].start = initState;
    n->transVec[ntsize + 2].end = acceptState;
    n->transVec[ntsize + 2].c = EPSILON;
    n->initState = initState;
    n->acceptState = acceptState;
    n->isAnyStart = false;
    n->isAnyEnd = false;
    return n;
}

NFA *buildNumNFA(NFA *n, int num) {
    int tsize = n->transSize, ssize = n->stateSize;

    n->transSize = tsize * num + (num - 1);
    n->stateSize = ssize * num;
    n->transVec =
        (Transition *)realloc(n->transVec, sizeof(Transition) * n->transSize);

    for (int i = 1; i < num; i++) {
        n->transVec[(tsize + 1) * i - 1].start =
            ssize * (i - 1) + n->acceptState;
        n->transVec[(tsize + 1) * i - 1].end = ssize * i + n->initState;
        n->transVec[(tsize + 1) * i - 1].c = EPSILON;
        for (int j = 0; j < tsize; j++) {
            n->transVec[(tsize + 1) * i + j].start =
                ssize * i + n->transVec[j].start;
            n->transVec[(tsize + 1) * i + j].end =
                ssize * i + n->transVec[j].end;
            n->transVec[(tsize + 1) * i + j].c = n->transVec[j].c;
        }
    }

    n->acceptState = ssize * (num - 1) + n->acceptState;
    if (n->type == DOUBLE_PT) {
        n->type = TEXT_PT;
    }

    return n;
}

uint8_t *buildCharsets(char c) {
    uint8_t *chsets = (uint8_t *)calloc(sizeof(uint8_t), ASCII_SZ);
    chsets[(int)c] = 1;
    return chsets;
}

uint8_t *addCharsets(uint8_t *chsets1, uint8_t *chsets2) {
    for (int i = 0; i < ASCII_SZ; i++) {
        chsets1[i] = chsets1[i] | chsets2[i];
    }
    free(chsets2);
    return chsets1;
}

uint8_t *buildRangeCharsets(char start, char end) {
    uint8_t *chsets = (uint8_t *)calloc(sizeof(uint8_t), ASCII_SZ);
    for (char i = start; i <= end; i++) {
        chsets[(int)i] = 1;
    }
    return chsets;
}

uint8_t *negateCharsets(uint8_t *chsets) {
    for (int i = 0; i < ASCII_SZ; i++) {
        chsets[i] = chsets[i] ^ 1;
    }
    return chsets;
}
