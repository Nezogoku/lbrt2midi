#include <algorithm>
#include <iterator>
#include <string>
#include <variant>
#include <vector>
#include "mid_types.hpp"
#include "mid_cc.hpp"
#include "mid_sysex.hpp"
#include "mid_meta.hpp"
#include "mid_shared.hpp"


void midi::reset() {
    for (int t = 0; t < this->mhead.mthd_trks; ++t) {
        if (this->mmsgs[t]) delete[] this->mmsgs[t]; this->mmsgs[t] = 0;
    }
    if (this->mmsgs) delete[] this->mmsgs; this->mmsgs = 0;
    if (this->amnt_mmsg) delete[] this->amnt_mmsg; this->amnt_mmsg = 0;
    this->mhead = midi_mthd();
}

template<typename... Args>
std::string midi::setFstr(const char *in, Args... args) {
    std::variant<std::string, unsigned char*, unsigned int, int> ftmp[] = {args...};
    std::string out = "";

    while (in[0]) {
        if (in[0] == '{') {
            int p = in[1] - '0';
            switch(ftmp[p].index()) {
                case 0: out += std::get<0>(ftmp[p]); break;
                case 1: out += (const char*)(std::get<1>(ftmp[p])); break;
                case 2: out += std::to_string(std::get<2>(ftmp[p])); break;
                case 3: out += std::to_string(std::get<3>(ftmp[p])); break;
                default: out += *(in++); continue;
            }
            in += 3;
        }
        else out += *(in++);
    }
    return out;
}


void midi::setMidiRaw(unsigned char *in, unsigned length, int track_id) {
    const unsigned char *in_end = in + length;
    unsigned tabst = 0, tstat = STAT_NONE;
    unsigned char *tmdat = 0, tmstat = META_NONE, tmsiz = 0;
    std::vector<midi_mesg> ttrk;
    
    auto get_vlv = [&]() -> unsigned {
        unsigned out = 0;
        while ((out |= in[0] & 0x7F) && (*(in++) & 0x80)) { out <<= 7; }
        return out;
    };
    auto get_bnd = [&](unsigned short in) -> unsigned short {
        unsigned short out = in & 0x7F; in >>= 7; out = (out << 8) | (in & 0x7F);
        return out;
    };
    auto get_mesg_size = [&]() -> MidiMessageSize {
        if (tstat < STAT_PROGRAMME_CHANGE) return STAT_MESSAGE_DOUBLE;
        else if (tstat < STAT_PITCH_WHEEL) return STAT_MESSAGE_SINGLE;
        else if (tstat < STAT_SYSTEM_EXCLUSIVE) return STAT_MESSAGE_DOUBLE;
        else {
            switch (tstat) {
                case STAT_QUARTER_FRAME:
                case STAT_SEQUENCE_REQUEST:
                    return STAT_MESSAGE_SINGLE;
                case STAT_SEQUENCE_POINTER:
                    return STAT_MESSAGE_DOUBLE;
                case STAT_SYSTEM_EXCLUSIVE:
                case STAT_SYSTEM_EXCLUSIVE_STOP:
                case STAT_RESET:
                    return STAT_MESSAGE_VARIABLE;
                default:
                    return STAT_MESSAGE_NONE;
            }
        }
    };
    
    //Get MIDI messages
    while (in < in_end) {
        tabst += get_vlv();

        if (!(in[0] & 0x80) && (tstat == STAT_NONE)) continue;
        else if (in[0] & 0x80) tstat = *(in++);
        
        auto msiz = get_mesg_size();
        if (msiz == STAT_MESSAGE_NONE) {
            ttrk.emplace_back(tabst, tstat);
        }
        else if (msiz == STAT_MESSAGE_SINGLE) {
            unsigned char tval = *(in++);
            ttrk.emplace_back(tabst, tstat, tval);
        }
        else if (msiz == STAT_MESSAGE_DOUBLE) {
            unsigned short tval = 0;
            for (int v = 0; v < 2; ++v) tval = (tval << 8) | *(in++);
            if (tstat == STAT_PITCH_WHEEL) tval = get_bnd(tval);
            ttrk.emplace_back(tabst, tstat, tval);
        }
        else if (msiz == STAT_MESSAGE_VARIABLE) {
            std::vector<unsigned char> tval;
            while (true) {
                tval.push_back(*(in++));
                if (tstat == STAT_RESET) {
                    if (tval.size() > 2 && (tval.size() - 2) == tval[1]) {
                        tmstat = tval[0];
                        tmsiz = tval[1];
                        tmdat = tval.data() - 2;
                        break;
                    }
                }
                else if ((tval.size() - 1) == tval[0]) {
                    tmstat = META_NONE;
                    tmsiz = tval[0];
                    tmdat = tval.data() - 1;
                    break;
                }
            }
            
            ttrk.emplace_back(tabst, tstat, midi_mval(tmstat, tmsiz, tmdat));
        }
        if (tstat >= STAT_SYSTEM_EXCLUSIVE) tstat = STAT_NONE;
    }

    //Assign MIDI messages
    this->amnt_mmsg[track_id] = ttrk.size();
    if (this->amnt_mmsg[track_id]) {
        this->mmsgs[track_id] = new midi_mesg[this->amnt_mmsg[track_id]] {};
        std::move(ttrk.begin(), ttrk.end(), this->mmsgs[track_id]);
    }
}

