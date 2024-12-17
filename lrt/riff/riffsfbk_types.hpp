#ifndef RIFFSFBK_TYPES_HPP
#define RIFFSFBK_TYPES_HPP

#include <algorithm>
#include <compare>
#include <vector>
#include "chunk_type.hpp"
#include "riff_forms.hpp"
#include "riffsfbk_forms.hpp"
#include "riffsfbk_const.hpp"

///Sample Data Fields
struct sdtainfo {
    ~sdtainfo() = default;
    sdtainfo(const std::vector<short> sm = {}, const std::vector<unsigned char> s4 = {}) :
        smpl(std::vector<unsigned>(sm.begin(),sm.end())), sm24(s4) {}
    sdtainfo(const unsigned sb, const unsigned se) :
        smpl(std::vector<unsigned>{sb, se}) {}
    sdtainfo(const sdtainfo &sdt) = default;
    sdtainfo(sdtainfo &&sdt) = default;

    sdtainfo& operator=(const sdtainfo &sdt) = default;
    sdtainfo& operator=(sdtainfo &&sdt) = default;
    
    auto operator<=>(const sdtainfo &sdt) const {
        if (auto cmp = smpl <=> sdt.smpl; cmp != 0) return cmp;
        if (auto cmp = sm24 <=> sdt.sm24; cmp != 0) return cmp;
        return std::strong_ordering::equal;
    }
    bool operator==(const sdtainfo &sdt) const = default;
    bool operator!=(const sdtainfo &sdt) const = default;
    
    std::vector<unsigned> smpl;
    std::vector<unsigned char> sm24;
    
    bool empty() const { return smpl.empty() && sm24.empty(); }
    bool isValid24() const {
        return smpl.size() == sm24.size() + (sm24.size() % 2 ? 1 : 0);
    }
};

///Modulator Fields
struct modinfo {
    ~modinfo() = default;
    modinfo(
        const unsigned short ms = 0, const unsigned short md = 0,
        const short ma = 0,
        const unsigned short sa = 0, const unsigned short st = 0) :
        modsrc(ms), moddst(md), modamnt(ma), srcamnt(sa), srctrns(st) {}
    modinfo(const modinfo &mod) = default;
    modinfo(modinfo &&mod) = default;

    modinfo& operator=(const modinfo &mod) = default;
    modinfo& operator=(modinfo &&mod) = default;
    
    auto operator<=>(const modinfo &mod) const {
        if (auto cmp = modsrc <=> mod.modsrc; cmp != 0) return cmp;
        if (auto cmp = moddst <=> mod.moddst; cmp != 0) return cmp;
        if (auto cmp = modamnt <=> mod.modamnt; cmp != 0) return cmp;
        if (auto cmp = srcamnt <=> mod.srcamnt; cmp != 0) return cmp;
        if (auto cmp = srctrns <=> mod.srctrns; cmp != 0) return cmp;
        return std::strong_ordering::equal;
    }
    bool operator<(const modinfo &mod) const = default;
    bool operator>(const modinfo &mod) const = default;
    bool operator==(const modinfo &mod) const = default;
    bool operator<=(const modinfo &mod) const = default;
    bool operator!=(const modinfo &mod) const = default;
    bool operator>=(const modinfo &mod) const = default;

    unsigned short modsrc;  // SfModulatorSource
    unsigned short moddst;  // SfGenerator
    short modamnt;
    unsigned short srcamnt; // SfModulatorSource
    unsigned short srctrns; // SfModulatorTransform
};

///Generator Fields
struct geninfo {
    ~geninfo() = default;
    geninfo(const unsigned short ty = 0, const unsigned v0 = -1, const unsigned v1 = -1) :
        type(ty), val(0) {
            if ((int)v0 < 0 && (int)v1 < 0);
            else if ((int)v0 < 0 && (int)v1 >= 0) val = v1 & 0xFFFF;
            else if ((int)v0 >= 0 && (int)v1 < 0) val = v0 & 0xFFFF;
            else val = (v0 & 0xFF) << 8 | (v1 & 0xFF);
        }
    geninfo(const geninfo &gen) = default;
    geninfo(geninfo &&gen) = default;

    geninfo& operator=(const geninfo &gen) = default;
    geninfo& operator=(geninfo &&gen) = default;
    
