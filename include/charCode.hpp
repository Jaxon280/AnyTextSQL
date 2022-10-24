#pragma once

const int NUL = 0x00;
const int TAB = 0x09;
const int NEWLINE = 0x0A;
const int SPACE = 0x20;
const int DEL = 0x7F;

#include "common.hpp"

bool isValidAscii(int c);
char getControlCharAscii(int c);
