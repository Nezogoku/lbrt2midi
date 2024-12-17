#ifndef RIFFWAVE_FUNC_HPP
#define RIFFWAVE_FUNC_HPP

#include <vector>
#include "chunk_type.hpp"
#include "riffwave_types.hpp"

#ifdef ALLRIFFWAVE_IMPLEMENTATION
#define NEEDEDRIFFWAVE_IMPLEMENTATION
#define PLAYLISTRIFFWAVE_IMPLEMENTATION
#define ASSOCIATEDRIFFWAVE_IMPLEMENTATION
#define UNPACKWAVL_IMPLEMENTATION
#define UNPACKSMPL_IMPLEMENTATION
#define UNPACKINST_IMPLEMENTATION
#endif

#ifdef NEEDEDRIFFWAVE_IMPLEMENTATION
#define UNPACKFMT_IMPLEMENTATION
#define UNPACKFACT_IMPLEMENTATION
#endif

#ifdef PLAYLISTRIFFWAVE_IMPLEMENTATION
#define UNPACKCUE_IMPLEMENTATION
#define UNPACKPLST_IMPLEMENTATION
#endif

#ifdef ASSOCIATEDRIFFWAVE_IMPLEMENTATION
#define UNPACKLIST_IMPLEMENTATION
#define UNPACKADTL_IMPLEMENTATION
#define UNPACKLABL_IMPLEMENTATION
#define UNPACKNOTE_IMPLEMENTATION
#define UNPACKLTXT_IMPLEMENTATION
#endif


inline extern riffwave wav_inf = {};

void unpackRiffWave(const chunk chnk);
void unpackRiffWave(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);
std::vector<unsigned char> packRiffWave();

#ifdef UNPACKFMT_IMPLEMENTATION
void unpackFmt(const chunk chnk);
void unpackFmt(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);
#endif

#ifdef UNPACKFACT_IMPLEMENTATION
void unpackFact(const chunk chnk);
void unpackFact(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);
#endif

#ifdef UNPACKCUE_IMPLEMENTATION
void unpackCue(const chunk chnk);
void unpackCue(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);
#endif

#ifdef UNPACKPLST_IMPLEMENTATION
void unpackPlst(const chunk chnk);
void unpackPlst(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);
#endif

#ifdef UNPACKADTL_IMPLEMENTATION
void unpackAdtl(const chunk chnk);
void unpackAdtl(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);
#endif

#ifdef UNPACKLABL_IMPLEMENTATION
void unpackLabl(const chunk chnk);
void unpackLabl(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);
#endif

#ifdef UNPACKNOTE_IMPLEMENTATION
void unpackNote(const chunk chnk);
void unpackNote(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);
#endif

#ifdef UNPACKLTXT_IMPLEMENTATION
void unpackLtxt(const chunk chnk);
void unpackLtxt(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);
#endif

#ifdef UNPACKWAVL_IMPLEMENTATION
void unpackWavl(const chunk chnk) ;
void unpackWavl(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);
#endif

#ifdef UNPACKSMPL_IMPLEMENTATION
void unpackSmpl(const chunk chnk);
void unpackSmpl(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);
#endif

#ifdef UNPACKINST_IMPLEMENTATION
void unpackInst(const chunk chnk);
void unpackInst(
    unsigned char *in, const unsigned length,
    const EndianType endian = ENDIAN_LITTLE, const bool is_rv = 0
);
#endif


#endif
