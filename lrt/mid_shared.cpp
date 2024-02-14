#include <algorithm>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>
#include "directory.hpp"
#include "mid_types.hpp"
#include "mid_cc.hpp"
#include "mid_sysex.hpp"
#include "mid_shared.hpp"


///Reads MIDI from file
int midi::setMidi(std::string mid_file) {
    unsigned char *mid_data = 0;
    unsigned mid_size = 0;

    if (!getFileData(mid_file.c_str(), mid_data, mid_size)) return 0;
    setMidi(mid_data, mid_size);

    return 1;
}

///Reads MIDI from memory
void midi::setMidi(unsigned char *in, unsigned length) {
    const unsigned char *in_end = in + length;
    auto cmp_str = [&](const char *in1, int length) -> bool {
        while ((*(in++) == (unsigned char)(*(in1++))) && --length);
        return !length;
    };
    auto get_int = [&](int length) -> unsigned {
        unsigned out = 0;
        while (length--) out = (out << 8) | *(in++);
        return out;
    };

    if (!cmp_str("MThd", 4)) return;
    else if (get_int(4) != 0x06) return;
    this->mhead.mthd_frmt = get_int(4);
    this->mhead.mthd_trks = get_int(2);
    this->mhead.mthd_divi = get_int(2);

    if (!this->mhead.mthd_trks) return;
    switch(this->mhead.mthd_frmt) {
        case MIDI_SINGLE_TRACK:
            if (this->mhead.mthd_trks != 1) return;
            else break;
        case MIDI_MULTIPLE_TRACK: break;
        case MIDI_MULTIPLE_SONG: break;
        default: return;
    }

    this->mmsgs.resize(this->mhead.mthd_trks);
    for (int t = 0; t < this->mhead.mthd_trks; ++t) {
        unsigned tsize;
        if (cmp_str("MTrk", 4) && (tsize = get_int(4)) < (in_end - in)) {
            setMidiRaw(in, tsize, t);
            in += tsize;
        }
        else return;
    }
}

///Reads MIDI track from memory
void midi::setMidiRaw(unsigned char *in, unsigned length, int track_id) {
    if (this->mmsgs.size() < track_id + 1) this->mmsgs.resize(track_id + 1);

    const unsigned char *in_end = in + length;
    unsigned short tstat = STAT_NONE;
    std::vector<unsigned char> tval;
    unsigned char tmsiz = 0;
    unsigned tabst = 0;

    auto get_vlv = [&]() -> unsigned {
        unsigned out = 0;
        while ((out |= in[0] & 0x7F) && (*(in++) & 0x80)) { out <<= 7; }
        return out;
    };

    //Get MIDI messages
    while (in < in_end) {
        tabst += get_vlv();

        if (!(in[0] & 0x80) && (tstat == STAT_NONE)) continue;
        else if (in[0] & 0x80) tstat = *(in++);

        switch (tstat & 0xF0) {
            case STAT_NOTE_OFF:
            case STAT_NOTE_ON:
            case STAT_KEY_PRESSURE:
            case STAT_CONTROLLER:
            case STAT_PITCH_WHEEL:
                tval.push_back(*(in++));
            case STAT_PROGRAMME_CHANGE:
            case STAT_CHANNEL_PRESSURE:
                tval.push_back(*(in++));
                break;
            case 0xF0:
                switch (tstat) {
                    case STAT_RESET:
                        tstat = (tstat << 8) | *(in++);
                    case STAT_SYSTEM_EXCLUSIVE:
                    case STAT_SYSTEM_EXCLUSIVE_STOP:
                        tmsiz = *(in++);
                        while (tval.size() < tmsiz) tval.push_back(*(in++));
                        break;
                    case STAT_SEQUENCE_POINTER:
                        tval.push_back(*(in++));
                    case STAT_QUARTER_FRAME:
                    case STAT_SEQUENCE_REQUEST:
                        tval.push_back(*(in++));
                    default:
                        break;
                }
                break;
        }
        this->mmsgs[track_id].emplace_back(tabst, tstat, tval.size(), tval.data());

        if (tstat >= STAT_SYSTEM_EXCLUSIVE) tstat = STAT_NONE;
        if (tmsiz) { tmsiz = 0; tval.clear(); }
    }
}


void midi::setFormat(unsigned char frmt) {

}

