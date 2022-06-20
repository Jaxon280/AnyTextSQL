#pragma once

#include <fcntl.h>
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
#include <map>
#include <queue>
#include <set>
#include <stack>
#include <string>
#include <utility>
#include <vector>

#include "x86intrin.h"

#define ST_TYPE uint8_t
#define SIMD_TYPE __m128i
#define SIMD_BYTES 16
#define DATA_TYPE uint8_t
#define SIZE_TYPE uint64_t

#define INV_STATE 0
#define INIT_STATE 1

#define ASCII_START 32
#define ASCII_END 126
#define ASCII_SZ 128