void midi::setMidi(unsigned char *in, unsigned length) {
    reset();

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

    if (!cmp_str(MIDI_MTHD, 4)) return;
    else if (get_int(4) != MIDI_MTHD_SIZE) return;
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

    this->amnt_mmsg = new unsigned[this->mhead.mthd_trks] {};
    for (int t = 0; t < this->mhead.mthd_trks; ++t) {
        unsigned tsize;
        if (cmp_str(MIDI_MTRK, 4) && (tsize = get_int(4)) < (in_end - in)) {
            setMidiRaw(in, tsize, t);
            in += tsize;
        }
        else return;
    }
}

void midi::setFormat(unsigned char frmt) {
    
}

/*
void midi::optimizeMidi() {
    if (!this->amnt_mmsg || !this->mmsgs) return;
    
    std::vector<std::vector<midi_mesg>> ttrks;
    std::vector<midi_mesg> glbl;
    
    ttrks.resize(this->mhead.mthd_trks);
    for (int t = 0; t < this->mhead.mthd_trks; ++t) {
        if (this->amnt_mmsg[t] && this->mmsgs[t]) {
            auto is_empty_or_system = [](const midi_mesg &msg) -> bool {
                bool isempty = msg.empty();
                bool issystm = (msg.mmsg_stat >= STAT_SYSTEM_EXCLUSIVE) && (msg.mmsg_stat < STAT_RESET);
                return isempty || issystm;
            };
            
            ttrks[t].assign(this->mmsgs[t], this->mmsgs[t] + this->amnt_mmsg[t]);
            delete[] this->mmsgs[t]; this->mmsgs[t] = 0;
            ttrks[t].erase(std::remove_if(ttrks[t].begin(), ttrks[t].end(), is_empty_or_system), ttrks[t].end());
            if (this->mhead.mthd_frmt == MIDI_MULTIPLE_TRACK) {
                std::copy_if(ttrks[t].begin(), ttrks[t].end(), std::back_inserter(glbl), is_global);
                ttrks[t].erase(std::remove_if(ttrks[t].begin(), ttrks[t].end(), is_global), ttrks[t].end());
            }
            std::sort(ttrks[t].begin(), ttrks[t].end());
        }
    }
    delete[] this->amnt_mmsg; this->amnt_mmsg = 0;
    
    if (this->mhead.mthd_frmt == MIDI_MULTIPLE_TRACK) {
        auto is_global = [](const midi_mesg &msg) -> bool {
            if (msg.mmsg_stat == STAT_RESET) {
                auto &meta = std::get<midi_mval>(msg.mmsg_data);
                switch(meta.dtyp) {
                    META_COPYRIGHT:
                    META_TRACK_NAME:
                    META_LYRICS:
                    META_MARKER:
                    META_CUE:
                    META_TEMPO:
                    META_SMPTE:
                        return true;
                    default:
                        return false;
                }
            }
            else return false;
        };
        auto is_global_track = [](std::vector<midi_mesg> &t) -> bool {
            return !t.empty() && std::all_of(t.begin(), t.end(), is_global);
        };
        auto itr_glbl = std::find_if(ttrks.begin(), ttrks.end(), is_global_track);
        
        if (itr_glbl != ttrks.end()) {
            glbl.assign(*itr_glbl.begin(), *itr_glbl.end());
            *itr_glbl.clear();
        }
        
        for (auto &t : ttrks) {
            auto has_global = [](std::vector<midi_mesg> &t) -> bool {
                int num_tnam = std::count_if(t.begin(), t.end(),
                                            [](const midi_mesg &msg)
                                            { });
            };
        }
    }
}
*/

