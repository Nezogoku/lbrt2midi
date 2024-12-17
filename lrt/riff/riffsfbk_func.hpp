#ifndef RIFFSFBK_FUNC_HPP
#define RIFFSFBK_FUNC_HPP

#include <vector>
#include "chunk_type.hpp"
#include "riffsfbk_types.hpp"

#ifdef ALLRIFFSFBK_IMPLEMENTATION
#define NEEDEDRIFFSFBK_IMPLEMENTATION
#define CHECKSFBK_IMPLEMENTATION
#endif

#ifdef NEEDEDRIFFSFBK_IMPLEMENTATION
#define UNPACKLIST_IMPLEMENTATION
#define UNPACKINFO_IMPLEMENTATION
#define UNPACKSDTA_IMPLEMENTATION
#define UNPACKPDTA_IMPLEMENTATION
#endif


inline extern riffsfbk sf2_inf = {};

void unpackRiffSfbk(const chunk chnk);
void unpackRiffSfbk(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);
std::vector<unsigned char> packRiffSfbk();

#ifdef UNPACKSDTA_IMPLEMENTATION
void unpackSdta(const chunk chnk);
void unpackSdta(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);
#endif

#ifdef UNPACKPDTA_IMPLEMENTATION
void unpackPdta(const chunk chnk);
void unpackPdta(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);

void unpackMod(const chunk chnk);
void unpackMod(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);

void unpackGen(const chunk chnk);
void unpackGen(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);

void unpackBag(const chunk chnk);
void unpackBag(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);

void unpackPhdr(const chunk chnk);
void unpackPhdr(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);

void unpackIhdr(const chunk chnk);
void unpackIhdr(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);

void unpackShdr(const chunk chnk);
void unpackShdr(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);
#endif

#ifdef CHECKSFBK_IMPLEMENTATION
int checkSfbk();
#endif


#endif
