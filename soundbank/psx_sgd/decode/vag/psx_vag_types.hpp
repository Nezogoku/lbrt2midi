#ifndef PSX_VAG_TYPES_HPP
#define PSX_VAG_TYPES_HPP

#include <cstdint>      //For uint8_t

#define SAMPLE_BYTES 14
#define SAMPLE_NIBBLE SAMPLE_BYTES * 2
#define SHORT_SAMPLE_BYTES 3
#define SHORT_SAMPLE_NIBBLE SHORT_SAMPLE_BYTES * 2


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
};

///VGS chunks
struct gchunk {
    uint8_t shift;
    uint8_t predict;
    int16_t data[SAMPLE_NIBBLE];
};


#endif
