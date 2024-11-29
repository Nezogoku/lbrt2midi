#ifndef LBRT_FUNC_HPP
#define LBRT_FUNC_HPP

#include "lrt_types.hpp"

inline extern bool lrt_debug = false, lrt_midicsv = false;
inline extern lbrtinfo lrt_inf = {};

extern void unpackLrt(const char *file = 0);
extern void unpackLrt(unsigned char *in, const unsigned length);

extern void extractLrt(const char *folder = 0);


#endif