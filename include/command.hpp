#pragma once

#include "common.hpp"
#include "interface.hpp"
#include "parser/nfa.hpp"
#include "parser/parser.hpp"
#include "parser/query.hpp"
#include "runtime.hpp"
#include "scanner/converter.hpp"
#include "scanner/dfa.hpp"

namespace vlex {

class CommandExecutor {
   private:
    typedef enum _action_type { SCAN, EXEC } CommandType;
    struct Command {
        CommandType type;
        std::vector<std::string> args;
    };

    double version = 1.0;

    std::string buffer;
    RegexParser rparser;
    QueryParser qparser;
    std::map<std::string, Table> tableMap;

    void initialize();

    Command* parseScan(const std::string& input) {
        Command* cmd = new Command();
        cmd->type = SCAN;
        std::stringstream ss(input);
        std::string buf;
        while (ss >> buf) {
            cmd->args.push_back(buf);
        }
        return cmd;
    }

    Command* parseExec(const std::string& input) {
        Command* cmd = new Command();
        cmd->type = EXEC;
        cmd->args.push_back(input);
        return cmd;
    }

    Command* parseCommand(const std::string& input) {
        int pos = input.find(' ');
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

   public:
    CommandExecutor() {
        rparser = RegexParser();
        qparser = QueryParser();
    }
    void exec();
};

}  // namespace vlex
