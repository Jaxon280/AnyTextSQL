#pragma once

#include "common/common.hpp"

namespace vlex {

const int NUL = 0x00;
const int TAB = 0x09;
const int NEWLINE = 0x0A;
const int SPACE = 0x20;
const int DEL = 0x7F;

bool isValidAscii(int c);
char getControlCharAscii(int c);

} // namespace vlex
