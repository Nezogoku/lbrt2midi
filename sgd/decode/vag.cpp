#include <string>
#include <cmath>
#include <climits>
#include <vector>
#include "vag.hpp"

using std::string;
using std::vector;
using std::round;
using std::max;
using std::min;


vector<int16_t> vagDecode(vector<char> vagData, uint32_t &loopStart, uint32_t &loopEnd) {
    vector<int16_t> wavData;

    bool isLooped = (loopStart == 0xFFFFFFFF && loopEnd == 0xFFFFFFFF) ? false : true;
    bool isLoopStart = false;
    bool isLoopEnd = false;

    double hist0 = 0.0,
           hist1 = 0.0;

    for (int i = 0; i < vagData.size(); ++i) {
        char decodingCoeff;
        decodingCoeff = vagData[i++];                                                       // Get decoding coefficient

        chunk vagChunk;
        vagChunk.shift = int8_t(decodingCoeff & 0xF);                                       // Get shift byte
        vagChunk.predict = int8_t((decodingCoeff & 0xF0) >> 4);                             // Get predicting byte
        vagChunk.flag = vagData[i++];                                                       // Get flag byte

        for (int b = i, ind = 0; b < i + 14; ++b, ++ind) vagChunk.data[ind] = vagData[b];   // Get compressed sound data
        i += 13;                                                                            // Play catch up

        if (vagChunk.flag == PLAYBACK_END) break;
        else if (isLooped && vagChunk.flag == LOOP_START) isLoopStart = true;
        else if (isLooped && vagChunk.flag == LOOP_END) isLoopEnd = true;

        int16_t *samples = new int16_t[SAMPLE_NIBBLE];

        ///Expand nibble to byte
        for (int b = 0; b < SAMPLE_BYTES; ++b) {
            samples[b * 2] = vagChunk.data[b] & 0xF;
            samples[(b * 2) + 1] = (vagChunk.data[b] & 0xF0) >> 4;
        }

        ///Decode samples
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
            newSample = int16_t(min(SHRT_MAX, max(int(round(sample)), SHRT_MIN)));

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

    vector<char> ().swap(vagData);


    //Set looping points if there are none
    if (!isLooped) {
        loopStart = 0;
        loopEnd = wavData.size() - 1;
    }

    return wavData;
}
