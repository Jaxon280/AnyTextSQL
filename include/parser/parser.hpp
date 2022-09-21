#pragma once

#include "common.hpp"
#include "parser/query/queryParser.hpp"
#include "parser/query/queryTree.hpp"
#include "parser/regex/regexParser.hpp"
#include "parser/regex/regexTree.hpp"

typedef struct yy_buffer_state *YY_BUFFER_STATE;
typedef size_t yy_size_t;

extern YY_BUFFER_STATE query_yy_scan_string(const char *yystr);
extern void query_yy_delete_buffer(YY_BUFFER_STATE buffer);

extern YY_BUFFER_STATE regex_yy_scan_string(const char *yystr);
extern void regex_yy_delete_buffer(YY_BUFFER_STATE buffer);

namespace vlex {
class Parser {
   public:
    Parser() {}

    void error(const char *s) {
        printf("ERROR: %s\n\n", s);
        errorno = 1;
    }
    inline bool isError() const {
        if (errorno) {
            return true;
        } else {
            return false;
        }
    }

   private:
    int errorno = 0;
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
