#pragma once

#include "common/common.hpp"
#include "common/spark.hpp"
#include "parser/parser.hpp"
#include "parser/inter/command.hpp"
#include "parser/inter/nfa.hpp"
#include "parser/inter/query.hpp"
#include "runner/runtime.hpp"
#include "runner/table.hpp"
#include "scanner/dfa.hpp"
#include "scanner/vfa.hpp"

namespace vlex {

class CommandExecutor {
   public:
    CommandExecutor() {
        cparser = new CommandParser();
        rparser = new RegexParser();
        qparser = new QueryParser();
    }

    CommandContext* parseCommand(const std::string& input) const;
    void execScan(CommandContext *cmd);
    void execWithSpark(const std::string &query, SparkContext *sctx);
    void exec();

   private:
    void initialize() const;
    NFA* constructRegexNFA(NFA* nfa) const;
    void execCommand(CommandContext* ctx);

    double version = 1.0;
    std::string buffer;
    CommandParser* cparser;
    RegexParser* rparser;
    QueryParser* qparser;
    std::map<std::string, Table> tableMap;
};

}  // namespace vlex
