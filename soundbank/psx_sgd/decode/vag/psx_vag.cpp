#include <algorithm>    //For std::min(), std::max()
#include <cmath>        //For std::round()
#include <cstdint>      //For uintX_t, intX_t, INT16_MIN, INT16_MAX
#include <vector>       //For std::vector
#include "../decode.hpp"
#include "psx_vag_types.hpp"
#include "psx_vag_tables.hpp"

using std::round;
using std::max;
using std::min;
using std::vector;


//Derived from SonyVagDecoder found in es-ps2-vag-tool
//By jmarti856
std::vector<int16_t> vagDecode(char *vagData, uint32_t vagSize, int32_t &loopStart, int32_t &loopEnd) {
    vector<int16_t> wavData;

    bool isLoopStart = false;
    bool isLoopEnd = false;

    double hist0 = 0.0,
           hist1 = 0.0;

    for (int i = 0; i < vagSize; ++i) {
        uint8_t decodingCoeff;
        //Get decoding coefficient
        decodingCoeff = vagData[i++];

        chunk vagChunk;
        //Get shift byte
        vagChunk.shift = int8_t(decodingCoeff & 0x0F);
        //Get predicting byte
        vagChunk.predict = int8_t((decodingCoeff & 0xF0) >> 4);
        //Get flag byte
        vagChunk.flag = vagData[i++];
        //Get compressed sound data
        for (int b = i, ind = 0; b < i + 14; ++b, ++ind) vagChunk.data[ind] = vagData[b];
        i += 13;

        if (vagChunk.flag == PLAYBACK_END) break;
        else if (vagChunk.flag == LOOP_START) isLoopStart = true;
        else if (vagChunk.flag == LOOP_LAST_BLOCK) isLoopEnd = true;

        int16_t *samples = new int16_t[SAMPLE_NIBBLE];

        //Expand nibble to byte
        for (int b = 0; b < SAMPLE_BYTES; ++b) {
            samples[b * 2] = vagChunk.data[b] & 0x0F;
            samples[(b * 2) + 1] = (vagChunk.data[b] & 0xF0) >> 4;
        }

        //Decode samples
        for (int b = 0; b < SAMPLE_NIBBLE; ++b) {
            //Shift nibble to top range
            int16_t samp = samples[b] << 12;

            if ((samp & 0x8000) != 0) samp = int16_t(samp | 0xFFFF0000);

            double sample;
            sample = samp;
            sample = int16_t(sample) >> vagChunk.shift;
            sample += hist0 * vagLut[vagChunk.predict][0];
            sample += hist1 * vagLut[vagChunk.predict][1];

            hist1 = hist0;
            hist0 = sample;

            //Ensure new sample is not outside int16_t value range
            int16_t newSample;
            newSample = int16_t(min(INT16_MAX, max(int(round(sample)), INT16_MIN)));

            wavData.push_back(newSample);

            //Set looping points
            if (isLoopStart) {
                loopStart = wavData.size() - 1;
                isLoopStart = false;
            }
            else if (isLoopEnd) {
                loopEnd = wavData.size() - 1;
                isLoopEnd = false;
            }
        }

        delete[] samples;
    }

    //Set looping points if there are none
    if (loopStart == 0xFFFFFFFF) loopStart = 0;
    if (loopEnd == 0xFFFFFFFF) loopEnd = wavData.size() - 1;

    return wavData;
}


std::vector<int16_t> vgsDecode(char *vgsData, uint32_t vgsSize, int32_t &loopStart, int32_t &loopEnd) {
    vector<int16_t> wavData;



    return wavData;
}
