#ifndef DECODE_HPP
#define DECODE_HPP

#include <cstdint>      //For uintX_t, intX_t
#include <vector>       //For std::vector


///Decodes header-less PCMBE files
std::vector<int16_t> pcmbDecode(char *pcmbData, uint32_t pbeSize, uint32_t &loopStart, uint32_t &loopEnd);

///Decodes OGG-VORBIS files
std::vector<int16_t> oggDecode(char *oggData, uint32_t oggSize, int ch, uint32_t rt, uint32_t &loopStart, uint32_t &loopEnd);

///Decodes header-less VAG files
std::vector<int16_t> vagDecode(char *vagData, uint32_t vagSize, uint32_t &loopStart, uint32_t &loopEnd);

///Decodes AT3p files
std::vector<int16_t> atpDecode(char *atpData, uint32_t atpSize, int ch, uint32_t rt);

///Decodes header-less short VAG files
std::vector<int16_t> vgsDecode(char *vgsData, uint32_t vgsSize, uint32_t &loopStart, uint32_t &loopEnd);

///Decodes header-less AT3 files
std::vector<int16_t> at3Decode(char *at3Data, uint32_t at3Size, int ch, uint32_t rt);


#endif
