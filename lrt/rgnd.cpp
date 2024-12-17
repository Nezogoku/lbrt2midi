#include <algorithm>
#include <bitset>
#include <cstdio>
#include <cmath>
#include <string>
#include <vector>
#include "sgxd_types.hpp"
#include "sgxd_func.hpp"
#include "riff/riff_forms.hpp"
#include "riff/riffsfbk_forms.hpp"
#include "riff/riffsfbk_const.hpp"
#include "riff/riffsfbk_types.hpp"
#include "riff/riffsfbk_func.hpp"
#include "midi/midi_const.hpp"


///Unpacks variable region definitions from RGND data
void unpackRgnd(unsigned char *in, const unsigned length) {
    if (sgd_debug) fprintf(stderr, "    Unpack RGND\n");
    
    sgd_inf.rgnd = {};
    if (!sgd_beg || !in || length < 8) return;

    const unsigned char *in_end = in + length;
    unsigned t_sz;
    auto &out = sgd_inf.rgnd;

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

    if (sgd_debug) fprintf(stderr, "    Read RGND Header\n");
    out.flag = get_int(4);
    out.rgnd.resize(get_int(4));
    signed rgnoffs[out.rgnd.size()] {};

    if (sgd_debug) fprintf(stderr, "        Global flag: 0x%08X\n", out.flag);

    if (sgd_debug) fprintf(stderr, "    Read RGND Setup\n");
    for (int r = 0; r < out.rgnd.size(); ++r) {
        out.rgnd[r].resize(get_int(4));
        rgnoffs[r] = get_int(4);
    }

    if (sgd_debug) fprintf(stderr, "    Read RGND Region\n");
    for (int r = 0; r < out.rgnd.size(); ++r) {
        in = (unsigned char*)sgd_beg + rgnoffs[r];

        if (sgd_debug) fprintf(stderr, "        Current region: %d\n", r);
        for (auto &t : out.rgnd[r]) {
            t.flag = get_int(4);
            t.name = get_str(sgd_beg, get_int(4));
            t.rgnsiz = get_int(4);
            if (t.rgnsiz < 56) { in += 56 - t.rgnsiz - 12; continue; }
            else if (t.rgnsiz > 56) { in += t.rgnsiz - 56 - 12; continue; }
            t.voice = *(in++);
            t.excl = *(in++);
            t.bnkmode = *(in++);
            t.bnkid = *(in++);
            in += 4;
            t.effect = get_int(4);
            t.notelow = *(in++);
            t.notehigh = *(in++);
            in += 2;
            t.noteroot = *(in++);
            t.notetune = *(in++);
            t.notepitch = get_int(2);
            t.vol0 = get_int(2);
            t.vol1 = get_int(2);
            t.gendry = get_int(2);
            t.genwet = get_int(2);
            t.env0 = get_int(4);
            t.env1 = get_int(4);
            t.vol = *(in++);
            t.pan = *(in++);
            t.bendlow = *(in++);
            t.bendhigh = *(in++);
            t.smpid = get_int(4);

            if (sgd_debug) {
                fprintf(stderr, "            Current tone: %d\n", &t - out.rgnd[r].data());
                fprintf(stderr, "                Local flag: 0x%08X\n", t.flag);
                fprintf(stderr, "                Name: %s\n", t.name.c_str());
                fprintf(stderr, "                Region size: %d\n", t.rgnsiz);
                fprintf(stderr, "                Priority: %d\n", t.voice);
                fprintf(stderr, "                Group ID 1: %d\n", t.excl);
                fprintf(stderr, "                Group mode: %d\n", t.bnkmode);
                fprintf(stderr, "                Group ID 2: %d\n", t.bnkid);
                fprintf(stderr, "                Effect: %d\n", t.effect);
                fprintf(stderr, "                Note low: %d\n", t.notelow);
                fprintf(stderr, "                Note high: %d\n", t.notehigh);
                fprintf(stderr, "                Note root: %d\n", t.noteroot);
                fprintf(stderr, "                Note tune: %d\n", t.notetune);
                fprintf(stderr, "                Note pitch: %d\n", t.notepitch);
                fprintf(stderr, "                Volume 1: %d\n", t.vol0);
                fprintf(stderr, "                Volume 2: %d\n", t.vol1);
                fprintf(stderr, "                Generator dry: %d\n", t.gendry);
                fprintf(stderr, "                Generator wet: %d\n", t.genwet);
                fprintf(stderr, "                Envelope 1: %d\n", t.env0);
                fprintf(stderr, "                Envelope 2: %d\n", t.env1);
                fprintf(stderr, "                Volume: %d\n", t.vol);
                fprintf(stderr, "                Pan: %d\n", t.pan);
                fprintf(stderr, "                Bend low: %d\n", t.bendlow);
                fprintf(stderr, "                Bend high: %d\n", t.bendhigh);
                fprintf(stderr, "                Sample ID: %d\n", t.smpid);
            }
        }
    }
}

