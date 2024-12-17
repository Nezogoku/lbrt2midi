#include <algorithm>
#include <bitset>
#include <cstdio>
#include <vector>
#ifdef DECODESONYAT3P_IMPLEMENTATION
#define NEEDEDRIFFWAVE_IMPLEMENTATION
#endif
#include "sgxd_const.hpp"
#include "sgxd_types.hpp"
#include "sgxd_func.hpp"
#include "audio/audio_func.hpp"
#include "riff/fourcc_type.hpp"
#include "riff/chunk_type.hpp"
#include "riff/uuid_type.hpp"
#include "riff/riff_forms.hpp"
#include "riff/riff_func.hpp"
#include "riff/riffwave_forms.hpp"
#include "riff/riffwave_const.hpp"
#include "riff/riffwave_types.hpp"
#include "riff/riffwave_func.hpp"


#ifdef DECODESONYAT3P_IMPLEMENTATION
///SGXD AT3P GUID
MAKEUUID(WAVE_GUID_SONYATRAC3PLUS, 0xE923AABF, 0xCB58, 0x4471, 0xA119FFFA01E4CE62);
#endif

///Unpacks variable waveform definitions from WAVE data
void unpackWave(unsigned char *in, const unsigned length) {
    if (sgd_debug) fprintf(stderr, "    Unpack WAVE\n");
    
    sgd_inf.wave = {};
    if (!sgd_beg || !sgd_dat_beg || !sgd_dat_end || !in || length < 8) return;

    const unsigned char *in_end = in + length;
    unsigned t_sz;
    auto &out = sgd_inf.wave;

    auto get_int = [&in, &in_end](const unsigned length) -> unsigned {
        unsigned out = 0;
        for (int i = 0; i < length; ++i) {
            if (in >= in_end) break;
            out |= (unsigned)*(in++) << (8 * i);
        }
        return out;
    };
    auto get_str = [](const unsigned char *in, const unsigned adr) -> std::string {
        std::string out;
        if (!adr);
        else { out = (const char*)(in + adr); if (!out[0]) out.clear(); }
        return out;
    };

    if (sgd_debug) fprintf(stderr, "    Read WAVE Header\n");
    out.flag = get_int(4);
    out.wave.resize(get_int(4));
    signed tinf[out.wave.size()][4] {};

    if (sgd_debug) fprintf(stderr, "        Global flag: 0x%08X\n", out.flag);

    if (sgd_debug) fprintf(stderr, "    Read WAVE Definition\n");
    for (auto &w : out.wave) {
        w.flag = get_int(4);
        w.name = get_str(sgd_beg, get_int(4));
        tinf[&w - out.wave.data()][0] = *(in++);
        w.chns = *(in++);
        w.numloop = *(in++);
        in += 1;
        w.smprate = get_int(4);
        w.rate0 = get_int(4);
        w.rate1 = get_int(4);
        w.volleft = get_int(2);
        w.volright = get_int(2);
        w.looppos = get_int(4);
        w.loopsmp = get_int(4);
        w.loopbeg = get_int(4);
        w.loopend = get_int(4);
        tinf[&w - out.wave.data()][1] = get_int(4);
        tinf[&w - out.wave.data()][2] = get_int(4);
        tinf[&w - out.wave.data()][3] = get_int(4);
        
        if (sgd_debug) {
            fprintf(stderr, "        Current waveform: %d\n", &w - out.wave.data());
            fprintf(stderr, "            Local flag: 0x%08X\n", w.flag);
            fprintf(stderr, "            Name: %s\n", w.name.c_str());
            fprintf(stderr, "            Channels: %d\n", w.chns);
            fprintf(stderr, "            Loop amount: %d\n", w.numloop);
            fprintf(stderr, "            Sample rate: %d\n", w.smprate);
            fprintf(stderr, "            Bit rate: %d\n", w.rate0);
            fprintf(stderr, "            Byte rate: %d\n", w.rate1);
            fprintf(stderr, "            Left volume: %d\n", w.volleft);
            fprintf(stderr, "            Right volume: %d\n", w.volright);
            fprintf(stderr, "            Loop position: %d\n", w.looppos);
            fprintf(stderr, "            Loop samples: %d\n", w.loopsmp);
            fprintf(stderr, "            Loop begin: %d\n", w.loopbeg);
            fprintf(stderr, "            Loop end: %d\n", w.loopend);
        }
    }

    if (sgd_debug) fprintf(stderr, "    Decode WAVE\n");
    for (int w = 0; w < out.wave.size(); ++w) {
        if (sgd_debug) fprintf(stderr, "        Current waveform: %d\n", w);

        switch(tinf[w][0] & 0xFF) {
#if 1
            case SGXD_CODEC_PCM16LE:
            case SGXD_CODEC_PCM16BE:
                if (sgd_debug) fprintf(
                    stderr,
                    "            Decode 16bit %s Endian PCM\n",
                    (tinf[w][0] & 0xFF) == SGXD_CODEC_PCM16LE ? "Little" : "Big"
                );
                in = (unsigned char*)sgd_dat_beg + tinf[w][2];
                for (int d = 0; d < tinf[w][1]; ++d) {
                    if ((tinf[w][0] & 0xFF) == SGXD_CODEC_PCM16LE) out.wave[w].pcm.push_back(get_int(2));
                    else { out.wave[w].pcm.push_back((short)in[0] << 8 | in[1]); in += 2; }
                }
                break;
#endif
#ifdef DECODESONYADPCM_IMPLEMENTATION
            case SGXD_CODEC_SONY_ADPCM:
            case SGXD_CODEC_SONY_SHORT_ADPCM:
                if (sgd_debug) fprintf(
                    stderr,
                    "            Decode Sony %s\n",
                    (tinf[w][0] & 0xFF) == SGXD_CODEC_SONY_SHORT_ADPCM ? "Short ADPCM" : "ADPCM"
                );
                out.wave[w].pcm = decodeSonyAdpcm(
                    (unsigned char*)sgd_dat_beg + tinf[w][2], tinf[w][1], out.wave[w].loopsmp,
                    out.wave[w].chns, (tinf[w][0] & 0xFF) == SGXD_CODEC_SONY_SHORT_ADPCM
                );
                break;
#endif
#ifdef DECODESONYAT3P_IMPLEMENTATION
            case SGXD_CODEC_SONY_ATRAC3PLUS:
                if (sgd_debug) fprintf(stderr, "            Decode Sony Atrac3+\n");
                unpackRiff((unsigned char*)sgd_dat_beg + tinf[w][2], tinf[w][1]);
                unpackRiffWave(riff_inf.riff);
                if (wav_inf.fmt.guid != WAVE_GUID_SONYATRAC3PLUS) continue;
                else if (wav_inf.wavl.empty()) continue;
                out.wave[w].pcm = decodeSonyAt3p(
                    wav_inf.wavl[0].chnk.getArr().data(),
                    wav_inf.wavl[0].chnk.size() - 8, out.wave[w].loopsmp,
                    wav_inf.fmt.align, out.wave[w].chns,
                    (!wav_inf.fact.smpinfo.empty()) ? wav_inf.fact.smpinfo[0] : 0
                );
                break;
#endif
#ifdef DECODEDOLBYAC3_IMPLEMENTATION
            case SGXD_CODEC_DOLBY_AC_3:
                if ((sgd_dat_beg + tinf[w][2])[0] == 0x4F &&
                    (sgd_dat_beg + tinf[w][2])[1] == 0x67 &&
                    (sgd_dat_beg + tinf[w][2])[2] == 0x67 &&
                    (sgd_dat_beg + tinf[w][2])[3] == 0x53);
                else {
                    if (sgd_debug) fprintf(stderr, "            Decode Dolby AC-3\n");
                    out.wave[w].pcm = decodeDolbyAc3(
                        (unsigned char*)sgd_dat_beg + tinf[w][2], tinf[w][1],
                        out.wave[w].loopsmp, out.wave[w].rate1, out.wave[w].chns
                    );
                    break;
                }
#endif
#ifdef DECODEOGG_IMPLEMENTATION
            case SGXD_CODEC_OGG_VORBIS:
                if (sgd_debug) fprintf(stderr, "            Decode Ogg-Vorbis\n");
                out.wave[w].pcm = decodeOgg(
                    (unsigned char*)sgd_dat_beg + tinf[w][2], tinf[w][1],
                    out.wave[w].loopsmp, (unsigned short*)&out.wave[w].chns
                );
                break;
#endif
            case SGXD_CODEC_UNKNOWN0:
            case SGXD_CODEC_UNKNOWN1:
            default:
                if (sgd_debug) fprintf(stderr, "            Codec 0x%02X\n", tinf[w][0] & 0xFF);
                break;
        }
        
        if (out.wave[w].loopbeg < 0) out.wave[w].loopbeg = out.wave[w].loopsmp;
        if (out.wave[w].loopend < 0) out.wave[w].loopend = out.wave[w].loopsmp;
        
        if (!out.wave[w].pcm.empty()) {
            if (
                std::all_of(
                    out.wave[w].pcm.begin(),
                    out.wave[w].pcm.end(),
                    [](const short &s) { return !s; }
                )
            ) out.wave[w].pcm.clear();
            else if (sgd_debug) fprintf(stderr, "            Audio decode successful\n");
        }
    }
}

