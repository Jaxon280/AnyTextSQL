#pragma once

#include "common/common.hpp"
#include "parser/cmd/command-node.hpp"
#include "parser/cmd/command-parser.hpp"
#include "parser/query/query-node.hpp"
#include "parser/query/query-parser.hpp"
#include "parser/regex/regex-node.hpp"
#include "parser/regex/regex-parser.hpp"

typedef struct yy_buffer_state *YY_BUFFER_STATE;
typedef size_t yy_size_t;

extern YY_BUFFER_STATE command_yy_scan_string(const char *yystr);
extern void command_yy_delete_buffer(YY_BUFFER_STATE buffer);

extern YY_BUFFER_STATE query_yy_scan_string(const char *yystr);
extern void query_yy_delete_buffer(YY_BUFFER_STATE buffer);

extern YY_BUFFER_STATE regex_yy_scan_string(const char *yystr);
extern void regex_yy_delete_buffer(YY_BUFFER_STATE buffer);

namespace vlex {
class Parser {
   public:
    Parser() {}
};

class CommandParser : public Parser {
    using Parser::Parser;

   public:
    void parse(const std::string &input, CommandContext *ctx) {
        YY_BUFFER_STATE buffer = command_yy_scan_string(input.c_str());
        command_yyparse(*ctx);
        command_yy_delete_buffer(buffer);
    }
};

class RegexParser : public Parser {
    using Parser::Parser;

   public:
    NFA *parse(const std::string &input) {
        NFA *nfa = NULL;
        YY_BUFFER_STATE buffer = regex_yy_scan_string(input.c_str());
        regex_yyparse(&nfa);
        regex_yy_delete_buffer(buffer);
        return nfa;
    }
};

class QueryParser : public Parser {
    using Parser::Parser;

   public:
    QueryContext *parse(const std::string &input) {
        YY_BUFFER_STATE buffer = query_yy_scan_string(input.c_str());
        QueryContext *ctx = new QueryContext();
        query_yyparse(*ctx);
        query_yy_delete_buffer(buffer);
        if (ctx->isError()) {
            delete ctx;
            return NULL;
        } else {
            return ctx;
        }
    }
};
}  // namespace vlex
