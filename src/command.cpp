#include "runner/command.hpp"

namespace vlex {

void CommandExecutor::initialize() const {
    std::cout << "Welcome to AnyDB " << version << std::endl;
    std::cout << "Main commands are:" << std::endl;
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
            for (int pi = 0; pi < ksize; pi++) {
                keyNFAs[pi] = rparser->parse(cmd->getPatternKeys()[pi]);
            }
            KeyMap* keyMap = new KeyMap(keyNFAs, ksize);
            tableMap.insert(std::pair<std::string, Table>(
                cmd->getTablename(),
                Table(cmd->getTablename(), cmd->getFilename(), ksize, keyNFAs,
                      *keyMap)));
        } else {
            NFA* nfa = rparser->parse(cmd->getPattern());
            if (nfa != NULL) {
                KeyMap* keyMap = new KeyMap(nfa);
                tableMap.insert(std::pair<std::string, Table>(
                    cmd->getTablename(),
                    Table(cmd->getTablename(), cmd->getFilename(), nfa,
                          *keyMap)));
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
                    NFA** keyRegexNFAs = new NFA*[table.getKeySize()];
                    for (int k = 0; k < table.getKeySize(); k++) {
                        keyRegexNFAs[k] = constructRegexNFA(keyNFAs[k]);
                    }
                    RuntimeKeys runtime(table);
                    runtime.constructDFA(keyNFAs, keyRegexNFAs);
                    runtime.constructVFA(0.0002);
                    runtime.exec(ctx);
                } else {
                    NFA* nfa = table.getNFA();
                    NFA* regexNFA = constructRegexNFA(nfa);
                    RuntimeExpression runtime(table);
                    runtime.constructDFA(nfa, regexNFA);
                    runtime.constructVFA(0.0002);
                    runtime.exec(ctx);
                }
            }
        } else {
            return;
        }
    }
}


void CommandExecutor::execScan(CommandContext *cmd) {
    if (cmd->isKeys()) {
        int ksize = cmd->getPatternKeys().size();
        NFA** keyNFAs = new NFA*[ksize];
        for (int pi = 0; pi < ksize; pi++) {
            keyNFAs[pi] = rparser->parse(cmd->getPatternKeys()[pi]);
        }
        KeyMap* keyMap = new KeyMap(keyNFAs, ksize);
        tableMap.insert(std::pair<std::string, Table>(
            cmd->getTablename(),
            Table(cmd->getTablename(), cmd->getFilename(), ksize, keyNFAs,
                    *keyMap)));
    } else {
        NFA* nfa = rparser->parse(cmd->getPattern());
        if (nfa != NULL) {
            KeyMap* keyMap = new KeyMap(nfa);
            std::cout << nfa->regex << std::endl;
            tableMap.insert(std::pair<std::string, Table>(
                cmd->getTablename(),
                Table(cmd->getTablename(), cmd->getFilename(), nfa,
                        *keyMap)));
        } else {
            return;
        }
    }
}

void constructSparkSchema(const Table &tbl, SparkContext *sctx) {
    const int stringBytes = 8;
    int voffset = 8 + sctx->colSize * stringBytes; // todo: if col size is over 64...
    const int stringSize = stringBytes * 8; // TODO: get max string size
    int ci = 0;
    for (const auto &[s, k] : tbl.getKeyMap().getKeyMap()) {
        if (k.type == TEXT) {
            if (ci >= sctx->varSize) {
                printf("Too many var columns.");
                return;
            }
            sctx->varCols[ci].size = stringSize;
            sctx->varCols[ci].offset = voffset;
            voffset += stringSize;
            ci++;
        }
    }
}

void CommandExecutor::execWithSpark(const std::string &query, SparkContext *sctx) {
    QueryContext* ctx = qparser->parse(query);
    if (ctx != NULL) {
        for (const StringList* tbn = ctx->getTables(); tbn != NULL;
                tbn = tbn->next) {
            std::string s(tbn->str, strlen(tbn->str));
            Table& table = tableMap.find(s)->second;
            ctx->mapping(table.getKeyMap());        
            if (table.isKeys()) {
                NFA** keyNFAs = table.getKeyNFAs();
                constructSparkSchema(table, sctx);
                NFA** keyRegexNFAs = new NFA*[table.getKeySize()];
                for (int k = 0; k < table.getKeySize(); k++) {
                    keyRegexNFAs[k] = constructRegexNFA(keyNFAs[k]);
                }
                RuntimeKeys runtime(table);
                runtime.constructDFA(keyNFAs, keyRegexNFAs);
                runtime.constructVFA(0.0002);
                if (runtime.isInterleaved()) {
                    runtime.iexecWithSpark(ctx, sctx);
                } else {
                    runtime.execWithSpark(ctx, sctx);
                }
            } else {
                NFA *nfa = table.getNFA();
                constructSparkSchema(table, sctx);
                NFA* regexNFA = constructRegexNFA(nfa);
                RuntimeExpression runtime(table);
                runtime.constructDFA(nfa, regexNFA);
                runtime.constructVFA(0.0002);
                if (runtime.isInterleaved()) {
                    runtime.iexecWithSpark(ctx, sctx);
                } else {
                    runtime.execWithSpark(ctx, sctx);
                }
            }
        }
    } else {
        return;
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
