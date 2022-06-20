#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "csv_dfa.hpp"

#define ST_TYPE uint8_t

int main(int argc, char* argv[]) {
    int stateSize;
    int acceptStateSize;
    ST_TYPE** dfa;
    ST_TYPE* acceptStates;

    int fd = open(argv[1], O_RDONLY);
    struct stat fst;
    fstat(fd, &fst);
    unsigned long long size = sizeof(char) * fst.st_size;
    char* data = new char[size + 1];
    data[size] = '\0';

    if (read(fd, data, fst.st_size) < 0) {
        perror("read");
        exit(1);
    }

    generate_csv_dfa(&dfa, &acceptStates, &stateSize, &acceptStateSize);
    ST_TYPE curState = 1;
    unsigned long long i = 0;
    int count = 0;
    int recentAcceptState = 0, recentAcceptIndex = 0;

#if (defined BENCH)
    double ex_time = 0.0;
    timeval lex1, lex2;
    gettimeofday(&lex1, NULL);
#endif

    while (data[i] != '\0' && i < size) {
        curState = dfa[curState][(uint8_t)data[i]];
        if (curState == 0) {
            if (recentAcceptState != 0) {
                count++;
                i = recentAcceptIndex;
                recentAcceptState = 0;
            }
            recentAcceptState = 0, recentAcceptIndex = 0;
            curState = 1;
        } else if (curState == 17) {
            recentAcceptState = 17;
            recentAcceptIndex = i;
        }
        i++;
    }

#if (defined BENCH)
    gettimeofday(&lex2, NULL);
    ex_time =
        (lex2.tv_sec - lex1.tv_sec) + (lex2.tv_usec - lex1.tv_usec) * 0.000001;
    printf("#BENCH_Lex: %lf s\n\n", ex_time);
    printf("#File size: %d MB\n\n", (size >> 20));
    printf("#Throughput: %lf MB/s\n\n", ((double)(size >> 20) / ex_time));
#endif

    printf("count: %d\n", count);
}
