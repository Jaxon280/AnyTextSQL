#pragma once

#include "common.hpp"

class BitSet {
   public:
    BitSet() { reset(); }
    inline bool find(int n) const {
        uint32_t b = (1UL << (n - 1));
        return ((bit & b) != 0UL) ? true : false;
    }
    inline bool find(uint32_t n) const {
        return (bit & n) > 0UL ? true : false;
    }
    inline void add(int n) { bit |= (1UL << (n - 1)); }
    inline void add(uint32_t n) { bit |= n; }
    inline void reset() { bit = 0UL; }

   private:
    uint32_t bit;
};
