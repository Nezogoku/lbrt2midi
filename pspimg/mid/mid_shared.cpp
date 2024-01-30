#include <algorithm>
#include <string>
#include <variant>
#include <vector>
#include "mid_types.hpp"
#include "mid_cc.hpp"
#include "mid_sysex.hpp"
#include "mid_meta.hpp"
#include "mid_shared.hpp"


template<typename... Args>
std::string midi::formatStr(const char *in, Args... args) {
    std::string stmp = "";
    std::variant<std::string, unsigned char*, unsigned int, int> ftmp[] = {args...};

    while (in[0]) {
        if (in[0] == '{') {
            int p = in[1] - '0';
            switch(ftmp[p].index()) {
                case 0:
                    stmp += std::get<0>(ftmp[p]);
                    break;
                case 1:
                    stmp += (const char*)(std::get<1>(ftmp[p]));
                    break;
                case 2:
                    stmp += std::to_string(std::get<2>(ftmp[p]));
                    break;
                case 3:
                    stmp += std::to_string(std::get<3>(ftmp[p]));
                    break;
                default:
                    stmp += *(in++);
                    continue;
            }
            in += 3;
        }
        else stmp += *(in++);
    }
    return stmp;
}

void midi::reset() {
    for (int t = 0; t < this->mhead.mthd_trks; ++t) {
        if (this->mmsgs[t]) delete[] this->mmsgs[t]; this->mmsgs[t] = 0;
    }
    if (this->mmsgs) delete[] this->mmsgs; this->mmsgs = 0;
    if (this->amnt_mmsg) delete[] this->amnt_mmsg; this->amnt_mmsg = 0;
    this->mhead = midi_mthd();
}

bool midi::cmpStr(unsigned char *&in0, const char *in1, int length) {
    while ((*(in0++) == (unsigned char)(*(in1++))) && --length);
    return !length;
}

void midi::setStr(unsigned char *&out, const char *in) { while (in[0]) *(out++) = *(in++); }
void midi::setStr(unsigned char *&out, const char *in, int length) { setStr(out, (unsigned char*)in, length); }
void midi::setStr(unsigned char *&out, unsigned char *in, int length) {
    int s = 0;
    while (length && s < length) *(out++) = in[s++];
}

void midi::setInt(unsigned char *&out, const unsigned int in, int length) {
    for (int i = 0; i < length; ++i) out[(length - 1) - i] = in >> (8 * i);
    out += length;
}

unsigned midi::getInt(unsigned char *&in, int length) {
    unsigned out = 0;
    for (int i = 0; i < length; ++i) out = (out << 8) | *(in++);
    return out;
}

void midi::setVLV(unsigned char *&out, unsigned int in) {
    unsigned vlv = in & 0x7F;
    while (in >>= 7) { vlv = (vlv << 8) | ((in & 0x7F) | 0x80); }
    while ((out[0] = vlv & 0xFF) && (*(out++) & 0x80)) { vlv >>= 8; }
}

unsigned midi::getVLV(unsigned char *&in) {
    unsigned out = 0;
    while ((out |= in[0] & 0x7F) && (*(in++) & 0x80)) { out <<= 7; }
    return out;
}

unsigned midi::getLengthVLV(unsigned in) {
    unsigned char *buffer = new unsigned char[4] {};
    unsigned char *buffer_start = buffer;
    setVLV(buffer, in);
    unsigned out = buffer - buffer_start;
    buffer = buffer_start;
    return out;
}

void midi::setPitchBend(unsigned char *&out, unsigned short in) {
    unsigned short bend = in & 0x7F; in >>= 7; bend = (bend << 8) | (in & 0x7F);
    while ((*(out++) = bend & 0xFF) && (bend)) { bend >>= 8; }
}

unsigned short midi::getPitchBend(unsigned char *&in) {
    unsigned short out = *(in++) & 0x7F; out = (out << 7) | (*(in++) & 0x7F);
    return out;
}
unsigned short midi::getPitchBend(unsigned short in) {
    unsigned char *ain = new unsigned char[2] {in & 0xFF, in >> 8};
    return getPitchBend(ain);
}


void midi::setMidiRaw(unsigned char *in, unsigned length, int track_id) {
    const unsigned char *in_end = in + length;
    unsigned tabst = 0, tstat = STAT_NONE;
    std::vector<midi_mesg> ttrk;
    unsigned char *tmdat = 0, tmstat = META_NONE, tmsiz = 0;
    
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
        tabst += getVLV(in);
        std::vector<unsigned char> tval;

        if (!(in[0] & 0x80) && (tstat == STAT_NONE)) continue;
        else if (in[0] & 0x80) tstat = *(in++);
        
        switch (get_mesg_size()) {
            case STAT_MESSAGE_NONE:
                ttrk.emplace_back({tabst, tstat});
                break;
                
            case STAT_MESSAGE_SINGLE:
                ttrk.emplace_back({tabst, tstat, *(in++)});
                break;
                
            case STAT_MESSAGE_DOUBLE:
                while (tval.size() < 2) tval.push_back(*(in++));
                ttrk.emplace_back({tabst, tstat, {tval[0], tval[1]}});
                break;
                
            case STAT_MESSAGE_VARIABLE:
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
                
                ttrk.emplace_back({tabst, tstat, {tmstat, tmsiz, tmdat}});
                break;
        }
        if (tstat >= STAT_SYSTEM_EXCLUSIVE) tstat = STAT_NONE;
    }

    //Assign MIDI messages
    this->amnt_mmsg[track_id] = ttrk.size();
    if (this->amnt_mmsg[track_id]) {
        this->mmsgs[track_id] = new midi_mesg[this->amnt_mmsg[track_id]] {};
        for (int t = 0; t < this->amnt_mmsg[track_id]; ++t) this->mmsgs[track_id][t] = ttrk[t];
    }
    ttrk.clear();
}

