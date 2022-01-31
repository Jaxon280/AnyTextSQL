#include "common.hpp"
#include "converter.hpp"
#include "example/JSON/categories_dfa.hpp"
#include "example/JSON/stars_dfa.hpp"

int main(int argc, char** argv) {
    int stateSize;
    int acceptStateSize;
    ST_TYPE** dfa;
    ST_TYPE* acceptStates;

    std::string filename = "vfa.hpp";

    int fd = open(argv[1], O_RDONLY);
    struct stat fst;
    fstat(fd, &fst);
    int size = sizeof(char) * fst.st_size;
    char* data = new char[size + 1];
    data[size] = '\0';

    if (read(fd, data, fst.st_size) < 0) {
        perror("read");
        exit(1);
    }

    generate_categories_dfa(&dfa, &acceptStates, &stateSize, &acceptStateSize);
    // generate_stars_dfa(&dfa, &acceptStates, &stateSize, &acceptStateSize);

#if (defined BENCH)
    timeval lex1, lex2;
    gettimeofday(&lex1, NULL);
#endif

    vlex::VectFA vfa(dfa, acceptStates, stateSize, acceptStateSize, data, size);
    vfa.codegen(filename);

#if (defined BENCH)
    gettimeofday(&lex2, NULL);
    printf(
        "\n0 | #BENCH_Lex %lf\n\n",
        (lex2.tv_sec - lex1.tv_sec) + (lex2.tv_usec - lex1.tv_usec) * 0.000001);
#endif
}
