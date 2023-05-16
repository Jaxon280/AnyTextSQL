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

void parse_cmpestrm(char *buf, int size) {
    char simddata1[] = {',','\n','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};
    char simddata2[] = {'\n','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};
    char simddata3[] = {'C','l','e','v','e','l','a','n','d','\0','\0','\0','\0','\0','\0','\0'};
    __m128i s1 = _mm_loadu_si128((__m128i_u *)simddata1);
    __m128i s2 = _mm_loadu_si128((__m128i_u *)simddata2);
    __m128i s3 = _mm_loadu_si128((__m128i_u *)simddata3);

    int i = 0;
    int cnt = 0;
    int count = 0;
    int r;
    const int ccnt = 13;
    __m128i c;
q0:
    if (i >= size) {
        printf("%d\n", count);
        return;
    }
    c = _mm_loadu_si128((__m128i_u *)&buf[i]);
    __m128i mm = _mm_cmpestrm(s1, 2, c, 16, _SIDD_CMP_EQUAL_ANY | _SIDD_BIT_MASK);
    int m = _mm_extract_epi16(mm, 0);
    cnt += __builtin_popcount(m);
    if (cnt < ccnt) {
        i += 16;
        goto q0;
    } else if (cnt > ccnt) {
        int cm;
        while (cnt != ccnt) {
            cm = __builtin_ctz(m);
            m = m ^ (1 << cm);
            cnt--;
        }
        i += (cm + 1);
        goto q1;
    } else {
        i += 32 - __builtin_clz(m); // 17
        goto q1;
    }
q1:
    cnt = 0;
    c = _mm_loadu_si128((__m128i_u *)&buf[i]);
    r = _mm_cmpestri(s3, 9, c, 16, _SIDD_CMP_EQUAL_ORDERED);
    if (r != 0) {
        goto q2;
    }
    i += (r + 9);
    goto q3;
q2:
    c = _mm_loadu_si128((__m128i_u *)&buf[i]);
    r = _mm_cmpestri(s2, 1, c, 16, _SIDD_CMP_EQUAL_ANY);
    if (r == 16) {
        i += 16;
        goto q2;
    }
    i += (r + 1);
    goto q0;
q3:
    c = _mm_loadu_si128((__m128i_u *)&buf[i]);
    r = _mm_cmpestri(s2, 2, c, 16, _SIDD_CMP_EQUAL_ANY);
    if (r == 16) {
        i += 16;
        goto q3;
    }
    i += (r + 1);
    count++;
    goto q0;
}

void parse_cmpestri(char *buf, int size) {
    char simddata1[] = {',','\n','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};
    char simddata2[] = {'\n','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0','\0'};
    char simddata3[] = {'U','S','/','E','a','s','t','e','r','n','\0','\0','\0','\0','\0','\0'};
    __m128i s1 = _mm_loadu_si128((__m128i_u *)simddata1);
    __m128i s2 = _mm_loadu_si128((__m128i_u *)simddata2);
    __m128i s3 = _mm_loadu_si128((__m128i_u *)simddata3);

    int i = 0;
    int cnt = 0;
    int count = 0;
    int r;
    __m128i c;
q0:
    if (i >= size) {
        printf("%d\n", count);
        return;
    }
    c = _mm_loadu_si128((__m128i_u *)&buf[i]);
    r = _mm_cmpestri(s1, 2, c, 16, _SIDD_CMP_EQUAL_ANY);
    if (r == 16) {
        i += 16;
        goto q0;
    }
    i += r;
q1:
    c = _mm_loadu_si128((__m128i_u *)&buf[++i]);
    r = _mm_cmpestri(s1, 2, c, 16, _SIDD_CMP_EQUAL_ANY);
    if (r == 16) {
        i += 16;
        goto q1;
    }
    i += r;
q2:
    c = _mm_loadu_si128((__m128i_u *)&buf[++i]);
    r = _mm_cmpestri(s1, 2, c, 16, _SIDD_CMP_EQUAL_ANY);
    if (r == 16) {
        i += 16;
        goto q2;
    }
    i += r;
q3:
    c = _mm_loadu_si128((__m128i_u *)&buf[++i]);
    r = _mm_cmpestri(s1, 2, c, 16, _SIDD_CMP_EQUAL_ANY);
    if (r == 16) {
        i += 16;
        goto q3;
    }
    i += r;
q4:
    c = _mm_loadu_si128((__m128i_u *)&buf[++i]);
    r = _mm_cmpestri(s1, 2, c, 16, _SIDD_CMP_EQUAL_ANY);
    if (r == 16) {
        i += 16;
        goto q4;
    }
    i += r;
q5:
    c = _mm_loadu_si128((__m128i_u *)&buf[++i]);
    r = _mm_cmpestri(s1, 2, c, 16, _SIDD_CMP_EQUAL_ANY);
    if (r == 16) {
        i += 16;
        goto q5;
    }
    i += r;
q6:
    c = _mm_loadu_si128((__m128i_u *)&buf[++i]);
    r = _mm_cmpestri(s1, 2, c, 16, _SIDD_CMP_EQUAL_ANY);
    if (r == 16) {
        i += 16;
        goto q6;
    }
    i += r;
q7:
    c = _mm_loadu_si128((__m128i_u *)&buf[++i]);
    r = _mm_cmpestri(s1, 2, c, 16, _SIDD_CMP_EQUAL_ANY);
    if (r == 16) {
        i += 16;
        goto q7;
    }
    i += r;
q8:
    c = _mm_loadu_si128((__m128i_u *)&buf[++i]);
    r = _mm_cmpestri(s1, 2, c, 16, _SIDD_CMP_EQUAL_ANY);
    if (r == 16) {
        i += 16;
        goto q8;
    }
    i += r;
q9:
    c = _mm_loadu_si128((__m128i_u *)&buf[++i]);
    r = _mm_cmpestri(s1, 2, c, 16, _SIDD_CMP_EQUAL_ANY);
    if (r == 16) {
        i += 16;
        goto q9;
    }
    i += r;
q10:
    c = _mm_loadu_si128((__m128i_u *)&buf[++i]);
    r = _mm_cmpestri(s1, 2, c, 16, _SIDD_CMP_EQUAL_ANY);
    if (r == 16) {
        i += 16;
        goto q10;
    }
    i += r;
q11:
    c = _mm_loadu_si128((__m128i_u *)&buf[++i]);
    r = _mm_cmpestri(s1, 2, c, 16, _SIDD_CMP_EQUAL_ANY);
    if (r == 16) {
        i += 16;
        goto q11;
    }
    i += r;
q12:
    c = _mm_loadu_si128((__m128i_u *)&buf[++i]);
    r = _mm_cmpestri(s1, 2, c, 16, _SIDD_CMP_EQUAL_ANY);
    if (r == 16) {
        i += 16;
        goto q12;
    }
    i += r;
q13:
    c = _mm_loadu_si128((__m128i_u *)&buf[++i]);
    r = _mm_cmpestri(s1, 2, c, 16, _SIDD_CMP_EQUAL_ANY);
    if (r == 16) {
        i += 16;
        goto q13;
    }
    i += r;
q14:
    c = _mm_loadu_si128((__m128i_u *)&buf[++i]);
    r = _mm_cmpestri(s1, 2, c, 16, _SIDD_CMP_EQUAL_ANY);
    if (r == 16) {
        i += 16;
        goto q14;
    }
    i += r;
q15:
    c = _mm_loadu_si128((__m128i_u *)&buf[++i]);
    r = _mm_cmpestri(s1, 2, c, 16, _SIDD_CMP_EQUAL_ANY);
    if (r == 16) {
        i += 16;
        goto q15;
    }
    i += r;
q16:
    c = _mm_loadu_si128((__m128i_u *)&buf[++i]);
    r = _mm_cmpestri(s1, 2, c, 16, _SIDD_CMP_EQUAL_ANY);
    if (r == 16) {
        i += 16;
        goto q16;
    }
    i += r;
q17:
    c = _mm_loadu_si128((__m128i_u *)&buf[++i]);
    r = _mm_cmpestri(s1, 2, c, 16, _SIDD_CMP_EQUAL_ANY);
    if (r == 16) {
        i += 16;
        goto q17;
    }
    i += r;
q18:
    c = _mm_loadu_si128((__m128i_u *)&buf[++i]);
    r = _mm_cmpestri(s3, 10, c, 16, _SIDD_CMP_EQUAL_ORDERED);
    if (r != 0) {
        goto q19;
    }
    i += (r + 10);
    goto q20;
q19:
    c = _mm_loadu_si128((__m128i_u *)&buf[i]);
    r = _mm_cmpestri(s2, 1, c, 16, _SIDD_CMP_EQUAL_ANY);
    if (r == 16) {
        i += 16;
        goto q19;
    }
    i += (r + 1);
    goto q0;
q20:
    c = _mm_loadu_si128((__m128i_u *)&buf[i]);
    r = _mm_cmpestri(s2, 2, c, 16, _SIDD_CMP_EQUAL_ANY);
    if (r == 16) {
        i += 16;
        goto q20;
    }
    i += (r + 1);
    count++;
    goto q0;
}

int main(int argc, char *argv[]) {
    int fd = open("US_Accidents_Dec21_updated.csv", O_RDONLY);
    struct stat fst;
    fstat(fd, &fst);
    int rsize = fst.st_size;
    char *buf = new char[rsize + 16];
    read(fd, buf, rsize);
    buf[rsize] = '\0';

#if (defined BENCH)
    double ex_time = 0.0;
    timeval lex1, lex2;
    gettimeofday(&lex1, NULL);
#endif

    parse_cmpestri(buf, rsize);

#if (defined BENCH)
    gettimeofday(&lex2, NULL);
    ex_time =
        (lex2.tv_sec - lex1.tv_sec) + (lex2.tv_usec - lex1.tv_usec) * 0.000001;
    printf("#BENCH_parse: %lf s\n\n", ex_time);
#endif
}
