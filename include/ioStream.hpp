#pragma once

#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <string>

#include "common.hpp"

class ioStream {
   private:
    std::string filename;
    SIZE_TYPE size;
    SIZE_TYPE pos = 0;
    int fd;
    DATA_TYPE *data = NULL;

   public:
    ioStream(std::string &filename) : filename(filename) {
        fd = open(filename.c_str(), O_RDONLY);
        struct stat fst;
        fstat(fd, &fst);
        size = fst.st_size;
    }

    ~ioStream() { close(fd); }

    inline bool hasData() {
        if (pos >= (size - 1)) {
            return false;
        }
        return true;
    }

    int seek(SIZE_TYPE start) {
        off_t p = lseek(fd, start, SEEK_SET);
        if (p != start) return 0;

        pos = start;
        return 1;
    }

    int readFile(SIZE_TYPE start, SIZE_TYPE rsize) {
        if (data) delete data;

        // update pos
        if (pos >= (size - 1) || rsize <= 0) return 0;
        if ((pos + rsize) >= size) {
            rsize = (size - pos - 1);
        }
        pos += rsize;

        // allocate memory
        data = new DATA_TYPE[rsize + 16];
        for (int i = 0; i < 16; i++) {
            data[rsize + i] = '\0';
        }

        // read
        int rsz = read(fd, data, rsize);
        if (rsz < 0) {
            perror("read");
            return 0;
        } else if (rsz != rsize) {
            printf("Error: Read size\n");
            return 0;
        }

        return rsz;
    }

    inline DATA_TYPE *getData() { return data; }
    inline SIZE_TYPE getSize() { return size; }
};
