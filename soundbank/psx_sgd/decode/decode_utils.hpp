#ifndef DECODE_UTILS_HPP
#define DECODE_UTILS_HPP

#include <cstdint>      //For uintX_t, intX_t


///Gets char array from vector
char* getChars(char *data, int pos, int len) {
    char* out = new char[len];

    for (int s = 0; s < len; ++s) out[s] = data[pos + s];

    return out;
}


#endif
