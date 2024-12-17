#include <algorithm>
#include <vector>
#include "fourcc_type.hpp"
#include "chunk_type.hpp"
#include "riff_forms.hpp"
#include "riff_types.hpp"
#include "riff_func.hpp"
#include "riffsfbk_forms.hpp"
#include "riffsfbk_const.hpp"
#include "riffsfbk_types.hpp"
#include "riffsfbk_func.hpp"


///Unpacks SFBK info from SFBK data
void unpackRiffSfbk(unsigned char *in, const unsigned length,
                    const EndianType endian, const bool is_rv) {
    sf2_inf = {};
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
#ifdef UNPACKLIST_IMPLEMENTATION
        if (t_ch.getFcc() == LIST_INFO) {
            unpackList(t_ch);
            switch(riff_inf.list.getFcc.getInt()) {
#ifdef UNPACKINFO_IMPLEMENTATION
                case LIST_INFO:
                    unpackInfo(riff_inf.list);
                    sf2_inf.info.swap(riff_inf.info);
                    continue;
#endif
#ifdef UNPACKSDTA_IMPLEMENTATION
                case LIST_sdta:
                    unpackSdta(riff_inf.list);
                    continue;
#endif
#ifdef UNPACKPDTA_IMPLEMENTATION
                case LIST_pdta:
                    unpackPdta(riff_inf.list);
                    continue;
#endif
                default:
                    continue;
            }
        }
#endif
    }
}

///Unpacks SFBK info from SFBK chunk
void unpackRiffSfbk(const chunk chnk) {
    unpackRiffSfbk(chnk.getArr().data(), chnk.size() - 8, chnk.getEnd(), chnk.getRev());
}


