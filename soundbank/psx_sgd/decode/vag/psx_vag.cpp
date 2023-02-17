#include <cstdint>
#include <cmath>
#include <climits>

#define SAMPLE_BYTES 14
#define SAMPLE_NIBBLE SAMPLE_BYTES * 2

///For decoding VAG files
double vagLut[][2] = {
    {0.0,           0.0},
    {60.0 / 64.0,   0.0},
    {115.0 / 64.0,  -52.0 / 64.0},
    {98.0 / 64.0,   -55.0 / 64.0},
    {122.0 / 64.0,  -60.0 / 64.0},

    {30.0 / 64.0 ,  -0.0 / 64.0},
    {57.5 / 64.0 ,  -26.0 / 64.0},
    {49.0 / 64.0 ,  -27.5 / 64.0},
    {61.0 / 64.0 ,  -30.0 / 64.0},
    {15.0 / 64.0 ,  -0.0 / 64.0},
    {28.75/ 64.0 ,  -13.0 / 64.0},
    {24.5 / 64.0 ,  -13.75/ 64.0},
    {30.5 / 64.0 ,  -15.0 / 64.0},
    {32.0 / 64.0 ,  -60.0 / 64.0},
    {15.0 / 64.0 ,  -60.0 / 64.0},
    {7.0 / 64.0 ,   -60.0 / 64.0}
};

///Flags for VAG flag byte
enum flags {
    NOTHING = 0,         /* Nothing*/
    LOOP_LAST_BLOCK = 1, /* Last block to loop */
    LOOP_REGION = 2,     /* Loop region*/
    LOOP_END = 3,        /* Ending block of the loop */
    LOOP_FIRST_BLOCK = 4,/* First block of looped data */
    UNK = 5,             /* Ending position?*/
    LOOP_START = 6,      /* Starting block of the loop*/
    PLAYBACK_END = 7     /* Playback ending position */
};

///VAG chunks
struct vchunk {
    uint8_t shift;
    uint8_t predict;
    uint8_t flag;
    uint8_t data[SAMPLE_BYTES];

    double hist[2] {};
};


int adpcmDecode(unsigned char *adpcmData, int adpcmSize, uint8_t &loopType, int16_t *&outData, int outSize) {
    outData = new int16_t[outSize];

    int mn_i = 0;

    bool isLoopStart = false;
    bool isLoopEnd = false;

    vchunk adpcmChunk;

    unsigned char *adpcmTerm = adpcmData + adpcmSize;
    while (adpcmData < adpcmTerm) {
        char coeff = *adpcmData++;                                                          // Get decoding coefficient

        adpcmChunk.shift = int8_t(coeff & 0xF);                                             // Get shift byte
        adpcmChunk.predict = int8_t((coeff & 0xF0) >> 4);                                   // Get predicting byte
        adpcmChunk.flag = *adpcmData++;                                                     // Get flag byte

        //Get compressed sound data
        for (int i = 0; i < SAMPLE_BYTES; ++i) adpcmChunk.data[i] = *adpcmData++;

        if (adpcmChunk.flag == PLAYBACK_END) break;
        else if (adpcmChunk.flag == LOOP_FIRST_BLOCK) {
            isLoopStart = true;
            //loop_s = (mn_i * 2) + 1;
        }
        else if (adpcmChunk.flag == LOOP_LAST_BLOCK) {
            isLoopEnd = true;
            //loop_e = (mn_i * 2) + (SAMPLE_NIBBLE * 2) + 1;
        }

        uint8_t samples[SAMPLE_NIBBLE] = {};

        //Expand nibble to byte
        for (int b = 0; b < SAMPLE_BYTES; ++b) {
            samples[(b * 2) + 0] = (adpcmChunk.data[b] & 0x0F) >> 0;
            samples[(b * 2) + 1] = (adpcmChunk.data[b] & 0xF0) >> 4;
        }

        //Decode samples
        for (int b = 0; b < SAMPLE_NIBBLE; ++b) {
            //Shift nibble to top range
            int16_t samp = samples[b];
            samp = samp << 12;

            double sample;
            sample = samp;
            sample = int16_t(sample) >> adpcmChunk.shift;
            sample += adpcmChunk.hist[0] * vagLut[adpcmChunk.predict][0];
            sample += adpcmChunk.hist[1] * vagLut[adpcmChunk.predict][1];

            adpcmChunk.hist[1] = adpcmChunk.hist[0];
            adpcmChunk.hist[0] = sample;

            //Ensure new sample is not outside int16_t value range
            int16_t newSample;
            newSample = int16_t(std::min(SHRT_MAX, std::max(int(std::round(sample)), SHRT_MIN)));

            if (mn_i >= outSize) break;
            outData[mn_i++] = newSample;
        }
    }

    loopType = (isLoopStart && isLoopEnd);

    return 1;
}
