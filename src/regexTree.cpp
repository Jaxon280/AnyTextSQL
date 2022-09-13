#include "parser/regex/regexTree.hpp"

NFA *construct_NFA(int tsize, PatternType type) {
    NFA *nfa = (NFA *)malloc(sizeof(NFA));
    nfa->subms = NULL;
    nfa->transVec = (Transition *)malloc(sizeof(Transition) * tsize);
    nfa->type = type;
    return nfa;
}

void destroy_NFA(NFA *nfa) {
    free(nfa->transVec);
    free(nfa);
}

NFA *copy_NFA(NFA *n) {
    NFA *nfa = construct_NFA(n->transSize, n->type);
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

NFA *build_NFA(char c) {
    NFA *nfa = construct_NFA(1, TEXT_PT);
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

NFA *build_charsets_NFA(uint8_t *chsets) {
    int tsize = 0;
    for (int i = 0; i < ASCII_SZ; i++) {
        if (chsets[i] == 1) {
            tsize++;
        }
    }

    NFA *nfa = construct_NFA(tsize, TEXT_PT);
    nfa->initState = 0;
    nfa->acceptState = 1;
    nfa->stateSize = 2;
    nfa->transSize = tsize;
    nfa->transVec = (Transition *)malloc(sizeof(Transition) * tsize);
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

NFA *build_wildcard_NFA() {
    NFA *nfa = construct_NFA(ASCII_SZ, TEXT_PT);
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

NFA *build_digit_NFA() {
    NFA *nfa = construct_NFA(10, INT_PT);
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

NFA *build_alph_NFA() {
    NFA *nfa = construct_NFA(26, TEXT_PT);
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

NFA *build_capt_NFA() {
    NFA *nfa = construct_NFA(26, TEXT_PT);
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

NFA *build_INT() {
    uint8_t *chset1 = build_c_charsets('+');
    uint8_t *chset2 = build_c_charsets('-');
    uint8_t *chset3 = add_charsets(chset1, chset2);
    NFA *nfa1 = build_charsets_NFA(chset3);
    NFA *nfa2 = build_select_NFA(nfa1);

    NFA *nfa3 = build_NFA('0');
    uint8_t *chset4 = build_range_charsets('1', '9');
    NFA *nfa4 = build_charsets_NFA(chset4);
    uint8_t *chset5 = build_range_charsets('0', '9');
    NFA *nfa5 = build_charsets_NFA(chset5);
    NFA *nfa6 = build_star_NFA(nfa5);
    NFA *nfa7 = build_concat_NFA(nfa4, nfa6);
    NFA *nfa8 = build_union_NFA(nfa3, nfa7);

    NFA *nfa = build_concat_NFA(nfa2, nfa8);
    nfa->type = INT_PT;
    return nfa;
}

NFA *build_DOUBLE() {
    uint8_t *chset1 = build_c_charsets('+');
    uint8_t *chset2 = build_c_charsets('-');
    uint8_t *chset3 = add_charsets(chset1, chset2);
    NFA *nfa1 = build_charsets_NFA(chset3);
    NFA *nfa2 = build_select_NFA(nfa1);

    NFA *nfa3 = build_NFA('0');
    uint8_t *chset4 = build_range_charsets('1', '9');
    NFA *nfa4 = build_charsets_NFA(chset4);
    uint8_t *chset5 = build_range_charsets('0', '9');
    NFA *nfa5 = build_charsets_NFA(chset5);
    NFA *nfa6 = build_star_NFA(nfa5);
    NFA *nfa7 = build_concat_NFA(nfa4, nfa6);
    NFA *nfa8 = build_union_NFA(nfa3, nfa7);

    NFA *nfa9 = build_concat_NFA(nfa2, nfa8);

    NFA *nfa10 = build_NFA('.');
    uint8_t *chset6 = build_range_charsets('0', '9');
    NFA *nfa11 = build_charsets_NFA(chset6);
    NFA *nfa12 = build_plus_NFA(nfa11);
    NFA *nfa13 = build_concat_NFA(nfa10, nfa12);

    uint8_t *chset7 = build_c_charsets('E');
    uint8_t *chset8 = build_c_charsets('e');
    uint8_t *chset9 = add_charsets(chset7, chset8);
    NFA *nfa14 = build_charsets_NFA(chset9);
    uint8_t *chset10 = build_c_charsets('+');
    uint8_t *chset11 = build_c_charsets('-');
    uint8_t *chset12 = add_charsets(chset10, chset11);
    NFA *nfa15 = build_charsets_NFA(chset12);
    NFA *nfa16 = build_select_NFA(nfa15);
    NFA *nfa17 = build_NFA('0');
    uint8_t *chset13 = build_range_charsets('1', '9');
    NFA *nfa18 = build_charsets_NFA(chset13);
    uint8_t *chset14 = build_range_charsets('0', '9');
    NFA *nfa19 = build_charsets_NFA(chset14);
    NFA *nfa20 = build_star_NFA(nfa19);
    NFA *nfa21 = build_concat_NFA(nfa18, nfa20);
    NFA *nfa22 = build_union_NFA(nfa17, nfa21);
    NFA *nfa23 = build_concat_NFA(nfa14, nfa16);
    NFA *nfa24 = build_concat_NFA(nfa23, nfa22);
    NFA *nfa25 = build_select_NFA(nfa24);

    NFA *nfa26 = build_concat_NFA(nfa13, nfa25);

    NFA *nfa = build_concat_NFA(nfa9, nfa26);
    nfa->type = DOUBLE_PT;

    return nfa;
}

NFA *build_TEXT() {
    NFA *nfa1 = build_NFA('\'');

    NFA *nfa2 = build_NFA('\\');
    NFA *nfa3 = build_NFA('.');
    NFA *nfa4 = build_concat_NFA(nfa2, nfa3);

    uint8_t *chset1 = build_c_charsets('\n');
    uint8_t *chset2 = build_c_charsets('\'');
    uint8_t *chset3 = add_charsets(chset1, chset2);
    uint8_t *chset4 = build_c_charsets('\\');
    uint8_t *chset5 = add_charsets(chset3, chset4);
    uint8_t *chset6 = negate_charsets(chset5);
    NFA *nfa5 = build_charsets_NFA(chset6);

    NFA *nfa6 = build_union_NFA(nfa4, nfa5);
    NFA *nfa7 = build_star_NFA(nfa6);

    NFA *nfa8 = build_NFA('\'');
    NFA *nfa9 = build_concat_NFA(nfa1, nfa7);
    NFA *nfa = build_concat_NFA(nfa9, nfa8);
    nfa->type = TEXT_PT;

    return nfa;
}

NFA *build_submatch_NFA(NFA *nfa, char *name) {
    SubMatch *new_sub = (SubMatch *)malloc(sizeof(SubMatch));
    new_sub->start = nfa->initState;
    new_sub->end = nfa->acceptState;
    new_sub->isAnyStart = nfa->isAnyStart;
    new_sub->isAnyEnd = nfa->isAnyEnd;
    new_sub->name = strdup(name);
    new_sub->next = nfa->subms;
    new_sub->type = nfa->type;
    nfa->subms = new_sub;
    return nfa;
}

SubMatch *copy_submatch(SubMatch *sm) {
    SubMatch *sm_new = (SubMatch *)malloc(sizeof(SubMatch));
    sm_new->start = sm->start;
    sm_new->end = sm->end;
    sm_new->isAnyStart = sm->isAnyStart;
    sm_new->isAnyEnd = sm->isAnyEnd;
    sm_new->name = sm->name;
    sm_new->type = sm->type;
    return sm_new;
}

NFA *build_concat_NFA(NFA *n1, NFA *n2) {
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
    destroy_NFA(n2);
    return n1;
}

NFA *build_union_NFA(NFA *n1, NFA *n2) {
    int n1tsize = n1->transSize, n2tsize = n2->transSize,
        n1ssize = n1->stateSize, n2ssize = n2->stateSize;

    NFA *nfa = construct_NFA(n1tsize + n2tsize + 4, TEXT_PT);
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

    destroy_NFA(n1);
    destroy_NFA(n2);
    return nfa;
}

NFA *build_star_NFA(NFA *n) {
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

NFA *build_plus_NFA(NFA *n) {
    NFA *n1 = copy_NFA(n);
    NFA *nr = build_star_NFA(n1);
    NFA *npr = build_concat_NFA(n, nr);
    return npr;
}

NFA *build_select_NFA(NFA *n) {
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

NFA *build_num_NFA(NFA *n, int num) {
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

uint8_t *build_c_charsets(char c) {
    uint8_t *chsets = (uint8_t *)calloc(sizeof(uint8_t), ASCII_SZ);
    chsets[(int)c] = 1;
    return chsets;
}

uint8_t *add_charsets(uint8_t *chsets1, uint8_t *chsets2) {
    for (int i = 0; i < ASCII_SZ; i++) {
        chsets1[i] = chsets1[i] | chsets2[i];
    }
    free(chsets2);
    return chsets1;
}

uint8_t *build_range_charsets(char start, char end) {
    uint8_t *chsets = (uint8_t *)calloc(sizeof(uint8_t), ASCII_SZ);
    for (char i = start; i <= end; i++) {
        chsets[(int)i] = 1;
    }
    return chsets;
}

uint8_t *negate_charsets(uint8_t *chsets) {
    for (int i = 0; i < ASCII_SZ; i++) {
        chsets[i] = chsets[i] ^ 1;
    }
    return chsets;
}
