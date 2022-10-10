#include <cstdint>      //For uintX_t, intX_t
#include <vector>       //For std::vector
#include "../decode.hpp"
#include "stb_vorbis.h"


std::vector<int16_t> oggDecode(char *oggData, uint32_t oggSize, int ch, uint32_t rt, uint32_t &loopStart, uint32_t &loopEnd) {
    std::vector<int16_t> wavData;

    stb_vorbis *oggStream;
    stb_vorbis_info oggInfo;    //Technically no point but whatever
    int oggSamples;

    oggStream = stb_vorbis_open_memory((unsigned char*)oggData, oggSize, NULL, NULL);
    oggInfo = stb_vorbis_get_info(oggStream);

    //Update number channels
    oggInfo.channels = ch;
    //Update sample rate
    oggInfo.sample_rate = rt;
    //Get number samples
    oggSamples = stb_vorbis_stream_length_in_samples(oggStream) * oggInfo.channels;

    const int BUFFER_SIZE = 65536;
    int16_t buffer[BUFFER_SIZE];

    while (oggSamples) {
        oggSamples = stb_vorbis_get_samples_short_interleaved(oggStream, oggInfo.channels, buffer, BUFFER_SIZE);
        for (int b = 0; b < oggSamples; ++b) { wavData.push_back(buffer[b]); }
    }

    stb_vorbis_close(oggStream);


    //Set looping points if there are none
    if (loopStart == 0xFFFFFFFF) loopStart = 0;
    if (loopEnd == 0xFFFFFFFF) loopEnd = wavData.size() - 1;

    return wavData;
}