///Packs specified waveform into waveform data
std::vector<unsigned char> waveToWave(const int &wav) {
    if (sgd_debug) fprintf(stderr, "    Extract WAV\n");
    
    wav_inf = {};
    if (
        sgd_inf.wave.empty() ||
        wav < 0 || wav >= sgd_inf.wave.wave.size() ||
        sgd_inf.wave.wave[wav].pcm.empty()
    ) return {};

    const auto &wv = sgd_inf.wave.wave[wav];

    if (sgd_debug) fprintf(stderr, "        Set format fields to waveform\n");
    //Setup format fields
    wav_inf.fmt.codec = CODEC_PCM;
    wav_inf.fmt.chns = wv.chns;
    wav_inf.fmt.smprate = wv.smprate;
    wav_inf.fmt.bytrate = wv.smprate * 2;
    wav_inf.fmt.align = 2;
    wav_inf.fmt.bitrate = 16;
    if (wv.chns > 2) {
        wav_inf.fmt.codec = CODEC_EXTENSIBLE;
        wav_inf.fmt.smpinfo = 16;
        switch(wv.chns) {
            case 3:
                wav_inf.fmt.chnmask = SPKR_FL | SPKR_FR | SPKR_FC;
                break;
            case 4:
                wav_inf.fmt.chnmask = SPKR_FL | SPKR_FR |
                                      SPKR_BL | SPKR_BR;
                break;
            case 5:
                wav_inf.fmt.chnmask = SPKR_FL | SPKR_FR |
                                      SPKR_FC |
                                      SPKR_BL | SPKR_BR;
                break;
            case 6:
                wav_inf.fmt.chnmask = SPKR_FL | SPKR_FR |
                                      SPKR_FC | SPKR_LF |
                                      SPKR_BL | SPKR_BR;
                break;
            case 7:
                wav_inf.fmt.chnmask = SPKR_FL | SPKR_FR |
                                      SPKR_FC | SPKR_LF |
                                      SPKR_BC |
                                      SPKR_BL | SPKR_BR;
                break;
            case 8:
                wav_inf.fmt.chnmask = SPKR_FL | SPKR_FR |
                                      SPKR_FC | SPKR_LF |
                                      SPKR_SL | SPKR_SR |
                                      SPKR_BL | SPKR_BR;
                break;
            default:
                wav_inf.fmt.chnmask = SPKR_ALL;
                break;
        }
        wav_inf.fmt.guid = WAVE_GUID_PCM;
    }

    if (sgd_debug) fprintf(stderr, "        Set waveform data to waveform\n");
    //Setup data field
    wav_inf.wavl.emplace_back(wv.pcm);

    if (sgd_debug) fprintf(stderr, "        Set sampler info to waveform\n");
    //Setup sampler fields
    wav_inf.smpl.emplace_back();
    wav_inf.smpl.back().smpperiod = (1.00 / wv.smprate) * 1000000000;
    wav_inf.smpl.back().noteroot = 60;
    if (wv.loopbeg != wv.loopend) wav_inf.smpl.back().loops.emplace_back();
    for (auto &lp : wav_inf.smpl.back().loops) {
        lp.loopid = 0;
        lp.looptyp = 0;
        lp.loopbeg = wv.loopbeg;
        lp.loopend = wv.loopend;
        lp.loopres = 0;
        lp.loopfrq = wv.numloop;
    }

    return packRiffWave();
}

