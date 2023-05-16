#include "parser/cmd/command-node.hpp"

StringList *buildPatterns(StringList *idents, const char *ident) {
    StringList *idList = new StringList;
    idList->str = ident;
    idList->next = idents;
    return idList;
}
