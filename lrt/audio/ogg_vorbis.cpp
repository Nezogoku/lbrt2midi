#include <cmath>
#include <climits>
#include <utility>
#include <vector>
#define STB_VORBIS_NO_CRT
#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_NO_PUSHDATA_API
#define STB_VORBIS_NO_INTEGER_CONVERSION
#define STB_VORBIS_NO_FAST_SCALED_FLOAT
#define STB_VORBIS_NO_COMMENTS
#define STB_VORBIS_MAX_CHANNELS 6
#include "stb_vorbis/stb_vorbis.h"
#include "audio_func.hpp"

///Decodes Ogg Vorbis
std::vector<short> decodeOgg(unsigned char *in, const unsigned length, const unsigned smpls,
                             unsigned short *chns) {
    if (!in || length < 4) return {};
    
    const int BUFFER_SIZE = 8192;
    float **ptr0;
    short buf0[BUFFER_SIZE] {}, *ptr1;
    int num_s, num_c;
    std::vector<short> out;
    short *cur = 0, *end = 0;
    
    auto set_perm = [](float **in, short *out, const int num_c, const int num_s) -> short* {
        if (num_c > STB_VORBIS_MAX_CHANNELS) return 0;
        
        const char PERMUTE[][STB_VORBIS_MAX_CHANNELS] = {
            {},
            {0, 1},
            {0, 2, 1},
            {0, 1, 2, 3},
            {0, 1, 2, 3, 4},
            {0, 2, 1, 5, 3, 4},
        };
        
        for (int s = 0; s < num_s; ++s) {
            for (int c = 0; c < num_c; ++c) {
                out[s * num_c + PERMUTE[num_c - 1][c]] = std::min(
                    SHRT_MAX,
                    std::max(int(std::floor(in[c][s] * 32767.0f + 0.5f)), SHRT_MIN)
                );
            }
        }
        
        return out;
    };
    
    auto *vorb = stb_vorbis_open_memory(in, length, nullptr, nullptr);
    if (!vorb) return {};
    
    num_c = stb_vorbis_get_info(vorb).channels;
    if (chns) *chns = num_c;
    
    out.resize(smpls * num_c);
    cur = out.data(); end = cur + out.size();
    
    while ((num_s = stb_vorbis_get_frame_float(vorb, &num_c, &ptr0)) > 0) {
        ptr1 = set_perm(ptr0, buf0, num_c, num_s);
        if (ptr1) {
            num_s *= num_c;
            if (cur + num_s > end) num_s = end - cur;
            
            for (int d = 0; d < num_s; ++d) *cur++ = *ptr1++;
        }
    }
    
    stb_vorbis_close(vorb);
    
    if (cur < end) out.resize(cur - out.data());
    return std::move(out);
}