std::string midi::getCsv() { return getCsv(0, 0); }
std::string midi::getCsv(unsigned char *mid_data, unsigned mid_size) {
    if (mid_size && mid_data) {
        setMidi(mid_data, mid_size);
        delete[] mid_data; mid_data = 0;
    }

    std::string csv = "";
    csv += setFstr("0, 0, HEADER, 1, {0}, {1}\n",
                   this->mhead.mthd_trks, this->mhead.mthd_divi);

    for (int t = 0; t < this->mhead.mthd_trks; ++t) {
        csv += setFstr("{0}, 0, START_TRACK\n", t + 1);

        for (int m = 0; m < this->amnt_mmsg[t]; ++m) {
            unsigned &ab = this->mmsgs[t][m].mmsg_time;
            unsigned char &st = this->mmsgs[t][m].mmsg_stat;
            auto &ms = this->mmsgs[t][m].mmsg_data;
            std::string txt;

            if (ms.index() == STAT_MESSAGE_NONE) continue;
            else if (ms.index() == STAT_MESSAGE_SINGLE) {
                unsigned char &v0 = std::get<1>(ms);
                
                if (st < STAT_PITCH_WHEEL) {
                    txt = (st < STAT_CHANNEL_PRESSURE) ? "PROGRAM_C" : "CHANNEL_AFTERTOUCH_C";
                    csv += setFstr("{0}, {1}, {2}, {3}, {4}\n", t + 1, ab, txt, st & 0x0F, v0);
                }
                continue;
            }
            else if (ms.index() == STAT_MESSAGE_DOUBLE) {
                unsigned short &v0 = std::get<2>(ms);
                
                if (st < STAT_SYSTEM_EXCLUSIVE) {
                    if (st < STAT_PITCH_WHEEL) {
                        txt = (st < STAT_NOTE_ON) ? "NOTE_OFF_C" :
                              (st < STAT_KEY_PRESSURE) ? "NOTE_ON_C" :
                              (st < STAT_CONTROLLER) ? "POLY_AFTERTOUCH_C" : "CONTROL_C";
                        csv += setFstr("{0}, {1}, {2}, {3}, {4}, {5}\n",
                                       t + 1, ab, txt, st & 0x0F, v0 >> 8, v0 & 0xFF);
                    }
                    else {
                        txt = "PITCH_BEND_C";
                        csv += setFstr("{0}, {1}, {2}, {3}, {4}\n",
                                       t + 1, ab, txt, st & 0x0F, v0);
                    }
                }
                continue;
            }
            else if (ms.index() == STAT_MESSAGE_VARIABLE) {
                midi_mval &v0 = std::get<3>(ms);
                unsigned &siz = v0.dsiz, v2 = 0;
                unsigned char *v1 = v0.data, &typ = v0.dtyp;
                
                if (st == STAT_RESET) {
                    if (typ == META_SEQUENCE_ID) {
                        txt = "SEQUENCE_NUMBER";
                        for (int i = 0; i < siz; ++i) v2 = (v2 << 8) | v1[i];
                        csv += setFstr("{0}, {1}, {2}, {3}\n", t + 1, ab, txt, v2);
                    }
                    else if (typ < 0x10) {
                        txt = (typ == META_COPYRIGHT) ? "COPYRIGHT_T" :
                              (typ == META_TRACK_NAME) ? "TITLE_T" :
                              (typ == META_INSTRUMENT_NAME) ? "INSTRUMENT_NAME_T" :
                              (typ == META_LYRICS) ? "LYRIC_T" :
                              (typ == META_MARKER) ? "MARKER_T" :
                              (typ == META_CUE) ? "CUE_POINT_T" : "TEXT_T";
                        csv += setFstr("{0}, {1}, {2}, \"{3}\"\n", t + 1, ab, txt, (const char*)v1);
                    }
                    else if (typ < META_CHANNEL_PREFIX) continue;
                    else if (typ < 0x22) {
                        txt = (typ == META_CHANNEL_PREFIX) ? "CHANNEL_PREFIX" : "MIDI_PORT";
                        csv += setFstr("{0}, {1}, {2}, {3}\n", t + 1, ab, txt, v1[0]);
                    }
                    else if (typ == META_END_OF_SEQUENCE) {
                        txt = "END_TRACK";
                        csv += setFstr("{0}, {1}, {2}\n", t + 1, ab, txt);
                        break;
                    }
                    else if (typ == META_TEMPO) {
                        txt = "TEMPO";
                        for (int i = 0; i < siz; ++i) v2 = (v2 << 8) | v1[i];
                        csv += setFstr("{0}, {1}, {2}, {3}\n", t + 1, ab, txt, v2);
                    }
                    else if (typ == META_SMPTE) {
                        txt = "SMPTE_OFFSET";
                        csv += setFstr("{0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}\n",
                                       t + 1, ab, txt, v1[0], v1[1], v1[2], v1[3], v1[4]);
                    }
                    else if (typ == META_TIME_SIGNATURE) {
                        txt = "TIME_SIGNATURE";
                        csv += setFstr("{0}, {1}, {2}, {3}, {4}, {5}, {6}\n",
                                       t + 1, ab, txt, v1[0], v1[1], v1[2], v1[3]);
                    }
                    else if (typ == META_KEY_SIGNATURE) {
                        txt = "KEY_SIGNATURE";
                        csv += setFstr("{0}, {1}, {2}, {3}, {4}\n",
                                       t + 1, ab, txt, v1[0], v1[1]);
                    }
                    else if (typ == META_KEY_SIGNATURE) {
                        txt = "SEQUENCER_SPECIFIC";
                        csv += setFstr("{0}, {1}, {2}, {3}", t + 1, ab, txt, siz);
                        for (int d = 0; d < siz; ++d) csv += setFstr(", {0}", v1[d]);
                        csv += "\n";
                    }
                    else if (typ == META_NONE) continue;
                    else {
                        txt = "UNKNOWN_META_EVENT";
                        csv += setFstr("{0}, {1}, {2}, {3}, {4}", t + 1, ab, txt, typ, siz);
                        for (int d = 0; d < siz; ++d) csv += setFstr(", {0}", v1[d]);
                        csv += "\n";
                    }
                }
                else {
                    txt = "SYSTEM_EXCLUSIVE";
                    if (st == STAT_SYSTEM_EXCLUSIVE_STOP) txt += "_PACKET";
                    csv += setFstr("{0}, {1}, {2}, {3}", t + 1, ab, txt, siz);
                    for (int d = 0; d < siz; ++d) csv += setFstr(", {0}", v1[d]);
                    csv += "\n";
                }
                continue;
            }
            
            //Literally only breaks from loop if end_of_track
            break;
        }
    }
    csv += "0, 0, END_OF_FILE\n";

    return csv;
}

