#include "command.hpp"

using namespace vlex;

void CommandExecutor::initialize() {
    std::cout << "Welcome to AnyDB " << version << std::endl;
    std::cout << "Main commands are:" << std::endl;
    // std::cout
    //     << "\".import <filename> <tablename>\": import your raw data
    //     file."
    //     << std::endl;
    std::cout << "\".scan <filename> -t <tablename> -e <pattern>\": give "
                 "the regular "
                 "expression to scan file."
              << std::endl;
    std::cout
        << "You can execute your query on the raw data by FROM <tablename>"
        << std::endl;
    std::cout << "Type \"help\" for more information." << std::endl;
}

void CommandExecutor::execCommand(Command* cmd) {
    if (cmd->type == SCAN) {
        int result = rparser.parse(cmd->args[5]);
        if (result) {
            NFA* nfa = rparser.getNFA();
            KeyMap* keyMap = new KeyMap(nfa->subms);
            tableMap.insert(std::pair<std::string, Table>(
                cmd->args[3], Table(cmd->args[3], cmd->args[1], nfa, keyMap)));
        } else {
            return;
        }
    } else if (cmd->type == EXEC) {
        int result = qparser.parse(cmd->args[0]);
        if (result) {
            QueryContext* ctx = qparser.getContext();
            for (StringList* tbn = ctx->getTables(); tbn != NULL;
                 tbn = tbn->next) {
                std::string s(tbn->str, strlen(tbn->str));
                Table& table = tableMap.find(s)->second;
                ctx->mapping(table.getKeyMap());
                Runtime runtime(table, ctx);
                runtime.construct(0.0);
                runtime.exec();
            }
        } else {
            return;
        }
    }
}

void CommandExecutor::exec() {
    initialize();
    while (true) {
        std::cout << ">>> ";
        if (!std::getline(std::cin, buffer)) {
            return;
        }
        Command* cmd = parseCommand(buffer);
        if (cmd == NULL) {
            std::cout << "Reenter the command" << std::endl;
            continue;
        }
        execCommand(cmd);
    }
}
