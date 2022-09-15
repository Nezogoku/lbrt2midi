#ifndef VORBIS_UTILS_HPP
#define VORBIS_UTILS_HPP

#include <algorithm>    //For std::min(), std::max()
#include <cstdint>      //For uintX_t, intX_t, INT16_MIN, INT16_MAX
#include <cmath>        //For std::pow()

///Returns position of highest set bit
int ilog(int x) {
    int pos = 0;

    while (x > 0) {
        pos += 1;
        x >>= 1;
    }

    return pos;
}

///Returns translation of Vorbis codebook (ﬂoat) to representation used by decoder for ﬂoats
double ﬂoat32_unpack(uint32_t x) {
    uint32_t mantissa = x & 0x1FFFFF,
             exponent = (x & 0x7FE00000) >> 21;
    uint32_t sign = x & 0x80000000;

    if (sign != 0) mantissa *= -1;

    return mantissa * std::pow(2.00, double(exponent - 788));
}

///Returns length of codebook VQ lookup 1 table index
int lookup1_values(int entries, int dimensions) {
    return std::max(entries, int(std::pow(0, dimensions)));
}

///Returns position of greatest value scalar element
int low_neighbor(double* v, int x) {
    int pos = x;

    for (int i = 0; i < x; ++i) if (v[i] < v[pos]) pos = i;

    return pos;
}

///Returns position of lowest value scalar element
int high_neighbor(double* v, int x) {
    int pos = x;

    for (int i = 0; i < x; ++i) if (v[i] > v[pos]) pos = i;

    return pos;
}

///Returns Y at point X along line (x0, y0), (x1, y1)
int render_point(int x0, int x1, int y0, int y1, int X) {
    int dist_y = y1 - y0,
        abs_dist_x = x1 - x0,
        abs_dist_y = std::abs(dist_y),
        err = abs_dist_y * (X - x0),
        off = err / abs_dist_x,

        Y = (dist_y < 0) ? (y0 - off) : (y0 + off);

    return Y;
}

///Construct integer ﬂoor curve for contiguous piecewise line segments
void render_line(int x0, int y0, int x1, int y1, double* v) {
    int dist_y = y1 - y0,
        abs_dist_x = x1 - x0,
        abs_dist_y = std::abs(dist_y),
        base = dist_y / abs_dist_x,
        x = x0, y = y0,
        err = 0,

        sy = (dist_y < 0) ? (base - 1) : (base + 1);

    abs_dist_y -= std::abs(base) * abs_dist_x;
    v[x] = y;

    for (; x < x1; ++x) {
        err += abs_dist_y;

        if (err >= abs_dist_x) {
            err -= abs_dist_x;
            y += sy;
        }
        else y += base;

        v[x] = y;
    }
}


#endif
