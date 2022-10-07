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
    std::cout << "\".scan <filename> -t <tablename> -k [<pattern>,]\": give "
                 "the set of regular "
                 "expressions to scan file."
              << std::endl;
    std::cout
        << "You can execute your query on the raw data by FROM <tablename>"
        << std::endl;
    std::cout << "Type \"help\" for more information." << std::endl;
}

CommandContext* CommandExecutor::parseCommand(const std::string& input) const {
    auto pos = input.find(' ');
    if (pos != std::string::npos) {
        if (input.substr(0, pos) == ".scan" && pos < input.length() - 1) {
            CommandContext* ctx = new CommandContext(SCAN);
            cparser->parse(input, ctx);
            if (ctx->isError()) {
                delete ctx;
                return NULL;
            }
            return ctx;
        } else {
            CommandContext* ctx = new CommandContext(input);
            return ctx;
        }
    } else {
        perror("Enter the command.\n");
        return NULL;
    }
}

NFA* CommandExecutor::constructRegexNFA(NFA* nfa) const {
    NFA* nfa2 = copyNFA(nfa);
    NFA* n1 = buildWildcardNFA();
    NFA* n2 = buildStarNFA(n1);
    NFA* regexNFA = buildConcatNFA(n2, nfa2);
    return regexNFA;
}

void CommandExecutor::execCommand(CommandContext* cmd) {
    if (cmd->getMode() == SCAN) {
        if (cmd->isKeys()) {
            int ksize = cmd->getPatternKeys().size();
            NFA** keyNFAs = new NFA*[ksize];
            NFA** keyRegexNFAs = new NFA*[ksize];
            for (int pi = 0; pi < ksize; pi++) {
                keyNFAs[pi] = rparser->parse(cmd->getPatternKeys()[pi]);
                if (keyNFAs[pi] != NULL) {
                    keyRegexNFAs[pi] = constructRegexNFA(keyNFAs[pi]);
                } else {
                    delete keyNFAs;
                    return;
                }
            }
            KeyMap* keyMap = new KeyMap(keyNFAs, ksize);
            tableMap.insert(std::pair<std::string, Table>(
                cmd->getTablename(),
                Table(cmd->getTablename(), cmd->getFilename(), ksize, keyNFAs,
                      keyRegexNFAs, *keyMap)));
        } else {
            NFA* nfa = rparser->parse(cmd->getPattern());
            if (nfa != NULL) {
                KeyMap* keyMap = new KeyMap(nfa);
                NFA* regexNFA = constructRegexNFA(nfa);
                tableMap.insert(std::pair<std::string, Table>(
                    cmd->getTablename(),
                    Table(cmd->getTablename(), cmd->getFilename(), nfa,
                          regexNFA, *keyMap)));
            } else {
                return;
            }
        }
    } else if (cmd->getMode() == EXEC) {
        QueryContext* ctx = qparser->parse(cmd->getQuery());
        if (ctx != NULL) {
            for (const StringList* tbn = ctx->getTables(); tbn != NULL;
                 tbn = tbn->next) {
                std::string s(tbn->str, strlen(tbn->str));
                Table& table = tableMap.find(s)->second;
                ctx->mapping(table.getKeyMap());
                if (table.isKeys()) {
                    NFA** keyNFAs = table.getKeyNFAs();
                    NFA** keyRegexNFAs = table.getKeyRegexNFAs();

                    RuntimeKeys runtime(table);
                    runtime.constructDFA(keyNFAs, keyRegexNFAs);
                    runtime.constructVFA(0.0);
                    runtime.exec(ctx);
                } else {
                    NFA* nfa = table.getNFA();
                    NFA* regexNFA = table.getRegexNFA();
                    // NFA* nfa = qopter->optimize(table.getNFA(), ctx);
                    RuntimeExpression runtime(table);
                    runtime.constructDFA(nfa, regexNFA);
                    runtime.constructVFA(0.0);
                    runtime.exec(ctx);
                }
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
        CommandContext* cmd = parseCommand(buffer);
        if (cmd == NULL) {
            std::cout << "Reenter the command" << std::endl;
            continue;
        }
        execCommand(cmd);
    }
}

}  // namespace vlex
