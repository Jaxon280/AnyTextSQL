#pragma once

#include "common.hpp"
#include "flat_hash_map/flat_hash_map.hpp"
#include "query.hpp"

namespace vlex {

template <typename VType>
class AggregationMapBase {
   protected:
    using KType = std::string;
    int valueSize;
    ska::flat_hash_map<KType, VType *> hashTable;

   public:
    using HTType = ska::flat_hash_map<KType, VType *>;

    AggregationMapBase(int _valueSize) : valueSize(_valueSize) {}
    ~AggregationMapBase() {
        for (auto itr = hashTable.begin(); itr != hashTable.end(); ++itr) {
            delete itr->second;
        }
    }

    bool find(KType k) { return hashTable.count(k) > 0; }

    HTType &getHashTable() { return hashTable; }
};

class AggregationValueMap : public AggregationMapBase<data64> {
    using AggregationMapBase<data64>::AggregationMapBase;
    using VType = data64;

   public:
    void assign(KType k, QueryContext::Aggregation *aggContext) {
        VType *values = new VType[valueSize];
        hashTable.insert(std::pair<KType, VType *>{k, values});

        // initialization
        for (int vk = 0; vk < valueSize; vk++) {
            AggFuncType ftype = aggContext->valueKeys[vk].ftype;
            Type ktype = aggContext->valueKeys[vk].ktype;
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
    void assign(KType k, QueryContext::Aggregation *aggContext) {
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
