/**********************************************
            SGXD File Chunks
**********************************************/

#ifndef SGXDCHK_HPP
#define SGXDCHK_HPP

#include <string>
#include <cstdint>

//RGND identifier
struct rgnd {
    struct program_data {
        uint8_t bank;
        uint8_t preset;
    } program;

    uint32_t determinator;              // 0x09 is usually regional info
    uint32_t value0;                    // Usually Null
    uint32_t value1;                    // Usually 0x38
    uint32_t value2;                    // Usually 0x64, sometimes 0x00 if SEQD not present
    uint32_t value3;                    // Usually Null
    uint32_t value4;                    // Usually Null
    uint8_t range_low;                  // Lowest note value
    uint8_t range_high;                 // Highest note value
    uint16_t reserved0;                 // Usually Null
    uint8_t root_key;                   // Root key
    uint16_t value5;                    //
    uint8_t value6;                     //
    uint32_t value7;                    //
    uint32_t value8;                    // Usually 4096
    uint32_t value9;                    //
    uint32_t valueA;                    //
    uint32_t final_val;                 // Currently being used as a type of terminator
    uint32_t sample_id;                 // ID of corresponding sample
};

//SEQD identifier
//Feels like Assembly... It's crazy
struct seqd {
    uint32_t name_offset;               // Offset to sequence name
    std::string name;                   // sequence name at offset
    std::vector<char> sequence;         // sequence data
};

//WAVE identifier
//Based off of vgmstream
struct wave {
    uint32_t reserved0;                 // Null
    uint32_t name_offset;               // Offset to sample name, 0x00 = no name
    uint8_t codec;                      // Codec of corresponding sample, 0x03 is IEEE Float
    uint8_t channels;                   // Number of channels for corresponding sample
    uint16_t reserved1;                 // Null
    uint32_t sample_rate;               // Sample rate of corresponding sample
    uint32_t info_type;                 // Type of following value, 0x00 is null/ 0x30 and 0x40 is size of data/ 0x80 and 0x90 is block size
    uint32_t info_value;                // Value of info_type
    uint32_t reserved2;                 //
    uint32_t reserved3;                 // Null
    uint32_t sample_size;               // Number of samples in sample
    uint32_t loop_start;                // Starting point of loop, 0xFF is no loop start
    uint32_t loop_end;                  // Ending point of loop, 0xFF is no loop end
    uint32_t stream_size;               // Size of stream in sample or interleave (type3)
    uint32_t stream_offset;             // Offset of stream
    uint32_t stream_size_full;          // Size of stream in sample (zero-padded) or interleave (type3)
};

//NAME identifier
struct name {
    uint16_t identifier;                // Sample ID
    uint16_t reserved0;                 //
    uint32_t name_offset;               // Offset to sample name
    std::string name;                   // Sample name at offset
};

#endif
