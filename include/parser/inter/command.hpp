#pragma once

#include "common/common.hpp"
#include "common/string-list.hpp"

namespace vlex {
typedef enum _command_mode { SCAN, EXEC } CommandMode;

class CommandContext {
   public:
    CommandContext() {}
    CommandContext(CommandMode _mode) : mode(_mode) {}
    CommandContext(std::string _query) : mode(EXEC), query(_query) {}
    inline CommandMode getMode() const { return mode; }
    inline const std::string &getFilename() const { return filename; }
    inline const std::string &getTablename() const { return tablename; }
    inline const std::string &getPattern() const { return pattern; }
    inline const std::vector<std::string> &getPatternKeys() const {
        return patternKeys;
    }
    inline bool isKeys() const { return patternKeys.size() > 0 ? true : false; }
    inline const std::string &getQuery() const { return query; }
    inline bool isError() const { return errorno; }
    void assignFilename(const char *str) {
        filename = std::string(str, strlen(str));
    }
    void assignTable(const char *str) {
        tablename = std::string(str, strlen(str));
    }
    void assignExp(const char *str) {
        std::string exp(str, strlen(str));
        pattern = exp.substr(1, exp.length() - 2);
    }
    void assignKeys(StringList *slist) {
        std::stack<std::string> sstack;
        for (StringList *sl = slist; sl != NULL; sl = sl->next) {
            std::string exp(sl->str, strlen(sl->str));
            sstack.push(exp.substr(1, exp.length() - 2));
        }
        while (!sstack.empty()) {
            std::string k = sstack.top();
            patternKeys.push_back(k);
            sstack.pop();
        }
    }
    void assignQuery(std::string _query) {
        mode = EXEC;
        query = _query;
    }
    void grammarError(const char *s) {
        printf("ERROR: %s\n\n", s);
        errorno = 1;
    }

   private:
    CommandMode mode;
    std::string filename;
    std::string tablename;
    std::string pattern;
    std::vector<std::string> patternKeys;
    std::string query;
    int errorno = 0;
};
}  // namespace vlex
