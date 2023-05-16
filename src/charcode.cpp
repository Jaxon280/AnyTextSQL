#include "common/charcode.hpp"

namespace vlex {
 
bool isValidAscii(int c) {
    // if (c == NEWLINE || c == TAB || (c >= SPACE && c < DEL)) {
    //     return true;
    // } else {
    //     return false;
    // }
    if (c < ASCII_SZ && c >= 0) {
        return true;
    } else {
        return false;
    }
}

char getControlCharAscii(int c) {
    if (c == NEWLINE) {
        return 'n';
    } else if (c == TAB) {
        return 't';
    } else {
        return '\0';
    }
}  

}