    auto operator<=>(const geninfo &gen) const {
        if (auto cmp = type <=> gen.type; cmp != 0) return cmp;
        if (auto cmp = val <=> gen.val; cmp != 0) return cmp;
        return std::strong_ordering::equal;
    }
    bool operator<(const geninfo &gen) const = default;
    bool operator>(const geninfo &gen) const = default;
    bool operator==(const geninfo &gen) const = default;
    bool operator<=(const geninfo &gen) const = default;
    bool operator!=(const geninfo &gen) const = default;
    bool operator>=(const geninfo &gen) const = default;

    unsigned short type;
    unsigned short val;
};

///Zone Fields
struct baginfo {
    ~baginfo() = default;
    baginfo(const std::vector<geninfo> gn = {}, const std::vector<modinfo> md = {}) :
        gens(gn), mods(md) {}
    baginfo(const baginfo &bag) = default;
    baginfo(baginfo &&bag) = default;

    baginfo& operator=(const baginfo &bag) = default;
    baginfo& operator=(baginfo &&bag) = default;
    
    auto operator<=>(const baginfo &bag) const {
        if (auto cmp = gens <=> bag.gens; cmp != 0) return cmp;
        if (auto cmp = mods <=> bag.mods; cmp != 0) return cmp;
        return std::strong_ordering::equal;
    }
    bool operator<(const baginfo &bag) const = default;
    bool operator>(const baginfo &bag) const = default;
    bool operator==(const baginfo &bag) const = default;
    bool operator<=(const baginfo &bag) const = default;
    bool operator!=(const baginfo &bag) const = default;
    bool operator>=(const baginfo &bag) const = default;
    
    std::vector<geninfo> gens;
    std::vector<modinfo> mods;
};

///Preset Header Fields
struct phdrinfo {
    ~phdrinfo() = default;
    phdrinfo(
        const char *nm = 0, const unsigned short pi = 0, const unsigned short bi = 0,
        const std::vector<baginfo> pb = {},
        const unsigned lb = 0, const unsigned gn = 0, const unsigned mp = 0) :
        prstid(pi), bankid(bi), pbag(pb),
        library(lb), genre(gn), morph(mp) {
            for (auto &n : name) { if (!nm || !nm[0]) break; n = *(nm++); }
        }
    phdrinfo(const phdrinfo &phd) = default;
    phdrinfo(phdrinfo &&phd) = default;

    phdrinfo& operator=(const phdrinfo &phd) = default;
    phdrinfo& operator=(phdrinfo &&phd) = default;

    auto operator<=>(const phdrinfo &phd) const {
        if (auto cmp = bankid <=> phd.bankid; cmp != 0) return cmp;
        if (auto cmp = prstid <=> phd.prstid; cmp != 0) return cmp;
        if (auto cmp = library <=> phd.library; cmp != 0) return cmp;
        if (auto cmp = genre <=> phd.genre; cmp != 0) return cmp;
        if (auto cmp = morph <=> phd.morph; cmp != 0) return cmp;
        for (int n = 0; n < SFBK_NAME_MAX; ++n) {
            if (auto cmp = name[n] <=> phd.name[n]; cmp != 0) return cmp;
        }
        if (auto cmp = pbag <=> phd.pbag; cmp != 0) return cmp;
        return std::strong_ordering::equal;
    }
    bool operator==(const phdrinfo &phd) const = default;
    bool operator!=(const phdrinfo &phd) const = default;

    char name[SFBK_NAME_MAX] {};
    unsigned short prstid;
    unsigned short bankid;
    std::vector<baginfo> pbag;
    unsigned library;
    unsigned genre;
    unsigned morph;
};

///Instrument Header Fields
struct ihdrinfo {
    ~ihdrinfo() = default;
    ihdrinfo(const char *nm = 0, const std::vector<baginfo> ib = {}) :
        ibag(ib) { for (auto &n : name) { if (!nm || !nm[0]) break; n = *(nm++); } }
    ihdrinfo(const ihdrinfo &ihd) = default;
    ihdrinfo(ihdrinfo &&ihd) = default;

    ihdrinfo& operator=(const ihdrinfo &ihd) = default;
    ihdrinfo& operator=(ihdrinfo &&ihd) = default;

