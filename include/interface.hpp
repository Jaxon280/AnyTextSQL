#pragma once

#include "common.hpp"
#include "parser/nfa.hpp"
#include "types.hpp"

namespace vlex {

struct Key {
    int id;
    Type type;

    Key() {}
    Key(int _id, Type _type) : id(_id), type(_type) {}
};

class KeyMap {
   public:
    KeyMap() {}
    KeyMap(SubMatch *smses) { construct(smses); }
    void construct(SubMatch *smses) {
        int id = 0;
        for (SubMatch *sms = smses; sms != NULL; sms = sms->next) {
            Type keyType;
            if (sms->type == INT_PT) {
                keyType = INT;
            } else if (sms->type == DOUBLE_PT) {
                keyType = DOUBLE;
            } else if (sms->type == TEXT_PT) {
                keyType = TEXT;
            }
            std::string keyName(sms->name, strlen(sms->name));
            Key key(id, keyType);
            map.insert(std::pair<std::string, Key>(keyName, key));
            id++;
        }
    }

    inline const Key &at(const std::string &name) const { return map.at(name); }
    inline bool find(const std::string &name) const {
        return map.find(name) != map.end();
    }

   private:
    std::map<std::string, Key> map;
};

class Table {
   public:
    Table(std::string &name, std::string &filename, NFA *nfa, KeyMap &keyMap)
        : name(name), filename(filename), nfa(nfa), keyMap(keyMap) {}
    inline const KeyMap &getKeyMap() const { return keyMap; }
    inline NFA *getNFA() const { return nfa; }
    inline const std::string &getFilename() const { return filename; }

   private:
    std::string &name;
    std::string &filename;
    NFA *nfa;
    KeyMap &keyMap;
};

}  // namespace vlex