///Extracts variable waveform definitions into string
std::string extractWave() {
    if (sgd_debug) fprintf(stderr, "    Extract WAVE info\n");
    
    std::string out;
    auto set_fstr = [&out]<typename... T>(const char *in, T&&... args) -> void {
        int s0 = snprintf(nullptr, 0, in, args...) + 1, s1 = out.size();
        out.resize(s1 + s0 - 1); snprintf(out.data() + s1, s0, in, args...);
    };

    set_fstr("Global Flags: %s\n", std::bitset<32>(sgd_inf.wave.flag).to_string().c_str());
    set_fstr("Waveforms:\n");
    for (const auto &w : sgd_inf.wave.wave) {
        set_fstr("    Waveform: %d\n", &w - sgd_inf.wave.wave.data());
        set_fstr("        Waveform Flags: %s\n", std::bitset<32>(w.flag).to_string().c_str());
        set_fstr("        Name: %s\n", (!w.name.empty()) ? w.name.c_str() : "(none)");
        set_fstr("        Channels: %d\n", w.chns);
        set_fstr("        Number Loops: %d\n", w.numloop);
        set_fstr("        Sample Rate: %d\n", w.smprate);
        set_fstr("        Bit Rate: %d\n", w.rate0);
        set_fstr("        Byte Rate: %d\n", w.rate1);
        set_fstr("        Left Volume: %d\n", w.volleft);
        set_fstr("        Right Volume: %d\n", w.volright);
        set_fstr("        Loop Position: %d\n", w.looppos);
        set_fstr("        Loop Samples: %d\n", w.loopsmp);
        set_fstr("        Loop Begin: %d\n", w.loopbeg);
        set_fstr("        Loop Size: %d\n", w.loopend);
    }

    return out;
}