void midi::setMidi(unsigned char *in, unsigned length) {
    reset();

    const unsigned char *in_end = in + length;

    if (!cmpStr(in, MIDI_MTHD, 4)) return;
    else if (getInt(in, 4) != MIDI_MTHD_SIZE) return;
    this->mhead.mthd_frmt = getInt(in, 4);
    this->mhead.mthd_trks = getInt(in, 2);
    this->mhead.mthd_divi = getInt(in, 2);
    
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
        if (cmpStr(in, MIDI_MTRK, 4) && (tsize = getInt(in, 4)) < (in_end - in)) {
            setMidiRaw(in, tsize, t);
            in += tsize;
        }
        else return;
    }
}

void midi::setFormat(unsigned char frmt) {
    
}

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
                
            };
        }
    }
}

std::string midi::getCsv() { return getCsv(0, 0); }
std::string midi::getCsv(unsigned mid_size, unsigned char *mid_data) {
    if (mid_size && mid_data) setMidi(mid_data, mid_size);

    std::string csv = "";
    csv += formatStr("0, 0, HEADER, 1, {0}, {1}\n",
                     this->mhead.mthd_trks, this->mhead.mthd_divi);

    for (int t = 0; t < this->mhead.mthd_trks; ++t) {
        csv += formatStr("{0}, 0, START_TRACK\n", t + 1);

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
                    csv += formatStr("{0}, {1}, {2}, {3}, {4}\n", t + 1, ab, txt, st & 0x0F, v0);
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
                        csv += formatStr("{0}, {1}, {2}, {3}, {4}, {5}\n",
                                         t + 1, ab, txt, st & 0x0F, v0 >> 8, v0 & 0xFF);
                    }
                    else {
                        txt = "PITCH_BEND_C";
                        csv += formatStr("{0}, {1}, {2}, {3}, {4}\n",
                                         t + 1, ab, txt, st & 0x0F, v0);
                    }
                }
                continue;
            }
            else if (ms.index() == STAT_MESSAGE_VARIABLE) {
                midi_mval &v0 = std::get<3>(ms);
                unsigned char *v1 = v0.data, &typ = v0.dtyp, &siz = v0.dsiz;
                
                if (st == STAT_RESET) {
                    if (typ == META_SEQUENCE_ID) {
                        txt = "SEQUENCE_NUMBER";
                        csv += formatStr("{0}, {1}, {2}, {3}\n", t + 1, ab, txt, getInt(v1, 2));
                    }
                    else if (typ < 0x10) {
                        txt = (typ == META_COPYRIGHT) ? "COPYRIGHT_T" :
                              (typ == META_TRACK_NAME) ? "TITLE_T" :
                              (typ == META_INSTRUMENT_NAME) ? "INSTRUMENT_NAME_T" :
                              (typ == META_LYRICS) ? "LYRIC_T" :
                              (typ == META_MARKER) ? "MARKER_T" :
                              (typ == META_CUE) ? "CUE_POINT_T" : "TEXT_T";
                        csv += formatStr("{0}, {1}, {2}, \"{3}\"\n", t + 1, ab, txt, (const char*)v1);
                    }
                    else if (typ < META_CHANNEL_PREFIX) continue;
                    else if (typ < 0x22) {
                        txt = (typ == META_CHANNEL_PREFIX) ? "CHANNEL_PREFIX" : "MIDI_PORT";
                        csv += formatStr("{0}, {1}, {2}, {3}\n", t + 1, ab, txt, v1[0]);
                    }
                    else if (typ == META_END_OF_SEQUENCE) {
                        txt = "END_TRACK";
                        csv += formatStr("{0}, {1}, {2}\n", t + 1, ab, txt);
                        break;
                    }
                    else if (typ == META_TEMPO) {
                        txt = "TEMPO";
                        csv += formatStr("{0}, {1}, {2}, {3}\n", t + 1, ab, txt, getInt(v1, 3));
                    }
                    else if (typ == META_SMPTE) {
                        txt = "SMPTE_OFFSET";
                        csv += formatStr("{0}, {1}, {2}, {3}, {4}, {5}, {6}, {7}\n",
                                         t + 1, ab, txt,
                                         v1[0], v1[1], v1[2], v1[3], v1[4]);
                    }
                    else if (typ == META_TIME_SIGNATURE) {
                        txt = "TIME_SIGNATURE";
                        csv += formatStr("{0}, {1}, {2}, {3}, {4}, {5}, {6}\n",
                                         t + 1, ab, txt,
                                         v1[0], v1[1], v1[2], v1[3]);
                    }
                    else if (typ == META_KEY_SIGNATURE) {
                        txt = "KEY_SIGNATURE";
                        csv += formatStr("{0}, {1}, {2}, {3}, {4}\n",
                                         t + 1, ab, txt, v1[0], v1[1]);
                    }
                    else if (typ == META_KEY_SIGNATURE) {
                        txt = "SEQUENCER_SPECIFIC";
                        csv += formatStr("{0}, {1}, {2}, {3}", t + 1, ab, txt, siz);
                        for (int d = 0; d < siz; ++d) csv += formatStr(", {0}", v1[d]);
                        csv += "\n";
                    }
                    else if (typ == META_NONE) continue;
                    else {
                        txt = "UNKNOWN_META_EVENT";
                        csv += formatStr("{0}, {1}, {2}, {3}, {4}", t + 1, ab, txt, typ, siz);
                        for (int d = 0; d < siz; ++d) csv += formatStr(", {0}", v1[d]);
                        csv += "\n";
                    }
                }
                else {
                    txt = "SYSTEM_EXCLUSIVE";
                    if (st == STAT_SYSTEM_EXCLUSIVE_STOP) txt += "_PACKET";
                    csv += formatStr("{0}, {1}, {2}, {3}", t + 1, ab, txt, siz);
                    for (int d = 0; d < siz; ++d) csv += formatStr(", {0}", v1[d]);
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

void midi::getMidi(unsigned &mid_size, unsigned char *&mid_data, unsigned char frmt) {
    if (mid_size && mid_data) setMidi(mid_data, mid_size);

    if ((frmt == MIDI_SINGLE_TRACK) && (amnt_trks > 1)) frmt = MIDI_MULTIPLE_TRACK;

    if ((timecode == TIME_PPQN) && (division < 0)) /* toPPQN() */ return;
    else if ((timecode == TIME_MTC) && (division > 0)) /* toMTC() */ return;


    //Set midi header
    midi_mthd fhed(frmt, this->amnt_trks, this->division);

    //Set track headers
    midi_mtrk theds[amnt_trks] {};
    for (int t = 0; t < amnt_trks; ++t) {
        theds[t].mtrk_size = getTrckSize(t);
    }

    //Set midi
    mid_size = 14;
    for (auto &hed : theds) mid_size += (8 + hed.mtrk_size);
    if (mid_data) { delete[] mid_data; mid_data = 0; }
    mid_data = new unsigned char[mid_size] {};

    unsigned char *midi_strt = mid_data;
    setStr(mid_data, MIDI_MTHD, 4);
    setInt(mid_data, MIDI_MTHD_SIZE, 4);
    setInt(mid_data, fhed.mthd_frmt, 2);
    setInt(mid_data, fhed.mthd_trks, 2);
    setInt(mid_data, fhed.mthd_divi, 2);

    for (unsigned t = 0; t < amnt_trks; ++t) {
        setStr(mid_data, MIDI_MTRK, 4);
        setInt(mid_data, theds[t].mtrk_size, 4);

        for (unsigned m = 0, a = 0; m < this->trcks[t].amnt_mmsg; ++m) {
            midi_mesg &tmsg = this->trcks[t].mmsgs[m];
            unsigned &ab = tmsg.mmsg_time;
            unsigned char &st = tmsg.mmsg_stat;
            auto &ms = tmsg.mmsg_data;

            if (tmsg.statIsValid()) {
                if (ms.index() == STAT_MESSAGE_SINGLE) {
                    setVLV(mid_data, ab - a);
                    *(mid_data++) = st;
                    *(mid_data++) = std::get<1>(ms);
                }
                else if (ms.index() == STAT_MESSAGE_DOUBLE) {
                    setVLV(mid_data, ab - a);
                    *(mid_data++) = st;
                    if ((st & 0xF0) == STAT_PITCH_WHEEL) {
                        setPitchBend(mid_data, std::get<2>(ms));
                    }
                    else setInt(mid_data, std::get<2>(ms), 2);
                }
                else {
                    midi_mval &val = std::get<3>(ms);
                    if (!(val.dtyp & 0x80)) {
                        setVLV(mid_data, ab - a);
                        *(mid_data++) = st;
                        *(mid_data++) = val.dtyp;
                        if (st == STAT_RESET) *(mid_data++) = val.dsiz;
                        setStr(mid_data, val.data, val.dsiz);
                        if (st == STAT_SYSTEM_EXCLUSIVE) *(mid_data++) = STAT_SYSTEM_EXCLUSIVE_STOP;

                        if (val.dtyp == META_END_OF_SEQUENCE) break;
                    }
                    else continue;
                }
            }

            a = ab;
        }
    }

    mid_data = midi_strt;
}
