#ifndef LBRT_TYPES_HPP
#define LBRT_TYPES_HPP

#include <string>
#include <vector>

///LBRT Message Info
struct lbrtmesg {
    signed id;
    signed dtim;
    signed tval;
    signed val0;
    short val1;
    short val2;
    short velon;
    short bndon;
    char chn;
    char stat;
    char veloff;
    char bndoff;

    bool operator==(const lbrtmesg &m) const = default;
};

///LBRT Track Info
struct lbrttrck {
    unsigned id;
    unsigned unk0;
    std::vector<lbrtmesg> msgs;
    std::vector<unsigned> qrts;

    bool operator==(const lbrttrck &t) const = default;
};

///LBRT Track Fields
struct lbrtinfo {
    //Header
    //const unsigned lbrt = 0x4C425254;
    unsigned soff;
    unsigned tpc;
    signed ppqn;
    //Sub Header
    std::vector<lbrttrck> trks;

    std::string path, name;

    bool empty() const {
        return
            !soff && !tpc && !ppqn &&
            trks.empty() && path.empty() && name.empty();
    }
};


#endif
