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
    std::vector<Token> tokens = lexer.lex();

    int cnt = 0;
    for (Token &token : tokens) {
        cnt++;
    }
    printf("%d\n", cnt);
}
