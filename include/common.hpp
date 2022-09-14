#pragma once

#include <fcntl.h>
#include <float.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <iterator>
#include <list>
#include <map>
#include <memory>
#include <queue>
#include <regex>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

#include "x86intrin.h"

#define DFA_ST_TYPE int
#define ST_TYPE uint8_t
#define SIMD_TEXTTYPE __m128i
#define SIMD_BYTES 16
#define DATA_TYPE uint8_t
#define SIZE_TYPE uint32_t
#define MAX_64BYTE (int64_t)0xffffffffffffffff

#define SIMD_128iTYPE __m128i
#define SIMD_256TYPE __m256
#define SIMD_256iuTYPE __m256i_u
#define SIMD_256iTYPE __m256i
#define SIMD_256dTYPE __m256d
#define SIMD_512iuTYPE __m512i_u
#define SIMD_512dTYPE __m512d
#define SIMD_512iTYPE __m512i

#define VECEX_BYTE 16
#define VECEX_BYTE4 VECEX_BYTE >> 2
#define VECEX_BYTE8 VECEX_BYTE >> 3
#define VECEX_BYTE16 VECEX_BYTE >> 4
#define VECEX_BYTE_PROB (double)1 / (double)VECEX_BYTE
#define VECEX_SIZE 512  // AVX-512
#define VECEX_TYPE __m256
#define VECEX_TYPEI __m256i
#define VECEX_TYPED __m256d

union data64 {
    int64_t i;
    double d;
    DATA_TYPE *p;
};

#define INV_STATE 0
#define INIT_STATE 1

#define ASCII_START 32
#define ASCII_END 126
#define ASCII_SZ 128
#define UNICODE_SZ 256
