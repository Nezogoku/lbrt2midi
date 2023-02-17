#ifndef BIT_BYTE_HPP
#define BIT_BYTE_HPP

#include <cmath>

typedef unsigned char uchar;


///Reverse variable integer
static unsigned int setReverse(unsigned int tmpInt, int tmpSiz) {
    unsigned int buffer = 0x00;
    for (int b = 0; b < tmpSiz; ++b) {
        buffer |= (tmpInt >> (0x00 + (8 * b))) & 0xFF;
        if (b + 1 < tmpSiz) buffer <<= 8;
    }
    return buffer;
}

///Get variable big-endian int from array
static unsigned int getBeInt(uchar *&src, int length) {
    unsigned int out = 0;
    for (int s = 0; s < length; ++s) {
        out = (out << 8) | *src++;
    }
    return out;
}

///Get variable little-endian int from array
static unsigned int getLeInt(uchar *&src, int length) {
    unsigned int out = getBeInt(src, length);
    return setReverse(out, length);
}

///Compare variable char from array to char array
static bool cmpChar(uchar *src0, const char *src1, int length) {
    for (int s = 0; s < length; ++s) {
        if (src0[s] != (unsigned char)src1[s]) return false;
    }
    return true;
}

///Convert double to hex
static unsigned int fractToHex(double tune) {
    unsigned int out_hex = 0;

    while (tune) {
        int buffer = std::floor(tune);

        out_hex = (out_hex | buffer) << 4;
        tune = (tune - buffer) * 16;
    }

    return out_hex;
}

///Get formatted C-style string
template<typename... Args>
static char *getFData(const char *f_string, Args... f_args) {
    char *f_data = new char[255];
    sprintf(f_data, f_string, f_args...);

    return f_data;
}

#endif
