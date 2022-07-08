#ifndef VAG_HPP
#define VAG_HPP

#include <vector>

#define SAMPLE_BYTES 14
#define SAMPLE_NIBBLE SAMPLE_BYTES * 2

///Flags for VAG flag byte
enum flags {
    NOTHING = 0,         /* Nothing*/
    LOOP_LAST_BLOCK = 1, /* Last block to loop */
    LOOP_REGION = 2,     /* Loop region*/
    LOOP_END = 3,        /* Ending block of the loop */
    LOOP_FIRST_BLOCK = 4,/* First block of looped data */
    UNK = 5,             /* ?*/
    LOOP_START = 6,      /* Starting block of the loop*/
    PLAYBACK_END = 7     /* Playback ending position */
};

///For decoding VAG files
static double vagLut[][2] = {{0.0, 0.0},
                            {60.0 / 64.0, 0.0},
                            {115.0 / 64.0, -52.0 / 64.0},
                            {98.0 / 64.0, -55.0 / 64.0},
                            {122.0 / 64.0, -60.0 / 64.0}
};

///Vag chunks
struct chunk {
    char shift;
    char predict;
    char flag;
    char data[SAMPLE_BYTES];
};

///Final loop data after conversion
struct finalLoop {
    uint32_t lpStrt;
    uint32_t lpEnd;
};

///Code for decoding header-less VAG files
///Specifically those found in SGXD files
std::vector<int16_t> vagDecode(std::vector<char> vagData, uint32_t &loopStart, uint32_t &loopEnd);

#endif
