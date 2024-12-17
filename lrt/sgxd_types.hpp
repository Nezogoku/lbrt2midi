#ifndef SGXD_TYPES_HPP
#define SGXD_TYPES_HPP

#include <string>
#include <vector>

//Fields courtesy of SGXD.bt in Nenkai's SGXDataBuilder

///Audio Buss Effect Fields
struct busseffect {
    unsigned flag;
    std::string name;
    std::string module;
    char assign[8] {};
    char numin;
    char numout;
    //unsigned short res0;
    char busin[4] {};
    char busout[4] {};
    short gainin[4] {};
    short gainout[4] {};
    std::string preset;
    //unsigned res1;
    //unsigned res2;
    //unsigned res3;
};

///Audio Buss Unit Fields
struct bussunit {
    unsigned flag;
    std::string name;
    //unsigned res0;
    //unsigned res1;
};

///Audio Buss Fields
struct bussbus {
    unsigned flag;
    std::string name;
    std::vector<bussunit> unit;
    std::vector<busseffect> effect;
    char munit;
    //unsigned char res0;
    //unsigned res1;
    //signed unitoffs;
    //signed effectoffs;
    std::string oper;
    std::string oparam;
    //unsigned res2;
    //unsigned res3;
    //unsigned res4;
    //unsigned res5;
};

///Audio Buss Fields Main
struct bussinfo {
    unsigned flag;
    std::vector<bussbus> buss;
    
    bool empty() const { return !flag && buss.empty(); }
};

///Region Definition Fields
struct rgndrgn {
    unsigned flag;
    std::string name;
    unsigned rgnsiz;
    char voice;
    unsigned char excl;
    char bnkmode;
    char bnkid;
    //unsigned res0;
    signed effect;
    char notelow;
    char notehigh;
    //unsigned short res1;
    char noteroot;
    char notetune;
    short notepitch;
    short vol0;
    short vol1;
    short gendry;
    short genwet;
    signed env0;
    signed env1;
    char vol;
    char pan;
    char bendlow;
    char bendhigh;
    signed smpid;
};

///Region Definition Fields Main
struct rgndinfo {
    unsigned flag;
    std::vector<std::vector<rgndrgn>> rgnd;
    
    bool empty() const { return !flag && rgnd.empty(); }
};

///Sequence Definition Fields
struct seqdseq {
    unsigned flag;
    std::string name;
    short fmt;
    short div;
    short volleft;
    short volright;
    std::vector<unsigned char> data;
    
    bool empty() const {
        return
            !flag &&
            name.empty() &&
            !fmt &&
            !div &&
            !volleft &&
            !volright &&
            data.empty();
    }
};

///Sequence Group Fields
struct seqdgrp {
    unsigned flag;
    std::vector<seqdseq> seq;
    
    bool empty() const { return !flag && seq.empty(); }
};

///Sequence Definition Fields Main
struct seqdinfo {
    unsigned flag;
    std::vector<seqdgrp> seqd;
    
    bool empty() const { return !flag && seqd.empty(); }
};

///Waveform Definition Fields
struct wavewav {
    unsigned flag;
    std::string name;
    //char codec;
    char chns;
    char numloop;
    //unsigned char res0;
    signed smprate;
    signed rate0;
    signed rate1;
    short volleft;
    short volright;
    signed looppos;
    signed loopsmp;
    signed loopbeg;
    signed loopend;
    //signed strmsize;
    //signed strmbeg;
    //signed strmend;
    std::vector<short> pcm;
};

///Waveform Definition Fields Main
struct waveinfo {
    unsigned flag;
    std::vector<wavewav> wave;
    
    bool empty() const { return !flag && wave.empty(); }
};

///Waveform Surround Sound Fields
struct wsursur {
    unsigned char flag;
    char chnid;
    short angle;
    short dist;
    short level;
};

///Waveform Surround Sound Fields Main
struct wsurinfo {
    unsigned flag;
    std::vector<std::vector<wsursur>> wsur;
    
    bool empty() const { return !flag && wsur.empty(); }
};

///Waveform Marker Fields
struct wmkrmkr {
    std::string labl;
    signed pos;
    signed siz;
};

///Waveform Marker Fields Main
struct wmkrinfo {
    unsigned flag;
    std::vector<std::vector<wmkrmkr>> wmkr;
    
    bool empty() const { return !flag && wmkr.empty(); }
};

///Configuration Fields
struct confcnf {
    unsigned flag;
    std::string name;
    std::string text;
};

///Configuration Fields Main
struct confinfo {
    unsigned flag;
    std::vector<confcnf> conf;
    
    bool empty() const { return !flag && conf.empty(); }
};

///Tuning Fields
struct tunetun {
    unsigned flag;
    signed labl;
    signed type;
    signed defn;
    //unsigned res0;
    //signed num;
    std::string name;
    std::vector<unsigned char> data;
};

///Tuning Fields
struct tuneinfo {
    unsigned flag;
    std::vector<tunetun> tune;
    
    bool empty() const { return !flag && tune.empty(); }
};

///ADSR Fields
struct adsrinfo {
    unsigned flag;
    std::vector<unsigned> adsr;
    
    bool empty() const { return !flag && adsr.empty(); }
};

///Name Fields
struct namenam {
    signed short reqsmp;
    unsigned char reqseq;
    unsigned char type;
    std::string name;
};

///Name Fields Main
struct nameinfo {
    unsigned flag;
    std::vector<namenam> name;
    
    bool empty() const { return !flag && name.empty(); }
};

///SGXD Fields
struct sgxdinfo {
    std::string file;
    bussinfo    buss;
    rgndinfo    rgnd;
    seqdinfo    seqd;
    waveinfo    wave;
    wsurinfo    wsur;
    wmkrinfo    wmkr;
    confinfo    conf;
    tuneinfo    tune;
    adsrinfo    adsr;
    nameinfo    name;
    
    bool empty() const {
        return
            file.empty() &&
            buss.empty() &&
            rgnd.empty() &&
            seqd.empty() &&
            wave.empty() &&
            wsur.empty() &&
            wmkr.empty() &&
            conf.empty() &&
            tune.empty() &&
            adsr.empty() &&
            name.empty();
    }
};


#endif