///Packs SFBK info into array
std::vector<unsigned char> packRiffSfbk() {
    if (sf2_inf.info.empty()) return {};

    chunk out, sfbk;

    //Initial setup of RIFF SFBK data
    sfbk.setFcc(RIFF_sfbk);

    //Set information chunk
    if (true) {
        auto has_ifil = []() -> bool {
            return std::find_if(
                sf2_inf.info.begin(), sf2_inf.info.end(),
                [](const chunk &c){return c.getFcc()==INFO_ifil;}
            ) != sf2_inf.info.end();
        };
        auto has_isng = []() -> bool {
            return std::find_if(
                sf2_inf.info.begin(), sf2_inf.info.end(),
                [](const chunk &c){return c.getFcc()==INFO_isng;}
            ) != sf2_inf.info.end();
        };
        auto has_inam = []() -> bool {
            return std::find_if(
                sf2_inf.info.begin(), sf2_inf.info.end(),
                [](const chunk &c){return c.getFcc()==INFO_INAM;}
            ) != sf2_inf.info.end();
        };

        chunk list, info;

        //Set information chunk
        info.setFcc(LIST_INFO);
        if (!has_ifil()) {
            chunk ifil;

            ifil.setRev(false);
            ifil.setEnd(ENDIAN_LITTLE);
            ifil.setFcc(INFO_ifil);
            ifil.setInt(0x02, 2);
            ifil.setInt(0x04, 2);

            info += ifil;
        }
        if (!has_isng()) {
            chunk isng;

            isng.setRev(false);
            isng.setEnd(ENDIAN_LITTLE);
            isng.setFcc(INFO_isng);
            isng.setZtr("EMU8000");

            info += isng;
        }
        if (!has_inam()) {
            chunk inam;

            inam.setRev(false);
            inam.setEnd(ENDIAN_LITTLE);
            inam.setFcc(INFO_INAM);
            inam.setZtr("UNKNOWN");

            info += inam;
        }
        for (auto &i : sf2_inf.info) {
            std::vector<unsigned char> dat;
            std::string tmp;
            switch(i.getFcc().getInt()) {
                case INFO_isng:
                case INFO_INAM:
                case INFO_irom:
                case INFO_ICRD:
                case INFO_IENG:
                case INFO_IPRD:
                case INFO_ICOP:
                case INFO_ICMT:
                case INFO_ISFT:
                    dat = i.getArr(); i.clrArr();
                    tmp = std::string(dat.begin(), dat.end());
                    tmp = tmp.substr(0, tmp.find_first_of('\0'));
                    i.setZtr(tmp); if (tmp.length() % 2 == 0) i.setPad();
                default:
                    info += i;
                    break;
            }
        }

        //Set list chunk
        list.setFcc(RIFF_LIST);
        list.setChk(info, false);

        sfbk += list;
    }

    //Set sample data chunk
    if (true) {
        auto has_smpl = []() -> bool {
            return std::find_if(
                sf2_inf.pdta.shdr.begin(), sf2_inf.pdta.shdr.end(),
                [](const shdrinfo &s){return s.isRam();}
            ) != sf2_inf.pdta.shdr.end();
        };
        auto has_sm24 = []() -> bool {
            return std::find_if_not(
                sf2_inf.pdta.shdr.begin(), sf2_inf.pdta.shdr.end(),
                [](const shdrinfo &s){return s.is24b();}
            ) == sf2_inf.pdta.shdr.end();
        };
        
        chunk list, sdta;

        //Set sample data chunk
        sdta.setFcc(LIST_sdta);
        if (has_smpl()) {
            chunk smpl;
            smpl.setFcc(SDTA_smpl);
            for (const auto &shd : sf2_inf.pdta.shdr) {
                for (const auto &s : shd.smpdata.smpl) smpl.setInt(s, 2);
                smpl.setPad(SFBK_SMPL_PAD * 2);
            }
            sdta += smpl;
        }
        if (has_sm24()) {
            chunk sm24;
            sm24.setFcc(SDTA_sm24);
            for (const auto &shd : sf2_inf.pdta.shdr) {
                sm24.setArr(shd.smpdata.sm24);
                if (shd.smpdata.sm24.size() % 2) sm24.setPad();
                sm24.setPad(SFBK_SMPL_PAD);
            }
            sdta += sm24;
        }

        //Set list chunk
        list.setFcc(RIFF_LIST);
        list.setChk(sdta, false);

        sfbk += list;
    }

    //Set preset data chunk
    if (true) {
        const auto get_bag = [](const std::vector<baginfo> &bags, int &g, int &m) -> std::vector<unsigned char> {
            chunk tmp;
            for (const auto &bg : bags) {
                tmp.setInt(g, 2); g += bg.gens.size();
                tmp.setInt(m, 2); m += bg.mods.size();
            }
            return tmp.getArr();
        };
        const auto get_mod = [](const std::vector<modinfo> &mods) -> std::vector<unsigned char> {
            chunk tmp;
            for (const auto &md : mods) {
                tmp.setInt(md.modsrc, 2);
                tmp.setInt(md.moddst, 2);
                tmp.setInt(md.modamnt, 2);
                tmp.setInt(md.srcamnt, 2);
                tmp.setInt(md.srctrns, 2);
            }
            return tmp.getArr();
        };
        const auto get_gen = [](const std::vector<geninfo> &gens) -> std::vector<unsigned char> {
            chunk tmp;
            for (const auto &gn : gens) {
                tmp.setInt(gn.type, 2);
                if (gn.type == GN_KEY_RANGE || gn.type == GN_VELOCITY_RANGE) {
                    tmp.setArr({gn.val >> 8, gn.val & 0xFF});
                }
                else tmp.setInt(gn.val, 2);
            }
            return tmp.getArr();
        };

        chunk list, pdta;

        //Set preset data chunk
        pdta.setFcc(LIST_pdta);
        //Set preset headers
        if (true) {
            chunk phdr;
            phdr.setFcc(PDTA_phdr);
            for (int p = 0, b = 0; p <= sf2_inf.pdta.phdr.size(); ++p) {
                if (p == sf2_inf.pdta.phdr.size()) {
                    phdr.setStr("EOP", SFBK_NAME_MAX);
                    phdr.setPad(4);
                    phdr.setInt(b, 2);
                    phdr.setPad(12);
                }
                else {
                    const auto &phd = sf2_inf.pdta.phdr[p];
                    phdr.setStr(phd.name, SFBK_NAME_MAX);
                    phdr.setInt(phd.prstid, 2);
                    phdr.setInt(phd.bankid, 2);
                    phdr.setInt(b, 2); b += phd.pbag.size();
                    phdr.setInt(phd.library, 4);
                    phdr.setInt(phd.genre, 4);
                    phdr.setInt(phd.morph, 4);
                }
            }
            
            pdta += phdr;
        }
        //Set preset zones
        if (true) {
            chunk pbag;
            int g = 0, m = 0;

            pbag.setFcc(PDTA_pbag);
            for (const auto &phd : sf2_inf.pdta.phdr) {
                pbag.setArr(get_bag(phd.pbag, g, m));
            }
            pbag.setInt(g, 2);
            pbag.setInt(m, 2);
            //pbag.setPad(SFBK_BAG_SIZE);
            
            pdta += pbag;
        }
        //Set preset modulators
        if (true) {
            chunk pmod;

            pmod.setFcc(PDTA_pmod);
            for (const auto &phd : sf2_inf.pdta.phdr) {
                for (const auto &bag : phd.pbag) pmod.setArr(get_mod(bag.mods));
            }
            pmod.setPad(SFBK_MOD_SIZE);
            
            pdta += pmod;
        }
        //Set preset generators
        if (true) {
            chunk pgen;

            pgen.setFcc(PDTA_pgen);
            for (const auto &phd : sf2_inf.pdta.phdr) {
                for (const auto &bag : phd.pbag) pgen.setArr(get_gen(bag.gens));
            }
            pgen.setPad(SFBK_GEN_SIZE);
            
            pdta += pgen;
        }
        //Set instrument headers
        if (true) {
            chunk inst;
            
            inst.setFcc(PDTA_inst);
            for (int i = 0, b = 0; i <= sf2_inf.pdta.inst.size(); ++i) {
                if (i == sf2_inf.pdta.inst.size()) {
                    inst.setStr("EOI", SFBK_NAME_MAX);
                    inst.setInt(b, 2);
                }
                else {
                    const auto &ihd = sf2_inf.pdta.inst[i];
                    inst.setStr(ihd.name, SFBK_NAME_MAX);
                    inst.setInt(b, 2); b += ihd.ibag.size();
                }
            }
            
            pdta += inst;
        }
        //Set instrument zones
        if (true) {
            chunk ibag;
            int g = 0, m = 0;

            ibag.setFcc(PDTA_ibag);
            for (const auto &ihd : sf2_inf.pdta.inst) {
                ibag.setArr(get_bag(ihd.ibag, g, m));
            }
            ibag.setInt(g, 2);
            ibag.setInt(m, 2);
            ibag.setPad(SFBK_BAG_SIZE);
            
            pdta += ibag;
        }
        //Set instrument modulators
        if (true) {
            chunk imod;

            imod.setFcc(PDTA_imod);
            for (const auto &ihd : sf2_inf.pdta.inst) {
                for (const auto &bag : ihd.ibag) imod.setArr(get_mod(bag.mods));
            }
            imod.setPad(SFBK_MOD_SIZE);
            
            pdta += imod;
        }
        //Set instrument generators
        if (true) {
            chunk igen;

            igen.setFcc(PDTA_igen);
            for (const auto &ihd : sf2_inf.pdta.inst) {
                for (const auto &bag : ihd.ibag) igen.setArr(get_gen(bag.gens));
            }
            igen.setPad(SFBK_GEN_SIZE);
            
            pdta += igen;
        }
        //Set sample headers
        if (true) {
            chunk shdr;
            
            shdr.setFcc(PDTA_shdr);
            for (int s = 0, sb = 0; s < sf2_inf.pdta.shdr.size(); ++s) {
                const auto &shd = sf2_inf.pdta.shdr[s];
                shdr.setStr(shd.name, SFBK_NAME_MAX);
                if (shd.isRom()) {
                    shdr.setInt(shd.begin(), 4);
                    shdr.setInt(shd.end(), 4);
                    shdr.setInt(shd.loopbeg, 4);
                    shdr.setInt(shd.loopend, 4);
                }
                else {
                    shdr.setInt(sb + shd.begin(), 4);
                    shdr.setInt(sb + shd.end(), 4);
                    shdr.setInt(sb + shd.loopbeg, 4);
                    shdr.setInt(sb + shd.loopend, 4);
                    sb += shd.size() + SFBK_SMPL_PAD;
                }
                shdr.setInt(shd.smprate, 4);
                shdr.setInt(shd.noteroot, 1);
                shdr.setInt(shd.notetune, 1);
                shdr.setInt(shd.smplink, 2);
                shdr.setInt(shd.smptyp, 2);
            }
            shdr.setStr("EOS", SFBK_NAME_MAX);
            shdr.setPad(SFBK_SHDR_SIZE - SFBK_NAME_MAX);
            
            pdta += shdr;
        }

        //Set list chunk
        list.setFcc(RIFF_LIST);
        list.setChk(pdta, false);

        sfbk += list;
    }

    //Set RIFF data
    out.setFcc(FOURCC_RIFF);
    out.setChk(sfbk, false);

    return out.getAll();
}