///Optimize MIDI tracks
void midi::optimizeMidi() {
    if (this->mmsgs.empty()) return;
    
    auto is_seq_id = [](const midi_mesg &m) -> bool { return m.mmsg_stat == META_SEQUENCE_ID; };
    auto is_copyr = [](const midi_mesg &m) -> bool { return m.mmsg_stat == META_COPYRIGHT; };
    auto is_track_name = [](const midi_mesg &m) -> bool { return m.mmsg_stat == META_TRACK_NAME; };
    auto is_inst_name = [](const midi_mesg &m) -> bool { return m.mmsg_stat == META_INSTRUMENT_NAME; };
    auto is_tempo = [](const midi_mesg &m) -> bool { return m.mmsg_stat == META_TEMPO; };
    auto is_smpte = [](const midi_mesg &m) -> bool { return m.mmsg_stat == META_SMPTE; };
    auto is_time_sig = [](const midi_mesg &m) -> bool { return m.mmsg_stat == META_TIME_SIGNATURE; };
    auto is_key_sig = [](const midi_mesg &m) -> bool { return m.mmsg_stat == META_KEY_SIGNATURE; };
    auto is_end_seq = [](const midi_mesg &m) -> bool { return m.mmsg_stat == META_END_OF_SEQUENCE; };
    
    auto is_global = [&](const midi_mesg &m) -> bool {
        return is_copyr(m) || is_track_name(m) || is_tempo(m) ||
               is_smpte(m) || is_time_sig(m) || is_key_sig(m);
    };
    auto is_init = [&](const midi_mesg &m) -> bool {
        return is_seq_id(m) || is_copyr(m) || is_track_name(m) || is_smpte(m);
    };

    //Give Format 1 MIDI to global track if applicable
    if (this->mhead.mthd_frmt == MIDI_MULTIPLE_TRACK) {
        //Check if first track has a non-global message
        if (std::find_if_not(this->mmsgs[0].begin(), this->mmsgs[0].end(), is_global) != this->mmsgs[0].end()) {
            std::vector<midi_mesg> glbl;

            //Move most global messages to global track
            for (auto &trk : this->mmsgs) {
                int num_gtnam = 0,
                    num_ttnam = std::count_if(trk.begin(), trk.end(), is_track_name);
                
                //Only move one Track_name message to global
                //Only if current track has multiple Track_name messages
                for (auto itr = trk.begin(); itr < trk.end();) {
                    if (is_global(*itr) && (!is_track_name(*itr) ||
                                            (is_track_name(*itr) && !num_gtnam++ && num_ttnam-- > 1))) {
                        glbl.push_back(*itr);
                        itr = trk.erase(itr);
                    }
                    else itr += 1;
                }
            }

            //Insert global track
            this->mmsgs.insert(this->mmsgs.begin(), glbl);
        }
    }

    //Misc optimizations
    unsigned abs = 0;
    for (auto &trk : this->mmsgs) {
        int num_tseqi = 0, num_tcpyr = 0, num_ttnam = 0, num_tinam = 0, num_tsmpt = 0;

        for (auto itr = trk.begin(); itr < trk.end();) {
            if (is_seq_id(*itr) && num_tseqi++) { itr = trk.erase(itr); continue; }
            else if (is_copyr(*itr) && num_tcpyr++) (*itr).mmsg_stat = META_TEXT;
            else if (is_track_name(*itr) && num_ttnam++) (*itr).mmsg_stat = META_TEXT;
            else if (is_inst_name(*itr) && num_tinam++) (*itr).mmsg_stat = META_TEXT;
            else if (is_smpte(*itr) && num_tsmpt++) { itr = trk.erase(itr); continue; }
            
            if (is_init(*itr)) (*itr).mmsg_time = 0;
            itr += 1;
        }

        //Get greatest absolute time
        abs = std::max(abs, (*std::max_element(trk.begin(), trk.end())).mmsg_time);
    }

    //Remove empty tracks
    for (auto itr = this->mmsgs.begin(); itr < this->mmsgs.end();) {
        if ((*itr).empty()) itr = this->mmsgs.erase(itr);
        else itr += 1;
    }

    //Ensure final event is End_of_sequence message
    for (auto &trk : this->mmsgs) {
        if (this->mhead.mthd_frmt == MIDI_MULTIPLE_SONG) {
            abs = (*std::max_element(trk.begin(), trk.end())).mmsg_time;
        }
        
        //Remove all End_of_sequence messages
        trk.erase(std::remove_if(trk.begin(), trk.end(), is_end_seq), trk.end());

        //Insert End_of_sequence message at end
        trk.emplace_back(abs, META_END_OF_SEQUENCE);
    }
}


///Writes CSV to specified file
int midi::getCsv(std::string csv_file) {
    try {
        std::string ext = csv_file.substr(csv_file.find_last_of('.'));
        if (ext != ".csv" || ext != ".txt") {
            auto rep = csv_file.rfind(ext);
            csv_file.replace(rep, rep + ext.length(), ".csv");
        }
    } catch (std::out_of_range) { csv_file += ".csv"; }

    std::string csv = getCsv();
    return createFile(csv_file.c_str(), csv.c_str(), csv.length());
}