    auto operator<=>(const ihdrinfo &ihd) const {
        for (int n = 0; n < SFBK_NAME_MAX; ++n) {
            if (auto cmp = name[n] <=> ihd.name[n]; cmp != 0) return cmp;
        }
        if (auto cmp = ibag <=> ihd.ibag; cmp != 0) return cmp;
        return std::strong_ordering::equal;
    }
    bool operator==(const ihdrinfo &ihd) const = default;
    bool operator!=(const ihdrinfo &ihd) const = default;

    char name[SFBK_NAME_MAX] {};
    std::vector<baginfo> ibag;
};

///Sample Header Fields
struct shdrinfo {
    ~shdrinfo() = default;
    shdrinfo(
        const char *nm = 0, const sdtainfo sd = {},
        const unsigned lb = 0, const unsigned le = 0,
        const unsigned sr = 0, const char nr = 0, const char nt = 0,
        const unsigned short sl = 0, const unsigned short st = 0) :
        smpdata(sd),
        loopbeg(lb), loopend(le), smprate(sr), noteroot(nr),
        notetune(nt), smplink(sl), smptyp(st) {
            for (auto &n : name) { if (!nm || !nm[0]) break; n = *(nm++); }
        }
    shdrinfo(const shdrinfo &shd) = default;
    shdrinfo(shdrinfo &&shd) = default;

    shdrinfo& operator=(const shdrinfo &shd) = default;
    shdrinfo& operator=(shdrinfo &&shd) = default;

    auto operator<=>(const shdrinfo &shd) const {
        if (auto cmp = smpdata <=> shd.smpdata; cmp != 0) return cmp;
        if (auto cmp = loopbeg <=> shd.loopbeg; cmp != 0) return cmp;
        if (auto cmp = loopend <=> shd.loopend; cmp != 0) return cmp;
        if (auto cmp = smprate <=> shd.smprate; cmp != 0) return cmp;
        if (auto cmp = noteroot <=> shd.noteroot; cmp != 0) return cmp;
        if (auto cmp = notetune <=> shd.notetune; cmp != 0) return cmp;
        if (auto cmp = smplink <=> shd.smplink; cmp != 0) return cmp;
        if (auto cmp = smptyp <=> shd.smptyp; cmp != 0) return cmp;
        for (int n = 0; n < SFBK_NAME_MAX; ++n) {
            if (auto cmp = name[n] <=> shd.name[n]; cmp != 0) return cmp;
        }
        return std::strong_ordering::equal;
    }
    bool operator==(const shdrinfo &shd) const = default;
    bool operator!=(const shdrinfo &shd) const = default;

    char name[SFBK_NAME_MAX] {};
    sdtainfo smpdata;
    unsigned loopbeg;
    unsigned loopend;
    unsigned smprate;
    unsigned char noteroot;
    char notetune;
    unsigned short smplink;
    unsigned short smptyp;
    
    unsigned begin() const {
        if (isRom() && smpdata.smpl.size() == 2) return smpdata.smpl[0];
        return 0;
    }
    unsigned end() const {
        if (isRom() && smpdata.smpl.size() == 2) return smpdata.smpl[1];
        return smpdata.smpl.size();
    }
    unsigned size() const { return end() - begin(); }
    bool isRam() const { return !(smptyp & 0x8000); }
    bool isRom() const { return (smptyp & 0x8000); }
    bool is24b() const { return smpdata.isValid24(); }
};

///Preset Data Fields
struct pdtainfo {
    ~pdtainfo() = default;
    pdtainfo(
        const std::vector<phdrinfo> ph = {},
        const std::vector<ihdrinfo> ih = {},
        const std::vector<shdrinfo> sh = {}
        ) : phdr(ph), inst(ih), shdr(sh) {}
    pdtainfo(const pdtainfo &pdt) = default;
    pdtainfo(pdtainfo &&pdt) = default;

    pdtainfo& operator=(const pdtainfo &pdt) = default;
    pdtainfo& operator=(pdtainfo &&pdt) = default;

    auto operator<=>(const pdtainfo &pdt) const {
        if (auto cmp = phdr <=> pdt.phdr; cmp != 0) return cmp;
        if (auto cmp = inst <=> pdt.inst; cmp != 0) return cmp;
        if (auto cmp = shdr <=> pdt.shdr; cmp != 0) return cmp;
        return std::strong_ordering::equal;
    }
    bool operator==(const pdtainfo &pdt) const = default;
    bool operator!=(const pdtainfo &pdt) const = default;
    
