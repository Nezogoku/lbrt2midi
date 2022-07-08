/**********************************************
            SGXD File Headers
**********************************************/

#ifndef SGXDHEADERS_HPP
#define SGXDHEADERS_HPP

#include <cstdint>

//SGXD identifier
struct sgxd_head {
    uint32_t chunktype;                 // Chunk type: "SGXD"
    uint32_t name_offset;               // Address to bank name
    uint32_t data_offset;               // Address to start of sample data
    uint32_t length;                    // Length of data + 2147483648 (0x80000000) bytes, zero-padded
};

//RGND chunk header
//Likely storage for sample bank (region?) info
struct rgnd_head {
    uint32_t chunktype;                 // Chunk type: "RGND"
    uint32_t length;                    // Length of chunk

    uint32_t reserved;                  // Null
    uint32_t amount_pre;                // Number of pre-definitions

    uint32_t addr_pre_start;            // Address to start of pre-definition data
    uint32_t addr_start;                // Address to start of definition data

    uint32_t amount;                    // Number of region definitions
};

//SEQD chunk header
//Likely storage for (sound effect?) sequence data
struct seqd_head {
    uint32_t chunktype;                 // Chunk type: "SEQD"
    uint32_t length;                    // Length of chunk

    uint32_t reserved;                  // Null

    uint32_t addr_start;                // Address to start of data
    uint32_t amount;                    // Number of sequences
};

//WAVE chunk header
//Storage for sample data info
struct wave_head {
    uint32_t chunktype;                 // Chunk type: "WAVE"
    uint32_t length;                    // Length of chunk

    uint32_t reserved;                  // Null
    uint32_t amount;                    // Number of wave definitions

    uint32_t addr_start;                // Address to start of data
};

//NAME header
//Storage for bank and sample names
struct name_head {
    uint32_t chunktype;                 // Chunk type: "NAME"
    uint32_t length;                    // Length of chunk

    uint32_t reserved;                  // Null
    uint32_t amount;                    // Number of names

    uint32_t addr_start;                // Address to start of data
};

#endif
