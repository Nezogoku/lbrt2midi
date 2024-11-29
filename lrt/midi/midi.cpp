#include <vector>
#include "midi_func.hpp"
#include "midi_forms.hpp"
#include "midi_const.hpp"
#include "midi_types.hpp"


///Unpacks variable MIDI messages from MTRK data
std::vector<mesginfo> unpackMesg(unsigned char *in, const unsigned length) {
    if (!in || length < 2) return {};
    
    const unsigned char *in_end = in + length;
    short t_st = STAT_NONE;
    std::vector<unsigned char> t_vl;
    unsigned t_ab = 0;
    std::vector<mesginfo> out;

    auto get_vlv = [&in]() -> unsigned {
        unsigned out = 0;
        while ((out |= in[0] & 0x7F) && (*(in++) & 0x80)) { out <<= 7; }
        return out;
    };
    
    while (in < in_end) {
        //Get delta time
        t_ab += get_vlv();

        //Get status
        if (!(in[0] & 0x80) && (t_st == STAT_NONE)) continue;
        else if (in[0] & 0x80) t_st = *(in++);

        //Get message
        switch (t_st & 0xF0) {
            //Has 2 data bytes
            case STAT_NOTE_OFF:
            case STAT_NOTE_ON:
            case STAT_KEY_PRESSURE:
            case STAT_CONTROLLER:
            case STAT_PITCH_WHEEL:
                t_vl.push_back(*(in++));
            //Has 1 data byte
            case STAT_PROGRAMME_CHANGE:
            case STAT_CHANNEL_PRESSURE:
                t_vl.push_back(*(in++));
                break;
            case 0xF0:
                if (t_st == STAT_RESET) t_st = (t_st << 8) | *(in++);
                
                if (t_st == STAT_SYSTEM_EXCLUSIVE ||
                    t_st == STAT_SYSTEM_EXCLUSIVE_STOP ||
                    t_st < 0) {
                    
                    t_vl.resize(get_vlv());
                    for (auto &v : t_vl) v = *(in++);
                }
                else {
                    switch (t_st) {
                        case STAT_SEQUENCE_POINTER:
                            t_vl.push_back(*(in++));
                        case STAT_QUARTER_FRAME:
                        case STAT_SEQUENCE_REQUEST:
                            t_vl.push_back(*(in++));
                        default:
                            break;
                    }
                }
                break;
        }
        out.emplace_back(t_ab, t_st, t_vl.data(), t_vl.size());

        //Cancel running status if applicable
        if (t_st > STAT_NONE && t_st < STAT_SYSTEM_EXCLUSIVE);
        else t_st = STAT_NONE;
        
        t_vl.clear();
    }
    
    return out;
}

///Unpacks MIDI info from MIDI data
midiinfo unpackMidi(unsigned char *in, const unsigned length) {
    if (!in || length < 14) return {};
    
    const unsigned char *in_end = in + length;
    midiinfo out;
    
    auto get_int = [&in](unsigned length) -> unsigned {
        unsigned out = 0;
        for (int i = 0; i < length; ++i) {
            out = (out << 8) | *(in++);
        }
        return out;
    };
    
    if (get_int(4) != FOURCC_MThd) return {};
    if (get_int(4) != 6) return {};
    out.fmt = get_int(2);
    out.trk = get_int(2);
    out.div = get_int(2);
    
    while (in < in_end) {
        unsigned t_sz;
        
        if (get_int(4) != FOURCC_MTrk) break;
        t_sz = get_int(4);
        if (in + t_sz > in_end) break;
        
        out.msg.emplace_back(unpackMesg(in, t_sz));
        in += t_sz;
    }
    
    return out;
}


///Packs MIDI data from MIDI info
std::vector<unsigned char> packMidi(const midiinfo &midi) {
    if (midi.fmt > MIDI_MULTIPLE_SONG) return {};
    else if (midi.trk != midi.msg.size()) return {};
    else if (midi.msg.empty()) return {};
    
    std::vector<unsigned char> out;
    
    auto set_int = [&out](const unsigned in, int length) -> void {
        while (length--) out.push_back((in >> (8 * length)) & 0xFF);
    };
    auto get_siz = [](const std::vector<mesginfo> &in) -> unsigned {
        const mesginfo *prv = 0;
        unsigned siz = 0;
        for (const auto &m : in) {
            siz += m.size(prv);
            prv = &m;
            if (m.getStat() == META_END_OF_SEQUENCE) break;
        }
        return siz;
    };
    auto set_dat = [&out](const std::vector<mesginfo> &in) -> void {
        const mesginfo *prv = 0;
        for (const auto &m : in) {
            auto dat = m.getAll(prv);
            prv = &m;
            out.insert(out.end(), dat.begin(), dat.end());
            if (m.getStat() == META_END_OF_SEQUENCE) break;
        }
    };
    
    //Set MIDI header
    set_int(FOURCC_MThd, 4);
    set_int(0x06, 4);
    set_int(midi.fmt, 2);
    set_int(midi.trk, 2);
    set_int(midi.div, 2);
    
    //Set MIDI tracks
    for (const auto &trk : midi.msg) {
        set_int(FOURCC_MTrk, 4);
        set_int(get_siz(trk), 4);
        set_dat(trk);
    }
    
    return out;
}