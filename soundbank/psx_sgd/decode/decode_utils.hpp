#ifndef DECODE_UTILS_HPP
#define DECODE_UTILS_HPP

#include <cstring>      //For std::strcmp()


///Compares chars from vector to char array
bool cmpChars(char *data, char str1[], int pos, int len) {
    char str0[len + 1];
    for (int p = 0; p < len; ++p) str0[p] = data[pos + p];
    return !strcmp(str0, str1);
}

///Gets variable chars from vector
void getChars(char *data, char *out, int pos, int len) {
    for (int p = 0; p < len; ++p) out[p] = data[pos + p];

    pos += len;
}

///Gets variable bits from vector
void getBits(char *data, int &out, int pos, int len, int &shift) {
    out = 0;
    for (int b = 0; b < len; ++b) {
        out <<= 1;
        out |= (data[pos] >> shift++) & 0x01;
        if (shift == 8) { shift = 0; pos += 1; }
    }
}

///Gets LE variable int from vector
void getLeInt(char *data, int &out, int pos, int len) {
    out = 0;
    for (int s = len; s > 0; --s) {
        out <<= 8;
        out |= data[pos + (s - 1)];
    }

    pos += len;
}

///Gets BE variable int from vector
void getBeInt(char *data, int &out, int pos, int len) {
    out = 0;
    for (int s = 0; s < len; ++s) {
        out <<= 8;
        out |= data[pos + s];
    }

    pos += len;
}


#endif
