#pragma once

#include "common.hpp"
#include "executor/executor.hpp"
#include "general/ios.hpp"
#include "interface.hpp"
#include "parser/nfa.hpp"
#include "parser/query.hpp"
#include "scanner/dfa.hpp"
#include "scanner/vfa.hpp"

#define PARTITION_SIZE (SIZE_TYPE)(1 << 31)  // 1024MB

namespace vlex {
class RuntimeBase {
   public:
    RuntimeBase(const Table& table);

   protected:
    void makePartitions(SIZE_TYPE size);
    DFA* mergeDFAs(const DFA* rDFA, const DFA* sDFA);
    DFA::StateSet createAcceptStates(
        const std::map<int, std::vector<int>>& transMap,
        const DFA::StateSet& acceptStates, const std::vector<int>& old2new,
        int ssize);
    DFA::SubMatches createSubMatches(const DFA::SubMatches& subms,
                                     const std::vector<int>& old2new,
                                     int ssize);
    DFA::TransTable createTransTable(
        const std::map<int, std::vector<int>>& transMap, int numStates,
        const std::vector<int>& old2new, int ssize);
    std::vector<int> createStateMap(
        const std::map<int, std::vector<int>>& transMap, int ssize);

    Executor* executor;
    ioStream* ios;
    DFAGenerator* dfag;
    const Table& table;

    DATA_TYPE* data;
    SIZE_TYPE size;
    std::vector<SIZE_TYPE> partitions;
};

class RuntimeExpression : public RuntimeBase {
    using RuntimeBase::RuntimeBase;

   public:
    void constructDFA(const NFA* nfa, const NFA* regexNFA);
    void constructVFA(double lr);
    void iexec(QueryContext* query);
    void exec(QueryContext* query);

   private:
    DFA* dfa;
    VectFA* vfa;
};

class RuntimeKeys : public RuntimeBase {
    using RuntimeBase::RuntimeBase;

   public:
    void constructDFAs(const NFA** keyNFAs, const NFA** keyRegexNFAs);
    void constructVFAs(double lr);
    void iexec(QueryContext* query);
    void exec(QueryContext* query);

   private:
    int keySize;
    DFA** dfa;
    VectFA** keyVFAs;
};
}  // namespace vlex
