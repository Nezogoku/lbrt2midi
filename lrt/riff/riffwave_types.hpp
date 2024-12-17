#ifndef RIFFWAVE_TYPES_HPP
#define RIFFWAVE_TYPES_HPP

#include <string>
#include <vector>
#include "fourcc_type.hpp"
#include "uuid_type.hpp"

///Format Fields
struct fmtinfo {
    //Common fields
    unsigned short codec;
    unsigned short chns;
    unsigned smprate;
    unsigned bytrate;
    unsigned short align;

    //Extra field
    unsigned short bitrate;

    //Extensible fields
    unsigned short smpinfo;
    unsigned chnmask;
    uuid guid;
    std::vector<unsigned char> extra;

    bool empty() const {
        return
            !codec &&
            !chns &&
            !smprate &&
            !bytrate &&
            !align &&
            !bitrate &&
            !smpinfo &&
            !chnmask &&
            guid == uuid{} &&
            extra.empty();
    }
};

///Fact Fields
struct factinfo {
    unsigned smpsize;
    std::vector<unsigned> smpinfo;

    bool empty() const { return !smpsize && smpinfo.empty(); }
};

///Cue Point Fields
struct cuepoint {
    unsigned point;
    unsigned smppos;
    fourcc chnktyp;
    unsigned chnkbeg;
    unsigned blokbeg;
    unsigned smpoffs;
};

///Playlist Segment Fields
struct plstsegment {
    unsigned cueid;
    unsigned smpsize;
    unsigned loopnum;
};

///Associated Data Label Fields
struct lablinfo {
    unsigned cueid;
    std::string label;
};

///Associated Data Note Fields
struct noteinfo {
    unsigned cueid;
    std::string comment;
};

///Associated Data Labeled Text Fields
struct ltxtinfo {
    unsigned cueid;
    unsigned smpsize;
    unsigned purpose;
    unsigned short ctry;
    unsigned short lang;
    unsigned short dial;
    unsigned short page;
};

///Associated Data Fields
struct adtlinfo {
    std::vector<lablinfo> labl;
    std::vector<noteinfo> note;
    std::vector<ltxtinfo> ltxt;

    bool empty() const { return labl.empty() && note.empty() && ltxt.empty(); }
};

///Sampler Loop Fields
struct loopinfo {
    unsigned loopid;
    unsigned looptyp;
    unsigned loopbeg;
    unsigned loopend;
    unsigned loopres;
    unsigned loopfrq;
};

///Sampler Fields
struct smplinfo {
    unsigned manucode;
    unsigned prodcode;
    unsigned smpperiod;
    unsigned noteroot;
    unsigned notetune;
    unsigned smptetyp;
    unsigned smpteoffs;
    std::vector<loopinfo> loops;
    std::vector<unsigned char> extra;
};

///Instrument Fields
struct instinfo {
    unsigned char noteroot;
    char notetune;
    char notegain;
    unsigned char notelow;
    unsigned char notehigh;
    unsigned char vellow;
    unsigned char velhigh;

    bool empty() const {
        return
            !noteroot &&
            !notetune &&
            !notegain &&
            !notelow &&
            !notehigh &&
            !vellow &&
            !velhigh;
    }
};

///Wave List Fields
struct wavlinfo {
    ~wavlinfo() = default;
    wavlinfo() = default;
    wavlinfo(const chunk c) : chnk(c) {}
    wavlinfo(const std::vector<short> p) : pcm(p) {}
    wavlinfo(const wavlinfo &d) = default;
    wavlinfo(wavlinfo &&d) = default;

    wavlinfo& operator=(const wavlinfo &d) = default;
    wavlinfo& operator=(wavlinfo &&d) = default;
    
    chunk chnk;
    std::vector<short> pcm;

    bool empty() const { return chnk.empty() && pcm.empty(); }
};

///Waveform Fields
struct riffwave {
    fmtinfo                     fmt;
    std::vector<wavlinfo>       wavl;
    std::vector<chunk>          info;
    adtlinfo                    adtl;
    factinfo                    fact;
    std::vector<cuepoint>       cue;
    std::vector<plstsegment>    plst;
    std::vector<smplinfo>       smpl;
    instinfo                    inst;

    bool empty() const {
        return
            fmt.empty() &&
            wavl.empty() &&
            info.empty() &&
            adtl.empty() &&
            fact.empty() &&
            cue.empty() &&
            plst.empty() &&
            smpl.empty() &&
            inst.empty();
    }
};


#endif