    std::vector<phdrinfo>   phdr;
    std::vector<ihdrinfo>   inst;
    std::vector<shdrinfo>   shdr;
    
    bool empty() const { return phdr.empty() && inst.empty() && shdr.empty(); }
};

///Soundfont Fields
struct riffsfbk {
    std::vector<chunk>  info;
    pdtainfo            pdta;
    
    bool empty() const { return info.empty() && pdta.empty(); }
    void setIfil(const unsigned short v0 = 2, const unsigned short v1 = 4) {
        info.emplace_back(set_ver(INFO_ifil, v0, v1));
    }
    void setIsng(const char *nm = 0) { info.emplace_back(set_str(INFO_isng, nm)); }
    void setInam(const char *nm = 0) { info.emplace_back(set_str(INFO_INAM, nm)); }
    void setIrom(const char *nm = 0) { info.emplace_back(set_str(INFO_irom, nm)); }
    void setIver(const unsigned short v0 = 0, const unsigned short v1 = 0) {
        info.emplace_back(set_ver(INFO_iver, v0, v1));
    }
    void setIcrd(const char *dt = 0) { info.emplace_back(set_str(INFO_ICRD, dt)); }
    void setIcrd(const char *mn, const unsigned char dy, const unsigned yr) {
        char tmp[30] {}; int pos = 0;
        while (pos < 29 && mn[0]) tmp[pos++] = *mn++;
        if (pos < 29) tmp[pos++] = ' ';
        if (pos < 29) tmp[pos++] = '0' + (dy / 10);
        if (pos < 29) tmp[pos++] = '0' + (dy % 10);
        if (pos < 29) tmp[pos++] = ',';
        if (pos < 29) tmp[pos++] = ' ';
        if (pos < 29) tmp[pos++] = '0' + (yr / 1000);
        if (pos < 29) tmp[pos++] = '0' + ((yr % 1000) / 100);
        if (pos < 29) tmp[pos++] = '0' + ((yr % 100) / 10);
        if (pos < 29) tmp[pos++] = '0' + (yr % 10);
        setIcrd(tmp);
    }
    void setIeng(const char *nm = 0) { info.emplace_back(set_str(INFO_IENG, nm)); }
    void setIprd(const char *nm = 0) { info.emplace_back(set_str(INFO_IPRD, nm)); }
    void setIcop(const char *nm = 0) { info.emplace_back(set_str(INFO_ICOP, nm)); }
    void setIcmt(const char *nm = 0) { info.emplace_back(set_str(INFO_ICMT, nm)); }
    void setIsft(const char *nm = 0) { info.emplace_back(set_str(INFO_ISFT, nm)); }
    void setPhdr(
        const char *nm = 0, const unsigned short pi = 0, const unsigned short bi = 0,
        const std::vector<baginfo> zn = {}) {
        pdta.phdr.emplace_back(nm, pi, bi, zn);
    }
    void setInst(
        const char *nm = 0,
        const std::vector<baginfo> zn = {}) {
        pdta.inst.emplace_back(nm, zn);
    }
    void setShdr(
        const char *nm = 0, const sdtainfo sd = {},
        const unsigned lb = 0, const unsigned le = 0,
        const unsigned sr = 0, const char nr = 0, const char nt = 0,
        const unsigned short sl = 0, const unsigned short st = 0) {
        pdta.shdr.emplace_back(nm, sd, lb, le, sr, nr, nt, sl, st);
    }
    unsigned getPnum() const { return pdta.phdr.size(); }
    unsigned getInum() const { return pdta.inst.size(); }
    unsigned getSnum() const { return pdta.shdr.size(); }
    
    private:
        chunk set_ver(const unsigned fc, const unsigned short v0, const unsigned short v1) {
            chunk out;
            out.setFcc(fc);
            out.setInt(v0, 2);
            out.setInt(v1, 2);
            return out;
        }
        chunk set_str(const unsigned fc, const char *nm) {
            chunk out;
            out.setFcc(fc);
            out.setZtr(nm);
            return out;
        }
};


#endif
