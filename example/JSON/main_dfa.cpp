#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#include "categories_dfa.hpp"

#define DFA_ST_TYPE uint8_t

int main(int argc, char* argv[]) {
    int stateSize;
    int acceptStateSize;
    DFA_ST_TYPE** dfa;
    DFA_ST_TYPE* acceptStates;

    int fd = open(argv[1], O_RDONLY);
    struct stat fst;
    fstat(fd, &fst);
    size_t size = sizeof(char) * fst.st_size;
    char* data = new char[size + 1];
    data[size] = '\0';

    if (read(fd, data, fst.st_size) < 0) {
        perror("read");
        exit(1);
    }

    generate_dfa(&dfa, &acceptStates, &stateSize, &acceptStateSize);
    DFA_ST_TYPE curState = 1;
    size_t i = 0;
    int count = 0;
    int recentAcceptState = 0, recentAcceptIndex = 0;

#if (defined BENCH)
    double ex_time = 0.0;
    timeval lex1, lex2;
    gettimeofday(&lex1, NULL);
#endif

    while (data[i] != '\0' && i < size) {
        curState = dfa[curState][(int)data[i]];
        if (curState == 0) {
            if (recentAcceptState != 0) {
                count++;
                i = recentAcceptIndex;
                recentAcceptState = 0;
            }
            recentAcceptState = 0, recentAcceptIndex = 0;
            curState = 1;
        } else if (curState == 27) {
            recentAcceptState = 27;
            recentAcceptIndex = i;
        }
        i++;
    }

#if (defined BENCH)
    gettimeofday(&lex2, NULL);
    ex_time =
        (lex2.tv_sec - lex1.tv_sec) + (lex2.tv_usec - lex1.tv_usec) * 0.000001;
    printf("#BENCH_Lex: %lf s\n\n", ex_time);
#endif

    printf("count: %d\n", count);
    printf("size: %lld\n", i);
}
