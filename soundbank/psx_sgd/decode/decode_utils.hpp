#ifndef DECODE_UTILS_HPP
#define DECODE_UTILS_HPP


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
