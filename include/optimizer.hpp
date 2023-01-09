#pragma once

#include "common.hpp"
#include "table.hpp"
#include "parser/nfa.hpp"
#include "parser/parser.hpp"
#include "parser/query.hpp"

namespace vlex {
class QueryOptimizer {
   public:
    QueryOptimizer();
    void initialize();
    NFA *optimize(NFA *nfa, QueryContext *ctx);
    NFA **optimize(NFA **keyNFAs, int keySize, QueryContext *ctx);

   private:
    struct SplitNFA {
        int start;
        int end;
        SubMatch *tsms;
        int stateSize;
        std::vector<Transition> transVec;
        std::string regex;
        std::map<int, int> snfas2nfas;
        std::vector<SplitNFA> predNFAs;
        SplitNFA(int _start, int _end, int _stateSize, Transition *_transVec,
                 int _transSize, std::string _regex)
            : start(_start), end(_end), stateSize(_stateSize), regex(_regex) {
            transVec.resize(_transSize);
            for (int i = 0; i < _transSize; i++) {
                transVec[i] = _transVec[i];
            }
            delete _transVec;
        }
        SplitNFA() {}
    };

    void initNFAs();
    void initPreds();
    void getTextPred(QueryContext *ctx);
    void getKeysTextPred(NFA **keyNFAs, int keySize, QueryContext *ctx);
    NFA *optimizeNFA(NFA *originalNFA);
    void splitNFA(NFA *nfa);
    NFA *mergeNFA(NFA *n1, NFA *n2, const std::string &sub, int id);
    SplitNFA *constructPredNFA(OpTree *opt, const std::string &keyRegex);

    RegexParser *rparser;
    std::map<int, std::vector<Transition>> transMap;
    std::vector<SplitNFA> splittedNFAs;
    int count = 0;
    std::map<std::string, std::vector<OpTree *>> key2textPred;
    std::set<int> textPredKeys;
};
}  // namespace vlex
