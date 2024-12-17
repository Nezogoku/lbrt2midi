#include <vector>
#include "fourcc_type.hpp"
#include "chunk_type.hpp"
#include "riff_forms.hpp"
#include "riff_func.hpp"


///Unpacks subchunk from RIFF data
void unpackRiff(unsigned char *in, const unsigned length) {
    riff_inf.riff = {};
    if (!in || length < 12) return;

    const unsigned char *in_end = in + length;
    EndianType endian;
    bool is_rv;
    fourcc t_fc;
    unsigned t_sz;

    auto get_fcc = [&in]() -> unsigned {
        unsigned out = 0;
        for (int t = 0; t < 4; ++t) out = (out << 8) | *(in++);
        return out;
    };
    auto get_int = [&in, &endian]() -> unsigned {
        unsigned out = 0;
        for (int i = 0; i < 4; ++i) {
            switch (endian) {
                case ENDIAN_BIG:
                    out = (out << 8) | *(in++);
                    continue;
                case ENDIAN_LITTLE:
                    out |= (unsigned)*(in++) << (8 * i);
                    continue;
            }
        }
        return out;
    };

    t_fc = get_fcc();
    if (t_fc != FOURCC_RIFF && t_fc != FOURCC_RIFX) return;
    else if (t_fc < FOURCC_RIFF) { is_rv = false; endian = ENDIAN_LITTLE; }
    else if (t_fc < FOURCC_RIFX) { is_rv = false; endian = ENDIAN_BIG;    }
    else if (t_fc > FOURCC_RIFX) { is_rv = true;  endian = ENDIAN_LITTLE; }
    else if (t_fc > FOURCC_RIFF) { is_rv = true;  endian = ENDIAN_BIG;    }

    t_sz = get_int();
    if (in + t_sz > in_end) return;

    riff_inf.riff.setRev(is_rv);
    riff_inf.riff.setEnd(endian);
    riff_inf.riff.setFcc(get_fcc());
    riff_inf.riff.setArr(in, t_sz - 4);
}

///Unpacks subchunk from RIFF chunk
void unpackRiff(const chunk chnk) {
    unpackRiff(chnk.getAll().data(), chnk.size());
}
