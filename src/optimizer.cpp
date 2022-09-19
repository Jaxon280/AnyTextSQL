#include "optimizer.hpp"

using namespace vlex;

void QueryOptimizer::getTextPred(QueryContext *ctx) {
    count = 0;
    for (OpTree *opt : ctx->getPList()) {
        if (opt->type == TEXT) {
            map[opt->left->varKey].push_back(opt);
            opt->textPredId = count + 1;
            count++;
        }
    }
}

NFA *QueryOptimizer::optimizeNFA(NFA *originalNFA, QueryContext *ctx) {
    NFA *nfa = copy_NFA(originalNFA);
    for (auto it = map.begin(); it != map.end(); ++it) {
        int id = 1;
        for (OpTree *opt : it->second) {
            NFA *predNFA = constructPredNFA(opt);
            nfa = mergeNFA(nfa, predNFA, it->first, id);
            id++;
            // todo: multiple optrees
        }
    }
    return nfa;
}

NFA *QueryOptimizer::mergeNFA(NFA *n1, NFA *n2, const std::string &sub,
                              int id) {
    int s1 = n1->stateSize, s2 = n2->stateSize;
    int t1 = n1->transSize, t2 = n2->transSize;
    SubMatch *sms;
    for (SubMatch *sm = n1->subms; sm != NULL; sm = sm->next) {
        if (sub == std::string(n1->subms->name, strlen(n1->subms->name)) &&
            sm->predID == 0) {
            sms = sm;
        }
    }

    int ssize = s1 + s2 + 2, tsize = t1 + t2 + 4;
    NFA *nfa = construct_NFA(tsize, TEXT_PT);

    nfa->initState = n1->initState;
    nfa->acceptState = n1->acceptState;
    nfa->stateSize = ssize;
    nfa->transSize = tsize;
    nfa->isAnyStart = n1->isAnyStart, nfa->isAnyEnd = n1->isAnyEnd;
    int sinitState = ssize - 2, sacceptState = ssize - 1;
    for (int ti = 0; ti < t1; ti++) {
        nfa->transVec[ti].start = n1->transVec[ti].start;
        nfa->transVec[ti].end = n1->transVec[ti].end;
        nfa->transVec[ti].c = n1->transVec[ti].c;
        if (n1->transVec[ti].end == sms->start &&
            n1->transVec[ti].c == EPSILON) {
            nfa->transVec[ti].end = sinitState;
        }
        if (n1->transVec[ti].start == sms->end &&
            n1->transVec[ti].c == EPSILON) {
            nfa->transVec[ti].start = sacceptState;
        }
    }
    for (int ti = t1; ti < t1 + t2; ti++) {
        nfa->transVec[ti].start = n2->transVec[ti - t1].start + s1;
        nfa->transVec[ti].end = n2->transVec[ti - t1].end + s1;
        nfa->transVec[ti].c = n2->transVec[ti - t1].c;
    }
    nfa->transVec[tsize - 4].start = sinitState;
    nfa->transVec[tsize - 4].end = sms->start;
    nfa->transVec[tsize - 4].c = EPSILON;
    nfa->transVec[tsize - 3].start = sinitState;
    nfa->transVec[tsize - 3].end = n2->initState + s1;
    nfa->transVec[tsize - 3].c = EPSILON;
    nfa->transVec[tsize - 2].start = sms->end;
    nfa->transVec[tsize - 2].end = sacceptState;
    nfa->transVec[tsize - 2].c = EPSILON;
    nfa->transVec[tsize - 1].start = n2->acceptState + s1;
    nfa->transVec[tsize - 1].end = sacceptState;
    nfa->transVec[tsize - 1].c = EPSILON;

    nfa->subms = addSubMatch(n1->subms);
    nfa->subms->start = n2->initState + s1;
    nfa->subms->end = n2->acceptState + s1;
    nfa->subms->name = sub.c_str();
    nfa->subms->type = TEXT_PT;
    nfa->subms->isAnyStart = n2->isAnyStart;
    nfa->subms->isAnyEnd = n2->isAnyEnd;
    nfa->subms->predID = id;

    delete n1, n2;
    return nfa;
}

SubMatch *QueryOptimizer::addSubMatch(SubMatch *subms) {
    SubMatch *new_sm = new SubMatch;
    new_sm->next = subms;
    return new_sm;
}

NFA *QueryOptimizer::constructPredNFA(OpTree *opt) {
    char *s = (char *)opt->right->constData.p;
    std::string str(s, strlen(s));

    if (opt->opType == EQUAL) {
        std::string pattern = str.substr(1, str.length() - 2);
        int tsize = pattern.length() * 2 - 1;
        NFA *new_nfa = new NFA;
        new_nfa->stateSize = tsize + 1;
        new_nfa->transSize = tsize;
        new_nfa->initState = 0;
        new_nfa->transVec = new Transition[tsize];
        for (int ti = 0; ti < tsize; ti++) {
            new_nfa->transVec[ti].start = ti;
            new_nfa->transVec[ti].end = ti + 1;
            if (ti % 2 == 0) {
                new_nfa->transVec[ti].c = pattern[ti / 2];
            } else {
                new_nfa->transVec[ti].c = EPSILON;
            }
        }
        new_nfa->acceptState = new_nfa->stateSize - 1;
        new_nfa->isAnyStart = false;
        new_nfa->isAnyEnd = false;
        return new_nfa;
    } else if (opt->opType == NEQUAL) {
    } else if (opt->opType == LIKE) {
        std::string pattern;
        for (int si = 1; si < str.length() - 1; si++) {
            if (str[si] == '%') {
                pattern.push_back('.');
                pattern.push_back('*');
            } else {
                pattern.push_back(str[si]);
            }
        }
        NFA *nfa = rparser->parse(pattern);
        return nfa;
    } else if (opt->opType == REGEXP) {
        std::string pattern = str.substr(1, str.length() - 2);
        NFA *nfa = rparser->parse(pattern);
        return nfa;
    } else {
        perror("Invalid operator.");
        return NULL;
    }
}

QueryOptimizer::QueryOptimizer() { rparser = new RegexParser(); }
NFA *QueryOptimizer::optimize(NFA *nfa, QueryContext *ctx) {
    getTextPred(ctx);
    if (count == 0) {
        return nfa;
    }

    NFA *new_nfa = optimizeNFA(nfa, ctx);
    return new_nfa;
}
