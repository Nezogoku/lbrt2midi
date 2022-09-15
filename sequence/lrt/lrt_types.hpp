#ifndef LRT_TYPES_HPP
#define LRT_TYPES_HPP

#include <algorithm>
#include <cstdint>
#include <vector>
#include "../../types.hpp"


//Length and array for custom ordering events
//Because I don't know a C++ alternative to R's order(match()) function
const int numStat = 5;
const uchar statOrder[] = {0xF0, 0xC0, 0x90, 0xE0, 0xB0};

//Struct for MIDI events
struct lbrt_status {
    int quarter_note_id;        //Beat in a measure
    uint32_t absol_t;           //Absolute time
    uchar status_a,             //Event type
          status_b;             //Value or sub-event type
    std::vector<uchar> values;  //Values of event

    lbrt_status(int qnid, uint32_t ms, uchar stata, uchar statb, std::vector<uchar> val) :
        quarter_note_id(qnid), absol_t(ms), status_a(stata), status_b(statb), values(val) {}

    //To make sorting in ascending order easier
    bool operator<(const lbrt_status &lrt) const {
        int s0 = std::distance(statOrder, std::find(statOrder, statOrder + numStat, (status_a & 0xF0))),
            s1 = std::distance(statOrder, std::find(statOrder, statOrder + numStat, (lrt.status_a & 0xF0)));

        return (absol_t < lrt.absol_t) || ((absol_t == lrt.absol_t) && (s0 < s1));
    }
};

#endif
