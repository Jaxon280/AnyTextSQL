#include "vfa.hpp"

int main(int argc, char *argv[]) {
    int fd = open(argv[1], O_RDONLY);
    struct stat fst;
    fstat(fd, &fst);
    int size = sizeof(char) * fst.st_size;
    char *data = new char[size + 1];
    data[size] = '\0';

    if (read(fd, data, fst.st_size) < 0) {
        perror("read");
        exit(1);
    }

    VFALexer lexer(data, 0, size);

#if (defined BENCH)
    timeval lex1, lex2;
    gettimeofday(&lex1, NULL);
#endif

    std::vector<Token> tokens = lexer.lex();

#if (defined BENCH)
    gettimeofday(&lex2, NULL);
    printf(
        "\n0 | #BENCH_Lex %lf\n\n",
        (lex2.tv_sec - lex1.tv_sec) + (lex2.tv_usec - lex1.tv_usec) * 0.000001);
#endif

    int cnt = 0;
    for (Token &token : tokens) {
        cnt++;
    }
    printf("%d\n", cnt);
}
