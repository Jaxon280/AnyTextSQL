#include <iostream>

#include "ioStream.hpp"
#include "sys/time.h"
#include "vfa.hpp"

int main(int argc, char *argv[]) {
    std::string filename = std::string(argv[1]);
    ioStream ios(filename);
    const off_t partition = 1;
    off_t rsize = ios.getSize() / partition + 1;
    int iter = 1;

    double total_ex_time = 0.0;

    while (ios.hasData()) {
        std::vector<Token> tokens;
        int cnt = 0;
        ios.readFile(0, rsize);
        printf("read data: %d/%d\n", iter, partition);
        VFALexer lexer(ios.getData(), 0, rsize);

#if (defined BENCH)
        double ex_time = 0.0;
        timeval lex1, lex2;
        gettimeofday(&lex1, NULL);
#endif

        tokens = lexer.lex();
        // tokens = lexer.lex0();

#if (defined BENCH)
        gettimeofday(&lex2, NULL);
        ex_time = (lex2.tv_sec - lex1.tv_sec) +
                  (lex2.tv_usec - lex1.tv_usec) * 0.000001;
        printf("#BENCH_Lex: %d/%d %lf s\n\n", iter, partition, ex_time);
        total_ex_time += ex_time;
#endif

        for (Token &token : tokens) {
            cnt++;
        }
        printf("america count: %d\n", cnt);
        iter++;
    }

#if (defined BENCH)
    printf("#BENCH_Lex Total %lf s\n\n", total_ex_time);
    printf("#File size: %d MB\n\n", (ios.getSize() >> 20));
    printf("#Throughput: %lf MB/s\n\n",
           ((double)(ios.getSize() >> 20) / total_ex_time));
#endif
}
