#ifndef PSXSGD_TYPES_HPP
#define PSXSGD_TYPES_HPP

#include <cstdint>
#include <string>
#include <vector>

#define PSX_SGXD 0x53475844
#define PSX_RGND 0x52474E44
#define PSX_SEQD 0x53455144
#define PSX_WAVE 0x57415645
#define PSX_NAME 0x4E414D45


//SGXD identifier
struct sgxd_head {
    uint32_t chunktype;                 // Chunk type: "SGXD"
    uint32_t name_offset;               // Address to bank name
    uint32_t data_offset;               // Address to start of sample data
    uint32_t length;                    // Length of sample data + 2147483648 (0x80000000) bytes, zero-padded
};

//RGND chunk header
//Storage for sound bank info
struct rgnd_head {
    uint32_t chunktype;                 // Chunk type: "RGND"
    uint32_t length;                    // Length of chunk

    uint32_t reserved;                  // Null
    uint32_t addr_start;                // Address to start of data

    uint32_t amount_pre;                // Number of programs
    uint32_t addr_pre_start;            // Address to start of program pre-definition data
    uint32_t addr_def_start;            // Address to start of definition data

    uint32_t amount;                    // Number of region definitions
};

//SEQD chunk header
//Storage for (sound effect) sequence data
struct seqd_head {
    uint32_t chunktype;                 // Chunk type: "SEQD"
    uint32_t length;                    // Length of chunk

    uint32_t reserved;                  // Null
    uint32_t addr_start;                // Address to start of data

    uint32_t amount;                    // Number of sequence definitions
};

//WAVE chunk header
//Storage for sample data info
struct wave_head {
    uint32_t chunktype;                 // Chunk type: "WAVE"
    uint32_t length;                    // Length of chunk

    uint32_t reserved;                  // Null
    uint32_t addr_start;                // Address to start of data

    uint32_t amount;                    // Number of wave definitions
    uint32_t addr_wave_start;           // Address to start of wave definitions
};

//NAME header
//Storage for bank and sample names
struct name_head {
    uint32_t chunktype;                 // Chunk type: "NAME"
    uint32_t length;                    // Length of chunk

    uint32_t reserved;                  // Null
    uint32_t addr_start;                // Address to start of data

    uint32_t amount;                    // Number of names
    uint32_t addr_name_start;           // Address to start of name definitions
};


//Program data
struct program_data {
    uint32_t instrument;
    uint32_t program_offset;
    uint8_t bank;
    uint8_t preset;
};

//RGND identifier
struct rgnd {
    program_data program;

    uint32_t priority;                  // The priority, usually 0x09
    uint32_t group_id;                  // Corresponding group
    uint32_t noise_clock;               // Likely the noise clock, usually 0x38
    uint32_t tone_type;                 // Might be tone type; 0x00 is instrument, >0x00 is SFX?
    uint32_t value_0;                   // Usually Null
    uint32_t value_1;                   // Usually Null
    uint8_t range_low;                  // Lowest note value
    uint8_t range_high;                 // Highest note value
    uint16_t reserved0;                 // Usually Null
    uint8_t root_key;                   // Root key
    int8_t fine_tune;                   // Fine tuning (cents)
    int16_t volume_0;                   // Might be volume
    int8_t mod_0;                       // Some modifier (Sustain and Release?)
    int8_t mod_1;                       //
    int8_t mod_2;                       // Some modifier (Attack and Decay?)
    int8_t pan;                         // Might be pan control
    int16_t volume_1;                   // Might be volume
    uint16_t reserved1;                 // Usually Null
    uint32_t port_travel;               // Might be Portamento travel time
    uint8_t mod_3;                      //
    uint8_t mod_4;                      //
    uint8_t mod_5;                      //
    uint8_t mod_6;                      //
    uint32_t final_val;                 // Currently being used as a type of terminator
    uint32_t sample_id;                 // ID of corresponding sample, noise if 0xFFFFFFFF

    int8_t semi_tune;                   // Additional tuning (semi-tones)
};

//SEQD identifier
struct seqd {
    uint32_t name_offset;               // Address to sequence name
    std::string name;                   // sequence name at offset
    uint32_t sequence_offset;           // Address to sequence data
    uint8_t sequence_format;            // sequence format, 0x00 is one track, 0x01 is one or more synchronous tracks, 0x02 is one or more individual tracks
    std::vector<char> sequence;         // sequence data
};

//WAVE identifier
//Based off of vgmstream
struct wave {
    uint32_t reserved0;                 // Voice ID
    uint32_t name_offset;               // Address to sample name, 0x00 = no name?
    uint8_t codec;                      // Codec of corresponding sample, 0x01 is PCM16 Big Endian/ 0x02 is Ogg Vorbis/ 0x03 is Sony ADPCM/ 0x04 is Atrac3Plus/ 0x05 is Sony Short ADPCM/ 0x06 is Sony Atrac3
    uint8_t channels;                   // Number of channels for corresponding sample
    uint16_t reserved1;                 // Null
    uint32_t sample_rate;               // Sample rate of corresponding sample
    uint32_t info_type;                 // Type of following value, 0x00 is null/ 0x30 and 0x40 is size of data/ 0x80 and 0x90 is block size
    uint32_t info_value;                // Value of info_type
    uint16_t volume_0;                  // Maximum volume
    uint16_t volume_1;                  // Minimum volume
    uint32_t reserved3;                 // Null
    uint32_t sample_size;               // Number of samples in sample
    uint32_t loop_start;                // Starting point of loop, 0xFF is no loop start
    uint32_t loop_end;                  // Ending point of loop, 0xFF is no loop end
    uint32_t stream_size;               // Size of stream in sample or interleave (type3)
    uint32_t stream_offset;             // Address to stream
    uint32_t stream_size_full;          // Size of stream in sample (zero-padded) or interleave (type3)

    std::string name;                   // Name of sample
    std::vector<char> data;             // Sample data
    std::vector<int16_t> pcmle;         // Converted sample data
    bool isLoop;                        // Sample is or is not looped
};

//NAME identifier
struct name {
    uint16_t name_id;                   // Name ID
    uint16_t name_type;                 // Name type, 0x0000 is Bank name, 0x2000 - 0x3000 is sequence and sample name
    uint32_t name_offset;               // Address to name
    std::string name;                   // Sample name at offset
};

#endif
