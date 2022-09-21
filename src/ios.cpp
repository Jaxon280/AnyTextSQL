#include "general/ios.hpp"

namespace vlex {

int ioStream::seek(SIZE_TYPE start) {
    off_t p = lseek(fd, start, SEEK_SET);
    if (p != start) return 0;

    pos = start;
    return 1;
}

int ioStream::readFile(SIZE_TYPE start, SIZE_TYPE rsize) {
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
    ssize_t rsz = read(fd, data, rsize);
    if (rsz < 0) {
        perror("read");
        return 0;
    } else if ((SIZE_TYPE)rsz != rsize) {
        printf("Error: Read size\n");
        return 0;
    }

    return rsz;
}

}  // namespace vlex