///Writes CSV to string
std::string midi::getCsv() {
    std::string csv = "";
    auto set_fstr = [&]<typename... T>(std::string in, T... args) -> void {
        std::variant<std::string, unsigned, signed> ftmp[] = {args...};
        for (auto &tmp : ftmp) {
            std::string istr;
            switch (tmp.index()) {
                case 0: istr = std::get<0>(tmp); break;
                case 1: istr = std::to_string(std::get<1>(tmp)); break;
                case 2: istr = std::to_string(std::get<2>(tmp)); break;
            }
            auto ifnd = in.find_first_of('*');
            if (ifnd != std::string::npos) in.replace(ifnd, 1, istr);
        }
        csv += in;
    };
    int tid = 1;

    set_fstr("0, 0, HEADER, *, *, *\n", (short)this->mhead.mthd_frmt, this->mhead.mthd_trks, this->mhead.mthd_divi);
    for (auto &trks : this->mmsgs) {
        set_fstr("*, 0, START_TRACK\n", tid);

        for (auto &trk : trks) {
            std::string txt = "";

            if (trk.mmsg_stat == STAT_NONE || trk.mmsg_stat == META_NONE) continue;
            else if ((short)trk.mmsg_stat > 0) {
                switch (trk.mmsg_stat & 0xF0) {
                    case STAT_NOTE_OFF:         txt = "NOTE_OFF_C"; break;
                    case STAT_NOTE_ON:          txt = "NOTE_ON_C"; break;
                    case STAT_KEY_PRESSURE:     txt = "POLY_AFTERTOUCH_C"; break;
                    case STAT_CONTROLLER:       txt = "CONTROL_C"; break;
                    case STAT_PROGRAMME_CHANGE: txt = "PROGRAM_C"; break;
                    case STAT_CHANNEL_PRESSURE: txt = "CHANNEL_AFTERTOUCH_C"; break;
                    case STAT_PITCH_WHEEL:      txt = "PITCH_BEND_C"; break;
                    case 0xF0:
                        switch (trk.mmsg_stat) {
                            case STAT_SYSTEM_EXCLUSIVE_STOP:
                                txt = "_PACKET";
                            case STAT_SYSTEM_EXCLUSIVE:
                                txt = "PITCH_BEND_C" + txt + ", "; break;
                            default: continue;
                        }
                        break;
                }
                set_fstr("*, *, *, *", tid, trk.mmsg_time, txt, trk.mmsg_stat & 0x0F);
                if (txt == "PITCH_BEND_C") {
                    set_fstr(", *", (unsigned short)trk.mmsg_data[0] << 7 | trk.mmsg_data[1]);
                }
                else for (auto &val : trk.mmsg_data) set_fstr(", *", val);
                csv += "\n";
            }
            else {
                switch (trk.mmsg_stat) {
                    case META_SEQUENCE_ID:          txt = "SEQUENCE_NUMBER"; break;
                    case META_TEXT:                 txt = "TEXT_T"; break;
                    case META_COPYRIGHT:            txt = "COPYRIGHT_T"; break;
                    case META_TRACK_NAME:           txt = "TITLE_T"; break;
                    case META_INSTRUMENT_NAME:      txt = "INSTRUMENT_NAME_T"; break;
                    case META_LYRICS:               txt = "LYRIC_T"; break;
                    case META_MARKER:               txt = "MARKER_T"; break;
                    case META_CUE:                  txt = "CUE_POINT_T"; break;
                    case META_PATCH_NAME:           txt = "NOTE_ON_C"; break;
                    case META_PORT_NAME:            txt = "POLY_AFTERTOUCH_C"; break;
                    case META_MISC_TEXT_A:
                    case META_MISC_TEXT_B:
                    case META_MISC_TEXT_C:
                    case META_MISC_TEXT_D:
                    case META_MISC_TEXT_E:
                    case META_MISC_TEXT_F:          txt = "TEXT_T"; break;
                    case META_CHANNEL_PREFIX:       txt = "CHANNEL_PREFIX"; break;
                    case META_PORT:                 txt = "MIDI_PORT"; break;
                    case META_END_OF_SEQUENCE:      txt = "END_TRACK"; break;
                    case META_TEMPO:                txt = "TEMPO"; break;
                    case META_SMPTE:                txt = "SMPTE_OFFSET"; break;
                    case META_TIME_SIGNATURE:       txt = "TIME_SIGNATURE"; break;
                    case META_KEY_SIGNATURE:        txt = "KEY_SIGNATURE"; break;
                    case META_SEQUENCER_EXCLUSIVE:  txt = "SEQUENCER_SPECIFIC"; break;
                    default:                        txt = "UNKNOWN_META_EVENT"; break;
                }
                set_fstr("*, *, *", tid, trk.mmsg_time, txt);
                if (txt == "UNKNOWN_META_EVENT") set_fstr(", *", trk.mmsg_stat & 0xFF);
                if (txt == "SEQUENCER_SPECIFIC" || txt == "UNKNOWN_META_EVENT") set_fstr(", *", (unsigned)trk.mmsg_data.size());
                if (txt == "SEQUENCE_NUMBER") set_fstr(", *", (unsigned short)trk.mmsg_data[0] << 8 | trk.mmsg_data[1]);
                else if (trk.mmsg_stat < 0xFF10) {
                    std::string val(trk.mmsg_data.data(), trk.mmsg_data.data() + trk.mmsg_data.size());
                    set_fstr(", *", val);
                }
                else if (txt == "END_TRACK") { csv += "\n"; break; }
                else if (txt == "KEY_SIGNATURE") set_fstr("*, *", trk.mmsg_data[0], (char)trk.mmsg_data[1]);
                else for (auto &val : trk.mmsg_data) set_fstr(", *", val);
                csv += "\n";
            }
        }

        tid += 1;
    }
    csv += "0, 0, END_OF_FILE\n";

    return csv;
}


