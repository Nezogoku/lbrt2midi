#include <cstdint>
#include "stb_vorbis.h"
#include "../decode.hpp"

int oggVorbDecode(unsigned char *oggVorbData, int oggVorbSize, int channels, int samplerate, int16_t *&outData, int outSize) {
    int err = stb_vorbis_decode_memory(oggVorbData, oggVorbSize, &channels, &samplerate, &outData);

    return (err >= 0);
}
