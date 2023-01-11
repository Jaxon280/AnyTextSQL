#pragma once

#include "common.hpp"

struct VarlenColumn {
    int size;
    int offset;
};

struct SparkContext {
    void *ptr;
    int count;
    int sizeInRow;
    int varSize;
    VarlenColumn *varCols;
    SparkContext(void *baseAddr, int _sizeInRow, int _varSize) : ptr(baseAddr), count(0), sizeInRow(_sizeInRow), varSize(_varSize) {
        varCols = new VarlenColumn[_varSize];
    }
};