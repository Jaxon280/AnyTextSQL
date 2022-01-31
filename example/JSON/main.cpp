#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <unistd.h>

#include "vfa.hpp"

int main(int argc, char *argv[]) {
#if (defined BENCH)
    timeval total1, total2;
    gettimeofday(&total1, NULL);
#endif

#if (defined BENCH)
    timeval disk1, disk2;
    gettimeofday(&disk1, NULL);
#endif

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

#if (defined BENCH)
    gettimeofday(&disk2, NULL);
    printf("\n0 | #BENCH_Disk %lf\n\n",
           (disk2.tv_sec - disk1.tv_sec) +
               (disk2.tv_usec - disk1.tv_usec) * 0.000001);
#endif
    const int epochs = 30;

    double ex_time = 0.0;
    int cnt = 0;

    for (int i = 0; i < epochs; i++) {
        VFALexer lexer(data, 0, size);
        std::vector<Token> tokens;
#if (defined BENCH)
        timeval lex1, lex2;
        gettimeofday(&lex1, NULL);
#endif
        tokens = lexer.lex();
        // int count = lexer.lex();

#if (defined BENCH)
        gettimeofday(&lex2, NULL);
        ex_time += (lex2.tv_sec - lex1.tv_sec) +
                   (lex2.tv_usec - lex1.tv_usec) * 0.000001;
#endif
        for (Token &token : tokens) {
            cnt++;
        }
    }

#if (defined BENCH)
    printf("#BENCH_Lex %lf s\n\n", ex_time);
    printf("#File size: %d MB\n\n", (size >> 20) * epochs);
    printf("#Throughput: %lf MB/s\n\n",
           ((double)(size >> 20) * (double)epochs / ex_time));
#endif

    // printf("restaurant count: %d\n", count);
    // printf("above 4.5 restaurants: %d\n", count);

    printf("stars count: %d\n", cnt);

#if (defined BENCH)
    gettimeofday(&total2, NULL);
    printf("\n0 | #BENCH_Total %lf\n\n",
           (total2.tv_sec - total1.tv_sec) +
               (total2.tv_usec - total1.tv_usec) * 0.000001);
#endif
}