void midi::getMidi(unsigned char *&mid_data, unsigned &mid_size) {
    if (mid_size && mid_data) setMidi(mid_data, mid_size);
    
    std::vector<unsigned char> out;
    auto set_arr = [](std::vector<unsigned char>tmp, const unsigned char *in1, int length) -> void {
        if (!in1 || length < 1) tmp.push_back(0);
        else tmp.insert(tmp.end(), in1, in1 + length);
    };
    auto set_str = [&](std::vector<unsigned char>tmp, const char *in1, int length) -> void {
        set_arr(tmp, (unsigned char*)in1, length);
    };
    auto set_int = [](std::vector<unsigned char>tmp, const unsigned int in, int length) -> void {
        while (length--) tmp.push_back((in >> (8 * length)) & 0xFF);
    };
    
    //Set MIDI header
    set_str(out, MIDI_MTHD, 4);
    set_int(out, MIDI_MTHD_SIZE, 4);
    set_int(out, this->mhead.mthd_frmt, 2);
    set_int(out, this->mhead.mthd_trks, 2);
    set_int(out, this->mhead.mthd_divi, 2);
    
    //Set MIDI tracks
    for (int t = 0; t < this->mhead.mthd_trks; ++t) {
        std::vector<unsigned char> trk;
        auto set_vlv = [&](unsigned int in) -> void {
            unsigned vlv = in & 0x7F;
            while (in >>= 7) { vlv = (vlv << 8) | ((in & 0x7F) | 0x80); }
            do { trk.push_back(vlv & 0xFF); vlv >>= 8; } while (trk.back() & 0x80);
        };
        auto set_bnd = [&](unsigned short in) -> void {
            unsigned short bend = in & 0x7F; in >>= 7; bend = (bend << 8) | (in & 0x7F);
            set_int(trk, bend, 2);
        };
        
        //Get MIDI track messages
        for (int m = 0, abs = 0; m < this->amnt_mmsg[t]; ++m) {
            midi_mesg &tmsg = this->mmsgs[t][m];
            unsigned &ab = tmsg.mmsg_time;
            unsigned char &st = tmsg.mmsg_stat;
            auto &ms = tmsg.mmsg_data;
            
            if (!ms.index());
            else if (ms.index() == STAT_MESSAGE_SINGLE) {
                unsigned char val[] {st, std::get<1>(ms)};
                set_vlv(ab - abs);
                set_arr(trk, val, 2);
            }
            else if (ms.index() == STAT_MESSAGE_DOUBLE) {
                unsigned short val = std::get<2>(ms);
                set_vlv(ab - abs);
                if ((st & 0xF0) == STAT_PITCH_WHEEL) set_bnd(val);
                else set_int(trk, val, 2);
            }
            else if (ms.index() == STAT_MESSAGE_VARIABLE) {
                midi_mval &val = std::get<3>(ms);
                set_vlv(ab - abs);
                set_int(trk, st, 1);
                if (st == STAT_RESET) set_int(trk, val.dtyp, 1);
                set_int(trk, val.dsiz, 1);
                set_arr(trk, val.data, val.dsiz);
                if (val.dtyp == META_END_OF_SEQUENCE) break;
            }
            abs = ab;
        }
        
        //Set MIDI track
        set_str(out, MIDI_MTRK, 4);
        set_int(out, trk.size(), 4);
        out.insert(out.end(), std::make_move_iterator(trk.begin()),
                              std::make_move_iterator(trk.end()));
    }

    //Set MIDI
    if (mid_data) { delete[] mid_data; mid_data = 0; }
    mid_data = new unsigned char[mid_size] {};
    std::move(out.begin(), out.end(), mid_data);
}
