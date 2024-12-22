#ifndef AUDIO_FUNC_HPP
#define AUDIO_FUNC_HPP

#include <vector>

#ifdef ALLSGXDAUDIO_IMPLEMENTATION
#define DECODESONYADPCM_IMPLEMENTATION
#define DECODEDOLBYAC3_IMPLEMENTATION
#define DECODEOGG_IMPLEMENTATION
#define DECODESONYAT3P_IMPLEMENTATION
#endif


#ifdef DECODESONYADPCM_IMPLEMENTATION
std::vector<short> decodeSonyAdpcm(
    unsigned char *in, const unsigned length, const unsigned smpls,
    const unsigned short chns = 1, const bool is_short = 0,
    signed *loop_b = 0, signed *loop_e = 0
);
#endif

#ifdef DECODEPCM_IMPLEMENTATION
std::vector<short> decodePcm(
    unsigned char *in, const unsigned length, const unsigned short align, const unsigned smpls,
    const unsigned short chns, const unsigned short bits = 16, const unsigned short bytes = 2,
    const bool is_be = 0, const bool is_signed = 1, const bool is_lalign = 0, const bool is_interl = 1
);
#endif

#ifdef DECODEDOLBYAC3_IMPLEMENTATION
std::vector<short> decodeDolbyAc3(
    unsigned char *in, const unsigned length, const unsigned smpls,
    const unsigned short align, const unsigned short chns
);
#endif

#ifdef DECODEOGG_IMPLEMENTATION
std::vector<short> decodeOgg(
    unsigned char *in, const unsigned length, const unsigned smpls,
    unsigned short *chns = 0
);
#endif

#ifdef DECODESONYAT3P_IMPLEMENTATION
std::vector<short> decodeSonyAt3p(
    unsigned char *in, const unsigned length, const unsigned smpls,
    const unsigned short align, const unsigned short chns, const unsigned *skip = 0
);
#endif


#endif
