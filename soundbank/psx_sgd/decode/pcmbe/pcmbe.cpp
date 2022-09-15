#include <cmath>
#include <climits>
#include <vector>
#include "../decode.hpp"

using std::vector;

std::vector<int16_t> pcmbDecode(char *pcmbeData, uint32_t pbeSize, int ch) {
    vector<int16_t> wavData;

    //Currently Mono or non-interleave only
    for (int i = 0; i < pbeSize; i += 2) {
        int16_t sample = (((int16_t)pcmbeData[i + 1]) << 8) | (0x00FF & pcmbeData[i]);

        //Ensure new sample is not outside int16_t value range
        int16_t newSample;
        newSample = int16_t(std::min(SHRT_MAX, std::max(int(round(sample)), SHRT_MIN)));

        wavData.push_back(newSample);
    }

    return wavData;
}
