#ifndef MIDI_TYPES_HPP
#define MIDI_TYPES_HPP

#include <vector>
#include "midi_const.hpp"

///MIDI Message Fields
struct mesginfo {
    ~mesginfo() = default;
    mesginfo(const unsigned t_t = 0, const short t_s = STAT_NONE) :
        time(t_t), data() {
            if (t_s <= STAT_NONE || t_s >= STAT_SYSTEM_EXCLUSIVE) { stat = t_s; chan = -1; }
            else { stat = t_s & 0xF0; chan = t_s & 0x0F; }
        }
    mesginfo(const unsigned t_t, const short t_s, const unsigned char *t_d, const int t_z) :
        mesginfo{t_t, t_s} { if (t_z > 0) data.assign(t_d, t_d + t_z); }
    mesginfo(const unsigned t_t, const short t_s, const char *t_d) :
        mesginfo{t_t, t_s} { for(int d = 0; t_d[d]; ++d) data.push_back(t_d[d]); }
    template <int t_Z>
    mesginfo(const unsigned t_t, const short t_s, const unsigned char (&t_d)[t_Z]) :
        mesginfo{t_t, t_s, t_d, t_Z} {}
    mesginfo(const mesginfo &m) = default;
    mesginfo(mesginfo &&m) = default;
    
    mesginfo& operator=(const mesginfo &m) = default;
    mesginfo& operator=(mesginfo &&m) = default;
    
    //To make comparing easier
    auto operator<=>(const mesginfo &m) const {
        if (time < m.time) return std::strong_ordering::less;
        if (time > m.time) return std::strong_ordering::greater;
        if (get_id(stat) < get_id(m.stat)) return std::strong_ordering::less;
        if (get_id(stat) > get_id(m.stat)) return std::strong_ordering::greater;
        if (chan < m.chan) return std::strong_ordering::less;
        if (chan > m.chan) return std::strong_ordering::greater;
        if (data < m.data) return std::strong_ordering::less;
        if (data > m.data) return std::strong_ordering::greater;
        return std::strong_ordering::equal;
    }
    bool operator<(const mesginfo &m) const = default;
    bool operator>(const mesginfo &m) const = default;
    bool operator==(const mesginfo &m) const = default;
    bool operator<=(const mesginfo &m) const = default;
    bool operator!=(const mesginfo &m) const = default;
    bool operator>=(const mesginfo &m) const = default;
    
    //Get size of current message from pointer to previous message
    unsigned size(const mesginfo *m = 0) const {
        if (stat == META_NONE || STAT_NONE) return 0;
        
        unsigned out = 0; auto sid = get_id(stat);
        auto clc_vlv = [](unsigned v0) -> unsigned {
            unsigned out = 0;
            do { out += 1; } while (v0 >>= 7);
            return out;
        };
        
        //Size of delta time
        out += clc_vlv(time - (!m ? 0 : m[0].getTime()));
        //Size of status if applicable
        if (m && stat == m[0].getStat() && sid > 0x27 && sid < 0x2F);
        else out += (stat < 0) ? 2 : 1;
        //Size of size if applicable
        if (stat == STAT_SYSTEM_EXCLUSIVE ||
            stat == STAT_SYSTEM_EXCLUSIVE_STOP ||
            stat < 0) {
            out += clc_vlv(data.size());
        }
        //Size of data
        out += data.size();
        return out;
    }
    //Check if empty
    bool empty() const { return *this == mesginfo{}; }
    //Clear messages
    void clear() { *this = {}; }
    //Set message stuff
    void setTime(const unsigned &tm) { time = tm; }
    void setStat(const short &st) { stat = st; }
    void setChan(const char &ch) { chan = ch; }
    void setData(const std::vector<unsigned char> &dt) {
        data.insert(data.end(), dt.begin(), dt.end());
    }
    void setData(const unsigned char &vl, const unsigned &id) {
        if (id < data.size()) data[id] = vl;
    }
    //Get message stuff
    unsigned getTime() const { return time; }
    short getStat() const { return stat; }
    char getChan() const { return chan; }
    std::vector<unsigned char> getData() const { return data; }
    std::vector<unsigned char> getAll(const mesginfo *m = 0) const {
        if (stat == META_NONE || STAT_NONE) return {};
        
        std::vector<unsigned char> out; auto sid = get_id(stat);
        auto set_vlv = [&out](unsigned v0) -> void {
            unsigned v1 = v0 & 0x7F;
            while (v0 >>= 7) { v1 = (v1 << 8) | ((v0 & 0x7F) | 0x80); }
            do { out.push_back(v1 & 0xFF); v1 >>= 8; } while (out.back() & 0x80);
        };
        
        //Delta time
        set_vlv(time - (!m ? 0 : m[0].getTime()));
        //Status
        if (m && stat == m[0].getStat() && sid > 0x27 && sid < 0x2F);
        else {
            if (stat < 0) out.push_back(stat >> 8);
            out.push_back((stat | ((chan < 0) ? 0 : chan)) & 0xFF);
        }
        //Size
        if (stat == STAT_SYSTEM_EXCLUSIVE ||
            stat == STAT_SYSTEM_EXCLUSIVE_STOP ||
            stat < 0) {
            set_vlv(data.size());
        }
        //Data
        out.insert(out.end(), data.begin(), data.end());
        return out;
    }
    
    private:
        unsigned time;
        short stat;
        char chan;
        std::vector<unsigned char> data;
        
        unsigned char get_id(const short &s0) const {
            for (const auto &S : MIDISTATUS_ORDER) { if (s0 == S) return &S - MIDISTATUS_ORDER; }
            return sizeof(MIDISTATUS_ORDER)/sizeof(MidiStatus);
        }
};

struct midiinfo {
    //const unsigned MThd = 0x4D546864;
    //const unsigned size = 0x06;
    unsigned short fmt;
    unsigned short trk;
    unsigned short div;
    std::vector<
        //const unsigned MTrk = 0x4D54726B;
        std::vector<mesginfo>
    > msg;
    
    //To make setting division easier
    void setDivision(const short t0, const short t1 = -1) {
        if (t1 & 0xFF00) div = t0 & 0x7FFF;
        else div = 0x8000 | t0 << 8 | t1 & 0xFF;
    }
};


#endif
