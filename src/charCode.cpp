#include "charCode.hpp"

bool isValidAscii(int c) {
    if (c == NEWLINE || c == TAB || (c >= SPACE && c < DEL)) {
        return true;
    } else {
        return false;
    }
}

char getControlCharAscii(int c) {
    if (c == NEWLINE) {
        return 'c';
    } else if (c == TAB) {
        return 't';
    } else {
        return '\0';
    }
}
