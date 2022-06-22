#include <iostream>
#include <string>

#include "sample_dfa.hpp"
#include "stdio.h"
#include "sys/time.h"
#include "vlex.hpp"

using namespace vlex;

int main(int argc, char* argv[]) {
    std::string filename = std::string(argv[1]);

    DFA dfa = generate_reviewtext_dfa();

#if (defined BENCH)
    double ex_time = 0.0;
    timeval lex1, lex2;
    gettimeofday(&lex1, NULL);
#endif

    Vlex vlex = Vlex(filename, dfa);
    vlex.construct(0.002);
    vlex.exec();
    vlex.printCount();
    vlex.printTokens(10);

#if (defined BENCH)
    gettimeofday(&lex2, NULL);
    ex_time =
        (lex2.tv_sec - lex1.tv_sec) + (lex2.tv_usec - lex1.tv_usec) * 0.000001;
    printf("#BENCH_Total: %lf s\n\n", ex_time);
#endif
}