///Writes MIDI to specified file
int midi::getMidi(std::string mid_file) {
    try {
        std::string ext = mid_file.substr(mid_file.find_last_of('.'));
        if (ext != ".mid" || ext != ".smf") {
            auto rep = mid_file.rfind(ext);
            mid_file.replace(rep, rep + ext.length(), ".mid");
        }
    } catch (std::out_of_range) { mid_file += ".mid"; }

    unsigned char *mid_data = 0; unsigned mid_size = 0;
    getMidi(mid_data, mid_size);
    return createFile(mid_file.c_str(), mid_data, mid_size);
}

///Writes MIDI to pointer array
void midi::getMidi(unsigned char *&mid_data, unsigned &mid_size) {
    if (mid_data) { delete[] mid_data; mid_data = 0; }
    
    std::vector<unsigned char> out;
    auto set_arr = [](std::vector<unsigned char>&tmp, const std::vector<unsigned char>in) -> void {
        if (in.empty()) tmp.push_back(0);
        else tmp.insert(tmp.end(), in.begin(), in.end());
    };
    auto set_str = [&](std::vector<unsigned char>&tmp, const char *in, int length) -> void {
        if (!in or length < 0) return;
        set_arr(tmp, std::vector<unsigned char>(in, in + length));
    };
    auto set_int = [](std::vector<unsigned char>&tmp, const unsigned int in, int length) -> void {
        while (length--) tmp.push_back((in >> (8 * length)) & 0xFF);
    };

    //Set MIDI header
    set_str(out, "MThd", 4);
    set_int(out, 0x06, 4);
    set_int(out, this->mhead.mthd_frmt, 2);
    set_int(out, this->mhead.mthd_trks, 2);
    set_int(out, this->mhead.mthd_divi, 2);

    //Set MIDI tracks
    for (auto &trks : this->mmsgs) {
        std::vector<unsigned char> tout;
        auto set_vlv = [&](unsigned int in) -> void {
            unsigned vlv = in & 0x7F;
            while (in >>= 7) { vlv = (vlv << 8) | ((in & 0x7F) | 0x80); }
            do { tout.push_back(vlv & 0xFF); vlv >>= 8; } while (tout.back() & 0x80);
        };

        //Get MIDI track messages
        int abs = 0;
        for (auto &trk : trks) {
            if (trk.mmsg_stat == STAT_NONE || trk.mmsg_stat == META_NONE);
            else if ((short)trk.mmsg_stat > 0) {
                set_vlv(trk.mmsg_time - abs);
                set_int(tout, trk.mmsg_stat, 1);
                if (trk.mmsg_stat == STAT_SYSTEM_EXCLUSIVE ||
                    trk.mmsg_stat == STAT_SYSTEM_EXCLUSIVE_STOP) set_int(tout, trk.mmsg_data.size(), 1);
                set_arr(tout, trk.mmsg_data);
            }
            else {
                set_vlv(trk.mmsg_time - abs);
                set_int(tout, trk.mmsg_stat, 2);
                set_int(tout, trk.mmsg_data.size(), 1);
                set_arr(tout, trk.mmsg_data);
                if (trk.mmsg_stat == META_END_OF_SEQUENCE) break;
            }
            abs = trk.mmsg_time;
        }

        //Set MIDI track
        set_str(out, "MTrk", 4);
        set_int(out, tout.size(), 4);
        out.insert(out.end(), tout.begin(), tout.end());
        if (!tout.empty()) tout.clear();
    }

    //Set MIDI
    mid_size = out.size();
    mid_data = new unsigned char[mid_size] {};
    std::move(out.begin(), out.end(), mid_data);
}
