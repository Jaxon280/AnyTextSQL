#include "common.hpp"
#include "converter.hpp"
#include "example/JSON/categories_dfa.hpp"

int main(int argc, char** argv) {
    int stateSize = 29;
    ST_TYPE** dfa = (ST_TYPE**)malloc(sizeof(ST_TYPE*) * stateSize);
    for (int i = 0; i < stateSize; i++) {
        dfa[i] = (ST_TYPE*)malloc(sizeof(ST_TYPE) * 128);
    }
    int acceptStateSize = 1;
    ST_TYPE* acceptStates = (ST_TYPE*)malloc(sizeof(ST_TYPE) * acceptStateSize);
    acceptStates[0] = 27;

    std::string filename = "vfa.hpp";

    generate_sample_dfa(dfa, stateSize);

#if (defined BENCH)
    timeval lex1, lex2;
    gettimeofday(&lex1, NULL);
#endif

    vlex::VectFA vfa(dfa, acceptStates, stateSize, acceptStateSize);
    vfa.codegen(filename);

#if (defined BENCH)
    gettimeofday(&lex2, NULL);
    printf(
        "\n0 | #BENCH_Lex %lf\n\n",
        (lex2.tv_sec - lex1.tv_sec) + (lex2.tv_usec - lex1.tv_usec) * 0.000001);
#endif
}
