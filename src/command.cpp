#include "command.hpp"

namespace vlex {

void CommandExecutor::initialize() const {
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
        NFA* nfa = rparser->parse(cmd->args[5]);
        if (nfa != NULL) {
            KeyMap* keyMap = new KeyMap(nfa->subms);
            tableMap.insert(std::pair<std::string, Table>(
                cmd->args[3], Table(cmd->args[3], cmd->args[1], nfa, *keyMap)));
        } else {
            return;
        }
    } else if (cmd->type == EXEC) {
        QueryContext* ctx = qparser->parse(cmd->args[0]);
        if (ctx != NULL) {
            for (const StringList* tbn = ctx->getTables(); tbn != NULL;
                 tbn = tbn->next) {
                std::string s(tbn->str, strlen(tbn->str));
                Table& table = tableMap.find(s)->second;
                ctx->mapping(table.getKeyMap());
                NFA* nfa = qopter->optimize(table.getNFA(), ctx);
                Runtime runtime(table, nfa, ctx);
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

}  // namespace vlex
