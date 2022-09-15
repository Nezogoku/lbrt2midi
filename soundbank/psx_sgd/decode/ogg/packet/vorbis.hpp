#ifndef VORBIS_HPP
#define VORBIS_HPP

#include <cstdint>      //For uintX_t, intX_t
#include <vector>       //For std::vector


///Decodes VORBIS packets
std::vector<int16_t> vorbDecode(char *vrbData, uint32_t vrbSize, int ch, uint32_t rt);


#endif
