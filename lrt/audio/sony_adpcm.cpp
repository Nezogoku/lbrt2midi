#include <cmath>
#include <climits>
#include <utility>
#include <vector>
#include "audio_func.hpp"


///Decodes Sony ADPCM
std::vector<short> decodeSonyAdpcm(unsigned char *in, const unsigned length, const unsigned smpls,
                                   const unsigned short chns, const bool is_short,
                                   signed *loop_b, signed *loop_e) {
    if (!in || length < 4) return {};

    const unsigned char *in_end = in + length;
    const unsigned short VAG_BLOCK_ALIGN = 16 / pow(4, is_short);
    const unsigned short VAG_BLOCK_SAMPLES = (VAG_BLOCK_ALIGN - pow(2, !is_short)) * 2;
    const double VAG_LOOKUP_TABLE[][2] = {
        {0.0, 0.0},
        {60.0 / 64.0, 0.0},
        {115.0 / 64.0, -52.0 / 64.0},
        {98.0 / 64.0, -55.0 / 64.0},
        {122.0 / 64.0, -60.0 / 64.0},
        {30.0 / 64.0, -0.0 / 64.0},
        {57.5 / 64.0, -26.0 / 64.0},
        {49.0 / 64.0, -27.5 / 64.0},
        {61.0 / 64.0, -30.0 / 64.0},
        {15.0 / 64.0, -0.0 / 64.0},
        {28.75/ 64.0, -13.0 / 64.0},
        {24.5 / 64.0, -13.75/ 64.0},
        {30.5 / 64.0, -15.0 / 64.0},
        {32.0 / 64.0, -60.0 / 64.0},
        {15.0 / 64.0, -60.0 / 64.0},
        {7.0 / 64.0, -60.0 / 64.0},
    };
    double hist[chns][2] {};
    std::vector<short> out;
    short *cur = 0, *end = 0;
    
    out.resize(smpls * chns);
    cur = out.data(); end = cur + out.size();

    for (int ba_i = 0; ba_i < length / (VAG_BLOCK_ALIGN * chns); ++ba_i) {
        int data[VAG_BLOCK_SAMPLES * chns] {};
        int num_s = sizeof(data) / sizeof(int);

        for (int ch_i = 0; ch_i < chns; ++ch_i) {
            unsigned char coef, flag;
            coef = *(in++);
            if (!is_short) flag = *(in++);

            for (int n = 0; n < VAG_BLOCK_SAMPLES; ++in) {
                data[(chns * n++) + ch_i] = in[0] & 0x0F;
                data[(chns * n++) + ch_i] = in[0] >> 4;
            }

            if (!is_short) {
                if (flag & 0x01 && loop_e && *loop_e < 0) *loop_e = (out.size() / chns) + VAG_BLOCK_SAMPLES - 1; // Loop stop
                if (flag & 0x04 && loop_b && *loop_b < 0) *loop_b = (out.size() / chns); // Loop start
                if (flag == 0x07) break; // End playback
            }

            for (int bs_i = 0; bs_i < VAG_BLOCK_SAMPLES; ++bs_i) {
                auto &smpl = data[(chns * bs_i) + ch_i];
                smpl <<= 12;
                if ((short)smpl < 0) smpl |= 0xFFFF0000;

                float tsmp;
                tsmp = smpl;
                tsmp = short(tsmp) >> (coef & 0x0F);
                tsmp += hist[ch_i][0] * VAG_LOOKUP_TABLE[coef >> 4][0];
                tsmp += hist[ch_i][1] * VAG_LOOKUP_TABLE[coef >> 4][1];

                hist[ch_i][1] = hist[ch_i][0];
                hist[ch_i][0] = tsmp;

                smpl = std::min(SHRT_MAX, std::max(int(std::round(tsmp)), SHRT_MIN));
            }
        }
        if (cur + num_s > end) num_s = end - cur;

        for (int d = 0; d < num_s; ++d) *cur++ = data[d];
    }

    if (cur < end) out.resize(cur - out.data());
    return std::move(out);
}
