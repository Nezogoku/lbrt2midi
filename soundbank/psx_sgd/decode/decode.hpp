#ifndef DECODE_HPP
#define DECODE_HPP

#include <cstdint>

///Decodes OGG-VORBIS files
int oggVorbDecode(unsigned char *oggVorbData, int oggVorbSize, int channels, int samplerate, int16_t *&outData, int outSize);

///Decodes header-less VAG files
int adpcmDecode(unsigned char *adpcmData, int adpcmSize, uint8_t &loopType, int16_t *&outData, int outSize);


#endif
