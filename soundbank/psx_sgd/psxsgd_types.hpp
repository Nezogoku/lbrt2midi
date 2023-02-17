#ifndef PSXSGD_TYPES_HPP
#define PSXSGD_TYPES_HPP

#include <cstdint>
#include <string>

#define MAX_PROGRAM 128
#define MAX_SAMPLE 254

//SGXD identifier
struct sgxd_head {
    uint32_t chunktype;                 // Chunk type: "SGXD"
    uint32_t name_offset;               // Address to bank name
    uint32_t data_offset;               // Address to start of sample data
    uint32_t data_length;               // Length of sample data bytes
    bool flag;                          // Flagella FLAGER FLAG
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

    uint32_t amount_pre;                // Number of sequence definitions
    uint32_t addr_pre_start;            // Address to start of sequence definitions

    uint32_t amount;                    // Number of sequences
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


//RGND tone definitions
struct tone {
    uint32_t priority;                  // The priority, usually 0x09
    uint32_t group_id;                  // Corresponding group
    uint32_t noise_clock;               // Likely the noise clock, usually 0x38
    uint8_t bank;                       // Might be bank ID
    uint8_t value_0;                    //
    uint8_t value_1;                    //
    uint8_t value_2;                    //
    uint32_t value_3;                   // Usually Null
    uint32_t value_4;                   // Usually Null
    uint8_t range_low;                  // Lowest note value
    uint8_t range_high;                 // Highest note value
    uint16_t reserved0;                 // Usually Null
    uint8_t root_key;                   // Root key
    int8_t fine_tune;                   // Fine tuning (cents)
    int16_t volume_0;                   // Might be volume
    //int8_t mod_0;                       // Some modifier (Sustain and Release?)
    int16_t mod_1;                      // Release (?)
    //int8_t mod_2;                       // Some modifier (Attack and Decay?)
    int16_t pan;                        // Pan
    int16_t volume_1;                   // Might be volume
    uint16_t reserved1;                 // Usually Null
    uint32_t port_travel;               // Might be Portamento travel time
    uint8_t mod_3;                      //
    uint8_t mod_4;                      //
    uint8_t mod_5;                      //
    uint8_t mod_6;                      //
    uint32_t final_val;                 // Currently being used as a type of terminator
    int32_t sample_id;                  // ID of corresponding sample, noise if 0xFFFFFFFF

    int8_t semi_tune;                   // Additional tuning (semi-tones)
    //uint8_t preset;                     // Preset ID
};

//RGND Program data
struct s_rgnd {
    uint32_t amount_tones;
    uint32_t tone_offset;

    tone *tones;
};


//SEQD sequence data
struct seq {
    std::string name;                   // Sequence name at offset
    uint8_t seq_format;                 // Sequence format
    unsigned char *seq_data;            // Sequence address
};

//SEQD sequence definitions
struct s_seqd {
    uint32_t reserved;                  // Null
    uint32_t amount;                    // Number of sequences

    seq *seqs;
};

//WAVE identifier
//Based off of vgmstream
struct s_wave {
    uint32_t voice_id;                  // Voice ID
    uint32_t name_offset;               // Address to sample name, 0x00 = no name?
    uint8_t codec;                      // Codec of corresponding sample
    uint8_t channels;                   // Number of channels for corresponding sample
    uint16_t reserved1;                 // Null
    uint32_t sample_rate;               // Sample rate of corresponding sample
    uint32_t info_type;                 // Type of following value
    uint32_t info_value;                // Value of info_type
    uint16_t volume_0;                  // Maximum volume?
    uint16_t volume_1;                  // Minimum volume?
    uint32_t reserved3;                 // Null
    uint32_t sample_size;               // Number of samples in sample
    int32_t loop_start;                 // Starting point of loop, -1 is no loop start
    int32_t loop_end;                   // Ending point of loop, -1 is no loop end
    uint32_t stream_size;               // Size of stream in sample or interleave (type3)
    uint32_t stream_offset;             // Address to stream
    uint32_t stream_size_full;          // Size of stream in sample (zero-padded) or interleave (type3)

    std::string name;                   // Name of sample
    unsigned char *data = 0;            // Sample data
    int16_t *pcmle = 0;                 // Converted sample data
    uint8_t loop_type = 0;              // Sample loop type (0 is none, 1 is fade, 2 is continuous)
};

//NAME identifier
struct s_name {
    uint16_t name_id;                   // Name ID
    uint16_t name_type;                 // Name type, 0x0000 is Bank name, 0x2000 - 0x3000 is sequence and sample name
    uint32_t name_offset;               // Address to name
    std::string name;                   // Sample name at offset

    //To make sorting in ascending order easier
    bool operator<(const s_name &n) const {
        return (name_type < n.name_type) || ((name_type == n.name_type) && (name_id < n.name_id));
    }
};


#endif
