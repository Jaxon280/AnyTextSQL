#pragma once

#include "common.hpp"
#include "parser/nfa.hpp"
#include "parser/parser.hpp"
#include "parser/query.hpp"

namespace vlex {
class QueryOptimizer {
   public:
    QueryOptimizer();
    NFA *optimize(NFA *nfa, QueryContext *ctx);

   private:
    void getTextPred(QueryContext *ctx);
    NFA *optimizeNFA(NFA *originalNFA);
    NFA *mergeNFA(NFA *n1, NFA *n2, const std::string &sub, int id);
    NFA *constructPredNFA(OpTree *opt);

    int nbitmap;
    RegexParser *rparser;
    int count = 0;
    std::map<std::string, std::vector<OpTree *>> key2textPred;
};
}  // namespace vlex
