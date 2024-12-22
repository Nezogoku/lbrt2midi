#ifndef LBRT_FUNC_HPP
#define LBRT_FUNC_HPP

#include "lrt_types.hpp"

#ifndef PROGRAMME_IDENTIFIER
#define PROGRAMME_IDENTIFIER "lbrt2midi v8.1"
#endif


inline extern bool lrt_debug = false, lrt_midicsv = false;
inline extern lbrtinfo lrt_inf {};

void unpackLrt(const char *file = 0);
void unpackLrt(unsigned char *in, const unsigned length);

void extractLrt(const char *folder = 0);


#endif
