#include "optimizer/optimizer.hpp"

namespace vlex {

void QueryOptimizer::initNFAs() {
    transMap = std::map<int, std::vector<Transition>>();
    splittedNFAs = std::vector<SplitNFA>();
}

void QueryOptimizer::initPreds() {
    count = 0;
    key2textPred = std::map<std::string, std::vector<OpTree *>>();
    textPredKeys = std::set<int>();
}

void QueryOptimizer::getTextPred(QueryContext *ctx) {
    count = 0;
    for (OpTree *opt : ctx->getPList()) {
        if (opt->type == TEXT) {
            key2textPred[opt->left->varKey].push_back(opt);
            opt->textPredId = count + 1;
            count++;
        }
    }
}

void QueryOptimizer::getKeysTextPred(NFA **keyNFAs, int keySize,
                                     QueryContext *ctx) {
    count = 0;
    for (OpTree *opt : ctx->getPList()) {
        if (opt->type == TEXT) {
            key2textPred[opt->left->varKey].push_back(opt);
            opt->textPredId = count + 1;
            count++;
            for (int k = 0; k < keySize; k++) {
                for (SubMatch *sms = keyNFAs[k]->subms; sms != NULL;
                     sms = sms->next) {
                    if (opt->left->varKey ==
                        std::string(sms->name, strlen(sms->name))) {
                        textPredKeys.insert(k);
                    }
                }
            }
        }
    }
}

void QueryOptimizer::splitNFA(NFA *nfa) {
    for (int ti = 0; ti < nfa->transSize; ti++) {
        transMap[nfa->transVec[ti].start].push_back(nfa->transVec[ti]);
    }

    std::map<int, SubMatch *> start2tsms;
    std::map<int, SubMatch *> end2tsms;
    for (SubMatch *sms = nfa->subms; sms != NULL; sms = sms->next) {
        if (sms->type == TEXT_PT) {
            start2tsms[sms->start] = sms;
            end2tsms[sms->end] = sms;
        }
    }

    std::queue<int> bfsQueue;
    bfsQueue.push(nfa->initState);
    std::vector<int> visited(nfa->stateSize);  // splitted id
    splittedNFAs.resize(key2textPred.size() * 2 + 1);
    splittedNFAs[0].start = nfa->initState;
    splittedNFAs[0].tsms = NULL;
    splittedNFAs[0].stateSize = 0;
    visited[nfa->initState] = 1;

    while (!bfsQueue.empty()) {
        int s = bfsQueue.front();
        int sni = visited[s] - 1;
        bfsQueue.pop();
        if (s == nfa->acceptState) {
            splittedNFAs[sni].end = s;
        } else if (start2tsms.count(s) > 0) {
            splittedNFAs[sni].start = s;
            splittedNFAs[sni].tsms = start2tsms[s];
            splittedNFAs[sni].stateSize = 0;
        } else if (end2tsms.count(s) > 0) {
            splittedNFAs[sni].end = s;
        }
        splittedNFAs[sni].stateSize++;

        for (const Transition &t : transMap[s]) {
            if (visited[t.end] == 0) {
                bfsQueue.push(t.end);
                if (start2tsms.count(t.end) > 0) {
                    splittedNFAs[sni].end = s;
                    visited[t.end] = visited[s] + 1;
                } else if (end2tsms.count(s) > 0) {
                    int snni = visited[s] + 1;
                    splittedNFAs[snni - 1].start = t.end;
                    splittedNFAs[snni - 1].tsms = NULL;
                    splittedNFAs[snni - 1].stateSize = 0;
                    visited[t.end] = snni;
                } else {
                    visited[t.end] = visited[s];
                }
            }

            splittedNFAs[sni].transVec.push_back(t);
        }
    }
}

NFA *QueryOptimizer::optimizeNFA(NFA *originalNFA) {
    // set tsize, ssize
    std::map<std::string, std::vector<NFA *>> key2textNFA;
    int ssize = originalNFA->stateSize, tsize = originalNFA->transSize;
    SubMatch *subms = originalNFA->subms;
    for (SplitNFA &snfa : splittedNFAs) {
        if (snfa.tsms != NULL) {
            std::string key(snfa.tsms->name, strlen(snfa.tsms->name));
            for (OpTree *opt : key2textPred[key]) {
                SplitNFA *pNFA = constructPredNFA(
                    opt, snfa.tsms->regex);  // todo: increase text pred
                for (int si = 0; si < pNFA->stateSize; si++) {
                    pNFA->snfas2nfas[si] = ssize + si;
                }
                ssize += pNFA->stateSize;
                tsize += pNFA->transVec.size() + 2;
                snfa.predNFAs.push_back(*pNFA);

                SubMatch *new_sms = new SubMatch(
                    pNFA->snfas2nfas[pNFA->start], pNFA->snfas2nfas[pNFA->end], snfa.tsms->name, TEXT_PT, pNFA->regex);
                new_sms->next = subms;
                subms = new_sms;
            }
        }
    }

    Transition *trans = new Transition[tsize];

    for (int ti = 0; ti < originalNFA->transSize; ti++) {
        trans[ti] = originalNFA->transVec[ti];
    }
    int tss = originalNFA->transSize;
    for (int sni = 0; sni < splittedNFAs.size(); sni++) {
        for (SplitNFA &spnfa : splittedNFAs[sni].predNFAs) {
            for (int ti = 0; ti < spnfa.transVec.size(); ti++) {
                trans[tss + ti] =
                    Transition(spnfa.snfas2nfas[spnfa.transVec[ti].start],
                               spnfa.snfas2nfas[spnfa.transVec[ti].end],
                               spnfa.transVec[ti].c);
            }
            tss += spnfa.transVec.size();
            trans[tss] = Transition(splittedNFAs[sni - 1].end,
                                    spnfa.snfas2nfas[spnfa.start], EPSILON);
            trans[tss + 1] = Transition(spnfa.snfas2nfas[spnfa.end],
                                        splittedNFAs[sni + 1].start, EPSILON);
            tss += 2;
        }
    }

    NFA *nfa = new NFA(subms, trans, tsize, splittedNFAs[0].start,
                       splittedNFAs[splittedNFAs.size() - 1].end, ssize,
                       TEXT_PT, originalNFA->regex);
    return nfa;
}

QueryOptimizer::SplitNFA *QueryOptimizer::constructPredNFA(
    OpTree *opt, const std::string &keyRegex) {
    char *s = (char *)opt->right->constData.p;
    std::string str(s, strlen(s));

    if (opt->opType == EQUAL) {
        std::string pattern = str.substr(1, str.length() - 2);
        int tsize = pattern.length() * 2 - 1;
        SplitNFA *new_nfa = new SplitNFA;
        new_nfa->stateSize = tsize + 1;
        new_nfa->start = 0;
        new_nfa->transVec.resize(tsize);
        for (int ti = 0; ti < tsize; ti++) {
            new_nfa->transVec[ti].start = ti;
            new_nfa->transVec[ti].end = ti + 1;
            if (ti % 2 == 0) {
                new_nfa->transVec[ti].c = pattern[ti / 2];
            } else {
                new_nfa->transVec[ti].c = EPSILON;
            }
        }
        new_nfa->end = new_nfa->stateSize - 1;
        return new_nfa;
    } else if (opt->opType == NEQUAL) {
        // todo
        return NULL;
    } else if (opt->opType == LIKE) {
        std::string pattern;
        for (int si = 1; si < (int)str.length() - 1; si++) {
            if (str[si] == '%') {
                // todo: + -> *
                pattern += keyRegex;
            } else {
                pattern.push_back(str[si]);
            }
        }
        NFA *nfa = rparser->parse(pattern);
        if (nfa == NULL) {
            perror("failed to parse regular expression.");
        }
        SplitNFA *snfa =
            new SplitNFA(nfa->initState, nfa->acceptState, nfa->stateSize,
                         nfa->transVec, nfa->transSize, nfa->regex);
        return snfa;
    } else if (opt->opType == REGEXP) {
        std::string pattern = str.substr(1, str.length() - 2);
        NFA *nfa = rparser->parse(pattern);
        SplitNFA *snfa =
            new SplitNFA(nfa->initState, nfa->acceptState, nfa->stateSize,
                         nfa->transVec, nfa->transSize, nfa->regex);
        return snfa;
    } else {
        perror("Invalid operator.");
        return NULL;
    }
}

QueryOptimizer::QueryOptimizer() { rparser = new RegexParser(); }

void QueryOptimizer::initialize() {
    initNFAs();
    initPreds();
}

NFA *QueryOptimizer::optimize(NFA *nfa, QueryContext *ctx) {
    getTextPred(ctx);
    if (count == 0) {
        return nfa;
    }
    splitNFA(nfa);
    NFA *new_nfa = optimizeNFA(nfa);
    return new_nfa;
}

NFA **QueryOptimizer::optimize(NFA **keyNFAs, int keySize, QueryContext *ctx) {
    getKeysTextPred(keyNFAs, keySize, ctx);
    if (count == 0) {
        return keyNFAs;
    }

    NFA **newKeyNFAs = new NFA *[keySize];
    for (int k = 0; k < keySize; k++) {
        if (textPredKeys.find(k) != textPredKeys.end()) {
            initNFAs();
            splitNFA(keyNFAs[k]);
            newKeyNFAs[k] = optimizeNFA(keyNFAs[k]);
            if (newKeyNFAs[k] == NULL) {
                return keyNFAs;
            }
        } else {
            newKeyNFAs[k] = keyNFAs[k];
        }
    }
    delete keyNFAs;
    return newKeyNFAs;
}

}  // namespace vlex
