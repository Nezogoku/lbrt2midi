#include <algorithm>    //For std::min(), std::max()
#include <cmath>        //For std::floor()
#include <cstdint>      //For uintX_t, intX_t, INT16_MIN, INT16_MAX
#include <vector>       //For std::vector
#include "../decode.hpp"
#include "atrac_tables.hpp"

using std::round;
using std::max;
using std::min;
using std::floor;
using std::vector;


std::vector<int16_t> at3Decode(char *at3Data, uint32_t at3Size, int ch, uint32_t rt) {
	vector<int16_t> wavData;


    return wavData;
}


std::vector<int16_t> atpDecode(char *atpData, uint32_t atpSize, int ch, uint32_t rt) {
	vector<char> rawData;


    return at3Decode(rawData.data(), rawData.size(), ch, rt);
}