///Packs variable region definitions into soundfont data
std::vector<unsigned char> rgndToSfbk() {
    if (sgd_debug) fprintf(stderr, "    Extract SF2\n");
    
    sf2_inf = {};
    if (
        sgd_inf.file.empty() ||
        sgd_inf.rgnd.empty() ||
        sgd_inf.wave.empty()
    ) return {};
    
    struct prst { unsigned short bid, pid; std::vector<baginfo> zon; };
    std::vector<prst> prsts;

    auto set_nam = []<typename... T>(char *out, const int length, const char *in, T&&... args) -> void {
        snprintf(out, length + 1, in, args...);
    };
    auto get_prs = [&prsts](unsigned short bid, unsigned short pid) -> std::vector<prst>::iterator {
        auto itr = prsts.begin();
        itr = std::find_if(itr, prsts.end(), [&bid, &pid](const prst &p) { return p.bid == bid && p.pid == pid; });
        if (itr >= prsts.end()) { prsts.emplace_back(bid, pid); itr = prsts.end() - 1; }
        return itr;
    };

    if (sgd_debug) fprintf(stderr, "        Set info to soundbank\n");
    
    //Version Level
    if (sgd_debug) fprintf(stderr, "            Set version\n");
    sf2_inf.setIfil(2, 4);
    
    //Sound Engine
    if (sgd_debug) fprintf(stderr, "            Set sound engine\n");
    sf2_inf.setIsng("EMU8000");
    
    //Title
    if (sgd_debug) fprintf(stderr, "            Set title\n");
    sf2_inf.setInam(sgd_inf.file.c_str());
    
    //Software Package
    if (sgd_debug) fprintf(stderr, "            Set software\n");
    sf2_inf.setIsft(PROGRAMME_IDENTIFIER);

    if (sgd_debug) fprintf(stderr, "        Set samples to soundbank\n");
    const int siz = sgd_inf.wave.wave.size();
    for (int w = 0; w < siz; ++w) {
        const auto &wav = sgd_inf.wave.wave[w];
        const auto &pcm = (wav.chns != 1) ? std::vector<short>{} : wav.pcm;
        const auto &lpb = (pcm.empty()) ? 0 : wav.loopbeg;
        const auto &lpe = (pcm.empty()) ? 0 : wav.loopend;
        char nam[SFBK_NAME_MAX + 1] {};
        
        if (sgd_debug) fprintf(stderr, "            Set sample %d and header\n", w);
        if (wav.chns != 1) set_nam(nam, SFBK_NAME_MAX, "empt_%03d", w);
        else if (!wav.name.empty()) set_nam(nam, SFBK_NAME_MAX, wav.name.c_str());
        else set_nam(nam, SFBK_NAME_MAX, "smpl_%03d", w);
        
        sf2_inf.setShdr(nam, pcm, lpb, lpe, wav.smprate, 60, 0, 0, ST_RAM_MONO);
    }

    if (sgd_debug) fprintf(stderr, "        Set instruments and presets to soundbank\n");
    for (const auto &rgn : sgd_inf.rgnd.rgnd) {
        
        //new_val = (((old_val - old_min) * (new_max - new_min)) / (old_max - old_min)) + new_min

        prsts.clear();
        for (const auto &ton : rgn) {
            const auto &wav = sgd_inf.wave.wave[ton.smpid];
            const int i = sf2_inf.getInum();
            auto itr = get_prs(ton.bnkid, &rgn - sgd_inf.rgnd.rgnd.data());
            std::vector<geninfo> tgn;
            std::vector<modinfo> tmd;

            char nam[SFBK_NAME_MAX + 1] {};

            if (sgd_debug) fprintf(stderr, "            Set instrument %d header and zones\n", i);
            if (!ton.name.empty()) set_nam(nam, SFBK_NAME_MAX, ton.name.c_str());
            else set_nam(nam, SFBK_NAME_MAX, "inst_%03d", i);
            
            sf2_inf.setInst(
                nam, std::vector<baginfo>{{
                    std::vector<geninfo>{
                        {GN_KEY_RANGE, ton.notelow, ton.notehigh}, // Key range
                        //{GN_MODULATION_ENV_FINE_PITCH, ton.notepitch}, // Pitch
                        {GN_CHORUS_EFFECTS_SEND, ton.effect}, // Chorus
                        {GN_REVERB_EFFECTS_SEND, ((ton.genwet - ton.gendry) * 1000) / 4096.00}, // Reverb
                        {GN_DRY_PAN, (((ton.vol1 + 1024) * 1000) / 2048.00) - 500}, // Dry pan
                        {GN_VOLUME_ENV_RELEASE, 1200 * log2(ton.env0 / 100.00)}, // Envelope 1
                        {GN_MODULATION_ENV_DECAY, 1200 * log2(ton.env1 / 100.00)}, // Envelope 2
                        //{GN_INITIAL_ATTENUATION, 1440 - ((ton.vol0 * 1440) / 4096.00)}, // Initial attenuation
                        {GN_PITCH_COARSE_TUNE, (ton.noteroot < 0) ? 127 + ton.noteroot : 0}, // Coarse tune
                        {GN_PITCH_FINE_TUNE, ton.notetune}, // Fine tune
                        {GN_SAMPLE_MODE, (wav.loopbeg == wav.loopend) ? SM_NO_LOOP : SM_DEPRESSION_LOOP}, // Sample mode
                        {GN_SAMPLE_EXCLUSIVE_CLASS, ton.excl}, // Exclusive class
                        {GN_SAMPLE_OVERRIDE_ROOT, (ton.noteroot < 0) ? 127 : ton.noteroot}, // Root key
                        {GN_SAMPLE_ID, (ton.smpid < 0) ? ton.noteroot : ton.smpid}, // Sample ID
                    },
                    std::vector<modinfo>{
                        { // Channel volume
                            MS_LINEAR | MS_UNIPOLAR | MS_INCREASE | MS_MIDI_CONTROLLER | CC_CHANNEL_VOLUME_C,
                            GN_INITIAL_ATTENUATION, ton.vol,
                            MS_LINEAR | MS_UNIPOLAR | MS_INCREASE | MS_GENERAL_CONTROLLER | MS_NONE,
                            MT_LINEAR
                        },
                        { // Pan
                            MS_LINEAR | MS_UNIPOLAR | MS_INCREASE | MS_MIDI_CONTROLLER | CC_PAN_C,
                            GN_DRY_PAN, ton.pan,
                            MS_LINEAR | MS_UNIPOLAR | MS_INCREASE | MS_GENERAL_CONTROLLER | MS_NONE,
                            MT_LINEAR
                        }
                    }
                }}
            );

            itr[0].zon.emplace_back();
            itr[0].zon.back().gens.emplace_back(GN_KEY_RANGE, ton.notelow, ton.notehigh);
            itr[0].zon.back().gens.emplace_back(GN_INSTRUMENT_ID, i);
        }
        
        for (auto &p : prsts) {
            char nam[SFBK_NAME_MAX + 1] {};
            
            if (sgd_debug) fprintf(stderr, "            Set bank %d preset %d header and zones\n", p.bid, p.pid);
            set_nam(nam, SFBK_NAME_MAX, "prst_%03d_%04d", p.bid, p.pid);
            
            sf2_inf.setPhdr(nam, p.pid, p.bid, p.zon);
        }
    }

    return packRiffSfbk();
}

