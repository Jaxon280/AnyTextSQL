#pragma once

#include "common.hpp"

namespace vlex {

class ioStream {
   public:
    ioStream(const std::string &filename) {
        fd = open(filename.c_str(), O_RDONLY);
        struct stat fst;
        fstat(fd, &fst);
        size = fst.st_size;
    }
    ~ioStream() { close(fd); }

    int seek(SIZE_TYPE start);
    int readFile(SIZE_TYPE start, SIZE_TYPE rsize);

    inline DATA_TYPE *getData() const { return data; }
    inline bool hasData() const {
        if (pos >= (size - 1)) {
            return false;
        }
        return true;
    }
    inline SIZE_TYPE getSize() const { return size; }

   private:
    SIZE_TYPE size;
    SIZE_TYPE pos = 0;
    int fd;
    DATA_TYPE *data = NULL;
};

}  // namespace vlex
