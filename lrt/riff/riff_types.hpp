#ifndef RIFF_TYPES_HPP
#define RIFF_TYPES_HPP

#include <string>
#include <vector>
#include "fourcc_type.hpp"
#include "chunk_type.hpp"


///Character Set Fields
struct csetinfo {
    ~csetinfo() = default;
    csetinfo() = default;
    csetinfo(const unsigned t_ld) : lang(t_ld >> 16), dial(t_ld & 0xFFFF) {}
    csetinfo(const csetinfo &c) = default;
    csetinfo(csetinfo &&c) = default;
    
    csetinfo& operator=(const csetinfo &c) = default;
    csetinfo& operator=(csetinfo &&c) = default;
    
    //To make assignments easier
    void operator=(const unsigned ladi) { lang = ladi >> 16; dial = ladi & 0xFFFF; }
    
    //To make comparisons easier
    bool operator==(const unsigned &ladi) const {
        if (ladi >> 16 != lang) return 0;
        if (ladi & 0xFFFF != dial) return 0;
        return 1;
    }
    
    //Set character set info
    void setPage(const unsigned short &pg) { page = pg; }
    void setCode(const unsigned short &cd) { code = cd; }
    void setLang(const unsigned short &lg) { lang = lg; }
    void setDial(const unsigned short &dl) { dial = dl; }
    void setLnDi(const unsigned &ld) { lang = ld >> 16; dial = ld & 0xFFFF; }
    //Get character set info
    unsigned short getPage() const { return page; }
    unsigned short getCode() const { return code; }
    unsigned short getLang() const { return lang; }
    unsigned short getDial() const { return dial; }
    unsigned getLnDi() const { return (unsigned)lang << 16 || dial; }
    
    private: unsigned short page, code, lang, dial;
};

///CTOC Extra Fields
struct ctocfield { unsigned use, fld; };

///RIFF Compound File Table of Contents Fields
struct ctocinfo {
    unsigned offs;
    unsigned size;
    fourcc type;
    unsigned usag;
    unsigned comp;
    unsigned sraw;
    std::vector<ctocfield> efld;
    unsigned char eflg;
    std::string name;
    
    bool is_frcc;
};

///RIFF Fields
struct riffinfo {
    chunk                   riff;   // Stores RIFF and subchunk
    chunk                   list;   // Stores LIST and subchunk
    std::vector<chunk>      info;   // Stores variable INFO subchunks
    csetinfo                cset;   // Stores CSET chunk
    chunk                   disp;   // Stores DISP chunk
    std::vector<ctocinfo>   ctoc;   // Stores variable CTOC definitions
    chunk                   cgrp;   // Stores CGRP chunk
};


#endif
