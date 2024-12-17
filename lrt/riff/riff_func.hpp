#ifndef RIFF_FUNC_HPP
#define RIFF_FUNC_HPP

#include <vector>
#include "chunk_type.hpp"
#include "riff_types.hpp"

#ifdef RIFFLISTINFO_IMPLEMENTATION
#define UNPACKLIST_IMPLEMENTATION
#define UNPACKINFO_IMPLEMENTATION
#endif


inline extern riffinfo riff_inf = {};

void unpackRiff(const chunk chnk);
void unpackRiff(unsigned char *in, const unsigned length);

#ifdef UNPACKLIST_IMPLEMENTATION
void unpackList(const chunk chnk);
void unpackList(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);
#endif

#ifdef UNPACKINFO_IMPLEMENTATION
void unpackInfo(const chunk chnk);
void unpackInfo(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);
#endif

#ifdef UNPACKCSET_IMPLEMENTATION
void unpackCset(const chunk chnk);
void unpackCset(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);
#endif

#ifdef UNPACKDISP_IMPLEMENTATION
void unpackDisp(const chunk chnk);
void unpackDisp(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);
#endif

#ifdef UNPACKCTOC_IMPLEMENTATION
void unpackCtoc(const chunk chnk);
void unpackCtoc(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);
#endif

#ifdef UNPACKCGRP_IMPLEMENTATION
void unpackCgrp(const chunk cgrp, const chunk ctoc);
void unpackCgrp(const chunk cgrp, const ctocinfo *info, const unsigned num);
void unpackCgrp(
    unsigned char *in, const unsigned length,
    const ctocinfo *info, const unsigned num,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);
#endif

//void unpackJunk(const chunk chnk);
//void unpackJunk(
//    unsigned char *in, const unsigned length,
//    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
//);

//void unpackPad(const chunk chnk);
//void unpackPad(
//    unsigned char *in, const unsigned length,
//    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
//);

//void unpackFllr(const chunk chnk);
//void unpackFllr(
//    unsigned char *in, const unsigned length,
//    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
//);


#endif
