#pragma once

#include "common.hpp"
#include "interface.hpp"
#include "optimizer.hpp"
#include "parser/command.hpp"
#include "parser/nfa.hpp"
#include "parser/parser.hpp"
#include "parser/query.hpp"
#include "runtime.hpp"
#include "scanner/dfa.hpp"
#include "scanner/vfa.hpp"

namespace vlex {

class CommandExecutor {
   public:
    CommandExecutor() {
        cparser = new CommandParser();
        rparser = new RegexParser();
        qparser = new QueryParser();
        qopter = new QueryOptimizer();
    }
    void exec();

   private:
    void initialize() const;
    NFA* constructRegexNFA(NFA* nfa) const;
    CommandContext* parseCommand(const std::string& input) const;
    void execCommand(CommandContext* ctx);

    double version = 1.0;
    std::string buffer;
    CommandParser* cparser;
    RegexParser* rparser;
    QueryParser* qparser;
    QueryOptimizer* qopter;
    std::map<std::string, Table> tableMap;
};

}  // namespace vlex
