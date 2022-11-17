#ifndef DECODE_HPP
#define DECODE_HPP

#include <cstdint>      //For uintX_t, intX_t
#include <vector>       //For std::vector


///Decodes OGG-VORBIS files
std::vector<int16_t> oggDecode(char *oggData, uint32_t oggSize, int ch, uint32_t rt, uint32_t &loopStart, uint32_t &loopEnd);

///Decodes header-less VAG files
std::vector<int16_t> vagDecode(char *vagData, uint32_t vagSize, uint32_t &loopStart, uint32_t &loopEnd);


#endif
