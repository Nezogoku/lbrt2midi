#ifndef DECODE_DEFINES_HPP
#define DECODE_DEFINES_HPP

#include <cstdint>      //For uintX_t, intX_t


//Defines for PSX VAG
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

///VAG chunks
struct chunk {
    uint8_t shift;
    uint8_t predict;
    uint8_t flag;
    uint8_t data[SAMPLE_BYTES];
};

#endif
