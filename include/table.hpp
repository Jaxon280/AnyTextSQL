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
    KeyMap(NFA *nfa) { constructByRegex(nfa->subms); }
    KeyMap(NFA **keyNFAs, int keySize) {
        std::vector<SubMatch *> smsVec;
        for (int ki = 0; ki < keySize; ki++) {
            for (SubMatch *sms = keyNFAs[ki]->subms; sms != NULL;
                 sms = sms->next) {
                smsVec.push_back(sms);
            }
        }
        constructByKeys(smsVec);
    }
    void constructByRegex(SubMatch *smses) {
        int id = 0;
        for (SubMatch *sms = smses; sms != NULL; sms = sms->next) {
            addKey(sms, id);
            id++;
        }
    }

    void constructByKeys(const std::vector<SubMatch *> &smsVec) {
        for (int sid = 0; sid < (int)smsVec.size(); sid++) {
            addKey(smsVec[sid], sid);
        }
    }

    inline const Key &at(const std::string &name) const { return map.at(name); }
    inline bool find(const std::string &name) const {
        return map.find(name) != map.end();
    }

   private:
    void addKey(const SubMatch *sms, int id) {
        Type keyType;
        if (sms->type == INT_PT) {
            keyType = INT;
        } else if (sms->type == DOUBLE_PT) {
            keyType = DOUBLE;
        } else if (sms->type == TEXT_PT) {
            keyType = TEXT;
        }
        // sms->
        std::string keyName(sms->name, strlen(sms->name));
        Key key(id, keyType);
        map.insert(std::pair<std::string, Key>(keyName, key));
    }
    std::map<std::string, Key> map;
};

class Table {
   public:
    Table(const std::string &name, const std::string &filename, NFA *nfa,
          KeyMap &keyMap)
        : name(name),
          filename(filename),
          nfa(nfa),
          keySize(0),
          keyNFAs(NULL),
          keyMap(keyMap) {}
    Table(const std::string &name, const std::string &filename, int keySize,
          NFA **keyNFAs, KeyMap &keyMap)
        : name(name),
          filename(filename),
          nfa(NULL),
          keySize(keySize),
          keyNFAs(keyNFAs),
          keyMap(keyMap) {}
    inline const KeyMap &getKeyMap() const { return keyMap; }
    inline bool isKeys() const { return keySize > 0 ? true : false; }
    inline NFA *getNFA() const { return nfa; }
    inline int getKeySize() const { return keySize; }
    inline NFA **getKeyNFAs() const { return keyNFAs; }
    inline const std::string &getFilename() const { return filename; }

   private:
    const std::string &name;
    const std::string &filename;
    NFA *nfa;
    int keySize;
    NFA **keyNFAs;
    KeyMap &keyMap;
};

}  // namespace vlex
