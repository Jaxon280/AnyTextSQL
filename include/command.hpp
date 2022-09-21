#pragma once

#include "common.hpp"
#include "interface.hpp"
#include "optimizer.hpp"
#include "parser/nfa.hpp"
#include "parser/parser.hpp"
#include "parser/query.hpp"
#include "runtime.hpp"
#include "scanner/converter.hpp"
#include "scanner/dfa.hpp"

namespace vlex {

class CommandExecutor {
   public:
    CommandExecutor() {
        rparser = new RegexParser();
        qparser = new QueryParser();
        qopter = new QueryOptimizer();
    }
    void exec();

   private:
    typedef enum _action_type { SCAN, EXEC } CommandType;
    struct Command {
        CommandType type;
        std::vector<std::string> args;
    };

    void initialize() const;

    Command* parseScan(const std::string& input) const {
        Command* cmd = new Command();
        cmd->type = SCAN;
        std::stringstream ss(input);
        std::string buf;
        while (ss >> buf) {
            cmd->args.push_back(buf);
        }
        return cmd;
    }

    Command* parseExec(const std::string& input) const {
        Command* cmd = new Command();
        cmd->type = EXEC;
        cmd->args.push_back(input);
        return cmd;
    }

    Command* parseCommand(const std::string& input) const {
        auto pos = input.find(' ');
        if (pos != std::string::npos) {
            if (input.substr(0, pos) == ".scan" && pos < input.length() - 1) {
                return parseScan(input);
            } else {
                return parseExec(input);
            }
        } else {
            perror("Enter the command.\n");
            return NULL;
        }
    }
    void execCommand(Command* cmd);

    double version = 1.0;
    std::string buffer;
    RegexParser* rparser;
    QueryParser* qparser;
    QueryOptimizer* qopter;
    std::map<std::string, Table> tableMap;
};

}  // namespace vlex
