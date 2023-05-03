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
    int colSize;
    int varSize;
    VarlenColumn *varCols;
    SparkContext(void *baseAddr, int _sizeInRow, int _colSize, int _varSize) : ptr(baseAddr), count(0), sizeInRow(_sizeInRow), colSize(_colSize), varSize(_varSize) {
        varCols = new VarlenColumn[_varSize];
    }
};