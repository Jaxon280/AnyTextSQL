#pragma once

#include "common/common.hpp"
#include "flat_hash_map/flat_hash_map.hpp"
#include "parser/inter/query.hpp"

namespace vlex {

template <typename VType>
class AggregationMapBase {
   public:
    using KType = std::string;
    using HTType = ska::flat_hash_map<KType, VType *>;
    AggregationMapBase(int _valueSize) : valueSize(_valueSize) {}
    ~AggregationMapBase() {
        for (auto itr = hashTable.begin(); itr != hashTable.end(); ++itr) {
            delete itr->second;
        }
    }
    bool find(KType k) const { return hashTable.count(k) > 0; }
    const HTType &getHashTable() const { return hashTable; }

   protected:
    int valueSize;
    ska::flat_hash_map<KType, VType *> hashTable;
};

class AggregationValueMap : public AggregationMapBase<data64> {
    using AggregationMapBase<data64>::AggregationMapBase;
    using VType = data64;

   public:
    void assign(KType k, Aggregation *aggContexts) {
        VType *values = new VType[valueSize];
        hashTable.insert(std::pair<KType, VType *>{k, values});

        // initialization
        for (int vk = 0; vk < valueSize; vk++) {
            AggFuncType ftype = aggContexts[vk].ftype;
            Type ktype = aggContexts[vk].type;
            if (ktype == DOUBLE) {
                if (ftype == MIN) {
                    values[vk].d = DBL_MAX;
                } else if (ftype == MAX) {
                    values[vk].d = -DBL_MAX;
                } else {
                    values[vk].d = 0.0;
                }
            } else if (ktype == INT) {
                if (ftype == MIN) {
                    values[vk].i = INT64_MAX;
                } else if (ftype == MAX) {
                    values[vk].i = INT64_MIN;
                } else {
                    values[vk].i = 0;
                }
            }
        }
    }

    void sumInt(KType k, int64_t value, int vk) {
        hashTable.at(k)[vk].i += value;
    }

    void sumDouble(KType k, double value, int vk) {
        hashTable.at(k)[vk].d += value;
    }

    void maxInt(KType k, int64_t value, int vk) {
        if (value > hashTable.at(k)[vk].i) {
            hashTable.at(k)[vk].i = value;
        }
    }

    void minInt(KType k, int64_t value, int vk) {
        if (value < hashTable.at(k)[vk].i) {
            hashTable.at(k)[vk].i = value;
        }
    }

    void maxDouble(KType k, double value, int vk) {
        if (value > hashTable.at(k)[vk].d) {
            hashTable.at(k)[vk].d = value;
        }
    }

    void minDouble(KType k, double value, int vk) {
        if (value < hashTable.at(k)[vk].d) {
            hashTable.at(k)[vk].d = value;
        }
    }
};

class AggregationCountMap : public AggregationMapBase<int> {
    using AggregationMapBase<int>::AggregationMapBase;
    using VType = int;

   public:
    void assign(KType k) {
        VType *values = new VType[valueSize];
        hashTable.insert(std::pair<KType, VType *>{k, values});

        // initialization
        for (int vk = 0; vk < valueSize; vk++) {
            values[vk] = 0;
        }
    }

    void count(KType k, int vk) { hashTable.at(k)[vk]++; }
};
}  // namespace vlex
