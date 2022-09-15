#include <algorithm>    //For std::min(), std::max()
#include <cmath>        //For std::floor()
#include <cstdint>      //For uintX_t, intX_t, INT16_MIN, INT16_MAX
#include <string>       //For std::string
#include <vector>       //For std::vector
#include "../decode.hpp"
#include "../decode_utils.hpp"

using std::round;
using std::max;
using std::min;
using std::floor;
using std::string;
using std::vector;


std::vector<int16_t> oggDecode(char *oggData, uint32_t oggSize, int ch, uint32_t rt) {
    vector<int16_t> wavData;
    /* SECTION WORK IN PROGRESS
    int c = 0;

    //Iterate through ogg data
    while (c < oggSize) {
        string header = getChars(oggData, c, 4);
        bool isExten = false,
             isFirst = false,
             isLast  = false;

        if (header == "OggS") {
            c += 4;

            if (oggData[c++]) break;

            if (oggData[c] & 0x01) isExten = true;
            if (oggData[c] & 0x02) isFirst = true;
            if (oggData[c] & 0x04) isLast  = true;
        }
        else c += 1;
    }
    */

    return wavData;
}
