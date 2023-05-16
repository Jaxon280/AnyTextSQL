#pragma once

#include "common.hpp"

const uint8_t nmask = 63;

class BitSet {
   public:
    BitSet() { reset(); }
    inline bool find(const uint8_t n) const {
        int nn = n & nmask;
        unsigned long long b = (1ULL << nn);
        return ((data & b) != 0ULL) ? true : false;
    }
    inline void add(const uint8_t n) {
        int nn = n & nmask;
        data |= (1ULL << nn);
    }
    inline void remove(const uint8_t n) {
        int nn = n & nmask;
        data &= (UINT64_MAX - (1ULL << nn));
    }
    inline void reset() {
        data = 0ULL;
    }

   private:
    unsigned long long data;
};
