#include <vector>
#include "fourcc_type.hpp"
#include "chunk_type.hpp"
#include "riff_forms.hpp"
#include "riff_func.hpp"
#include "riffwave_forms.hpp"
#include "riffwave_const.hpp"
#include "riffwave_func.hpp"


///Unpacks WAVE info from WAVE data
void unpackRiffWave(unsigned char *in, const unsigned length,
                    const EndianType endian, const bool is_rv) {
    wav_inf = {};
    if (!in || length < 4) return;

    const unsigned char *in_end = in + length;

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

    while (in < in_end) {
        fourcc t_fc;
        unsigned t_sz;
        chunk t_ch;

        t_fc = get_fcc();
        t_sz = get_int();
        if (in + t_sz > in_end) break;

        t_ch.setRev(is_rv);
        t_ch.setEnd(endian);
        t_ch.setFcc(t_fc);
        t_ch.setArr(in, t_sz); in += t_sz;
        if (t_sz % 2 && !in[0]) { t_ch.setPad(); in += 1; }

        switch(t_ch.getFcc().getInt()) {
#ifdef UNPACKFMT_IMPLEMENTATION
            case WAVE_fmt :
                unpackFmt(t_ch);
                continue;
#endif
            case WAVL_data:
            case WAVL_slnt:
                wav_inf.wavl.emplace_back(t_ch);
                continue;
#ifdef UNPACKLIST_IMPLEMENTATION
            case RIFF_LIST:
                unpackList(t_ch);
                switch(riff_inf.list.getFcc.getInt()) {
#ifdef UNPACKINFO_IMPLEMENTATION
                    case LIST_INFO:
                        unpackInfo(riff_inf.list);
                        wav_inf.info.swap(riff_inf.info);
                        continue;
#endif
#ifdef UNPACKADTL_IMPLEMENTATION
                    case LIST_adtl:
                        unpackAdtl(riff_inf.list);
                        continue;
#endif
#ifdef UNPACKWAVL_IMPLEMENTATION
                    case LIST_wavl:
                        unpackWavl(riff_inf.list);
                        continue;
#endif
                    default:
                        continue;
                }
                continue;
#endif
#ifdef UNPACKFACT_IMPLEMENTATION
            case WAVE_fact:
                unpackFact(t_ch);
                continue;
#endif
#ifdef UNPACKCUE_IMPLEMENTATION
            case WAVE_cue :
                unpackCue(t_ch);
                continue;
#endif
#ifdef UNPACKPLST_IMPLEMENTATION
            case WAVE_plst:
                unpackPlst(t_ch);
                continue;
#endif
#ifdef UNPACKSMPL_IMPLEMENTATION
            case WAVE_smpl:
                unpackSmpl(t_ch)
                continue;
#endif
#ifdef UNPACKINST_IMPLEMENTATION
            case WAVE_inst:
                unpackInst(t_ch);
                continue;
#endif
            default:
                continue;
        }
    }
}

///Unpacks WAVE info from WAVE chunk
void unpackRiffWave(const chunk chnk) {
    unpackRiffWave(chnk.getArr().data(), chnk.size() - 8, chnk.getEnd(), chnk.getRev());
}


///Packs WAVE info into array
std::vector<unsigned char> packRiffWave() {
    if (wav_inf.fmt.empty() || wav_inf.wavl.empty()) return {};

    chunk out, wave;

    //Initial setup of RIFF WAVE data
    wave.setFcc(RIFF_WAVE);

    //Set format chunk
    if (true) {
        chunk fmt;
        unsigned t_sz;

        fmt.setFcc(WAVE_fmt);
        fmt.setInt(wav_inf.fmt.codec, 2);
        fmt.setInt(wav_inf.fmt.chns, 2);
        fmt.setInt(wav_inf.fmt.smprate, 4);
        fmt.setInt(wav_inf.fmt.bytrate, 4);
        fmt.setInt(wav_inf.fmt.align, 2);
        fmt.setInt(wav_inf.fmt.bitrate, 2);
        t_sz = 0;
        if (wav_inf.fmt.smpinfo != 0) t_sz += 2;
        if (wav_inf.fmt.chnmask != 0) t_sz += 4;
        if (wav_inf.fmt.guid != uuid{}) t_sz += 16;
        t_sz += wav_inf.fmt.extra.size();
        fmt.setInt(t_sz, 2);
        if (wav_inf.fmt.smpinfo != 0) fmt.setInt(wav_inf.fmt.smpinfo, 2);
        if (wav_inf.fmt.chnmask != 0) fmt.setInt(wav_inf.fmt.chnmask, 4);
        if (wav_inf.fmt.guid != uuid{}) {
            fmt.setInt(wav_inf.fmt.guid.g0, 4);
            fmt.setInt(wav_inf.fmt.guid.g1, 2);
            fmt.setInt(wav_inf.fmt.guid.g2, 2);
            fmt.setArr(wav_inf.fmt.guid.g3, 8);
        }
        fmt.setArr(wav_inf.fmt.extra);

        wave += fmt;
    }

    //Set data chunk
    if (true) {
        chunk data;

        data.setFcc(WAVL_data);
        for (const auto &w : wav_inf.wavl) {
            if (w.pcm.empty()) continue;
            for (const auto &s : w.pcm) data.setInt(s, 2);
        }

        wave += data;
    }

    //Set information list chunk if applicable
    if (!wav_inf.info.empty()) {
        chunk list, info;

        info.setFcc(LIST_INFO);
        for (const auto &i : wav_inf.info) info += i;

        list.setFcc(RIFF_LIST);
        list.setChk(info, false);

        wave += list;
    }

    //Set sampler chunk if applicable
    for (const auto &s : wav_inf.smpl) {
        chunk smpl;

        smpl.setFcc(WAVE_smpl);
        smpl.setInt(s.manucode, 4);
        smpl.setInt(s.prodcode, 4);
        smpl.setInt(s.smpperiod, 4);
        smpl.setInt(s.noteroot, 4);
        smpl.setInt(s.notetune, 4);
        smpl.setInt(s.smptetyp, 4);
        smpl.setInt(s.smpteoffs, 4);
        smpl.setInt(s.loops.size(), 4);
        smpl.setInt(s.extra.size(), 4);
        for (const auto &lp : s.loops) {
            smpl.setInt(lp.loopid, 4);
            smpl.setInt(lp.looptyp, 4);
            smpl.setInt(lp.loopbeg, 4);
            smpl.setInt(lp.loopend, 4);
            smpl.setInt(lp.loopres, 4);
            smpl.setInt(lp.loopfrq, 4);
        }
        smpl.setArr(s.extra);

        wave += smpl;
    }

    //Set instrument chunk if applicable
    if (!wav_inf.inst.empty()) {
        chunk inst;

        inst.setFcc(WAVE_inst);
        inst.setInt(wav_inf.inst.noteroot, 1);
        inst.setInt(wav_inf.inst.notetune, 1);
        inst.setInt(wav_inf.inst.notegain, 1);
        inst.setInt(wav_inf.inst.notelow, 1);
        inst.setInt(wav_inf.inst.notehigh, 1);
        inst.setInt(wav_inf.inst.vellow, 1);
        inst.setInt(wav_inf.inst.velhigh, 1);

        wave += inst;
    }

    //Set RIFF data
    out.setFcc(FOURCC_RIFF);
    out.setChk(wave, false);

    return out.getAll();
}
