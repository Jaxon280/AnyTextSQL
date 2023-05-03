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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("Enter how many multiples.");
        return 1;
    }
    int multiple = atoi(argv[1]);

    int rfd = open("tweet_1GB.json", O_RDONLY);
    int wfd = open("tweet_copy.json", O_WRONLY | O_CREAT, 0644);
    char data[] = "\"data\": [\n";
    __m128i d = _mm_loadu_si128((__m128i_u *)data);
    char meta[] = "\"meta\": {\n";
    __m128i m = _mm_loadu_si128((__m128i_u *)meta);

    struct stat fst;
    fstat(rfd, &fst);
    int rsize = fst.st_size;
    int totalSize = 3;
    int ri = 0, wi = 0;
    char *buf = new char[rsize + 10];
    char *dstbuf = new char[rsize + 10];
    char *tailbuf = new char[16];
    int start, end;
    read(rfd, buf, rsize);
    tailbuf[0] = '[';
    tailbuf[1] = '\n';
    write(wfd, tailbuf, 2);

    int offset = 0;
    while (ri < rsize) {
        __m128i c = _mm_loadu_si128((__m128i_u *)&buf[ri]);
        int r = _mm_cmpestri(d, 10, c, 16, _SIDD_CMP_EQUAL_ORDERED);
        if (r == 16) {
            ri += 16;
            continue;
        } else if (r > 6) {
            ri += r;
            continue;
        } else {
            ri += (r + 10);
            start = ri;
        }

search_meta:    
        c = _mm_loadu_si128((__m128i_u *)&buf[ri]);
        r = _mm_cmpestri(m, 10, c, 16, _SIDD_CMP_EQUAL_ORDERED);
        if (r == 16) {
            ri += 16;
            if (ri >= rsize) break;
            goto search_meta;
        } else if (r > 6) {
            ri += r;
            if (ri >= rsize) break;
            goto search_meta;
        } else {
            end = ri + r;
            ri += (r + 10);
        }
        int size = end - start - 12;
        memcpy(&dstbuf[offset], &buf[start], size); // space * 8 + \n + ,
        dstbuf[offset + size] = ',';
        dstbuf[offset + size + 1] = '\n';
        totalSize += (size + 2), offset += (size + 2);
    }

    // printf("%d\n", (int)dstbuf[totalSize - 4]);

    for (int i = 0; i < multiple - 1; i++) {
        write(wfd, dstbuf, totalSize - 3);
    }

    write(wfd, dstbuf, totalSize - 5);

    tailbuf[0] = '\n';
    tailbuf[1] = ']';
    tailbuf[2] = '\0';
    write(wfd, tailbuf, 3);
}