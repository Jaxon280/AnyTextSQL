#pragma once

#include "common.hpp"
#include "parser/nfa.hpp"
#include "parser/parser.hpp"
#include "parser/query.hpp"

namespace vlex {
class QueryOptimizer {
   private:
    int nbitmap;
    RegexParser *rparser;
    int count = 0;
    std::map<std::string, std::vector<OpTree *>> map;

    void getTextPred(QueryContext *ctx);
    NFA *optimizeNFA(NFA *originalNFA, QueryContext *ctx);
    NFA *mergeNFA(NFA *n1, NFA *n2, const std::string &sub, int id);
    SubMatch *addSubMatch(SubMatch *subms);
    NFA *constructPredNFA(OpTree *opt);

   public:
    QueryOptimizer();
    NFA *optimize(NFA *nfa, QueryContext *ctx);
};
}  // namespace vlex
