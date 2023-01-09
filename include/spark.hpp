#pragma once

#include "common.hpp"

struct Column {
    int size;
    int offset;
};

struct SparkContext {
    void *ptr;
    int count;
    int sizeInRow;
    int varSize;
    Column *varCols;
    SparkContext(void *baseAddr, int _sizeInRow, int _varSize) : ptr(baseAddr), count(0), sizeInRow(_sizeInRow), varSize(_varSize) {
        varCols = new Column[_varSize];
    }
};