///Extracts variable region definitions into string
std::string extractRgnd() {
    if (sgd_debug) fprintf(stderr, "    Extract RGND info\n");
    
    std::string out;
    auto set_fstr = [&out]<typename... T>(const char *in, T&&... args) -> void {
        int s0 = snprintf(nullptr, 0, in, args...) + 1, s1 = out.size();
        out.resize(s1 + s0 - 1); snprintf(out.data() + s1, s0, in, args...);
    };

    set_fstr("Global Flags: %s\n", std::bitset<32>(sgd_inf.rgnd.flag).to_string().c_str());
    set_fstr("Regions:\n");
    for (const auto &r : sgd_inf.rgnd.rgnd) {
        set_fstr("    Region: %d\n", &r - sgd_inf.rgnd.rgnd.data());
        for (const auto &t : r) {
            set_fstr("        Tone: %d\n", &t - r.data());
            set_fstr("            Tone Flags: %s\n", std::bitset<32>(t.flag).to_string().c_str());
            set_fstr("            Name: %s\n", (!t.name.empty()) ? t.name.c_str() : "(none)");
            set_fstr("            Region Size: %d\n", t.rgnsiz);
            set_fstr("            Priority: %d\n", t.voice);
            set_fstr("            Bank ID 1: %d\n", t.excl);
            set_fstr("            Bank Mode: %d\n", t.bnkmode);
            set_fstr("            Bank ID 2: %d\n", t.bnkid);
            set_fstr("            Effect: %d\n", t.effect);
            set_fstr("            Note Range: %d to %d\n", t.notelow, t.notehigh);
            set_fstr("            Note Root: %d\n", t.noteroot);
            set_fstr("            Note Tune: %d\n", t.notetune);
            set_fstr("            Note Pitch: %d\n", t.notepitch);
            set_fstr("            Main Volume: %d\n", t.vol0);
            set_fstr("            Pan Volume: %d\n", t.vol1);
            set_fstr("            Dry Generator: %d\n", t.gendry);
            set_fstr("            Wet Generator: %d\n", t.genwet);
            set_fstr("            Envelope 1: %d\n", t.env0);
            set_fstr("            Envelope 2: %d\n", t.env1);
            set_fstr("            Volume: %d\n", t.vol);
            set_fstr("            Pan: %d\n", t.pan);
            set_fstr("            Bend Range: %d to %d\n", t.bendlow, t.bendhigh);
            set_fstr("            Sample ID: %d\n", t.smpid);
        }
    }

    return out;
}
