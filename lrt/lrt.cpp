#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>
#include "directory.hpp"
#define PACKCSV_IMPLEMENTATION
#include "midi/midi_func.hpp"
#include "midi/midi_const.hpp"
#include "midi/midi_types.hpp"
#include "lrt_func.hpp"
#include "lrt_forms.hpp"
#include "lrt_const.hpp"
#include "lrt_types.hpp"


///Unpacks LBRT info from LBRT file
void unpackLrt(const char *file) {
    if (!file || !file[0]) return;

    std::string str;
    int p0, p1;

    str = file;
    p0 = str.find_last_of("\\/"); if (p0 == std::string::npos) p0 = 0;
    p1 = str.find_last_of('.'); if (p1 == std::string::npos || p1 <= p0) p1 = str.size();
    if (lrt_debug) fprintf(stderr, "Title of sequence: %s\n", std::string(file + p0 + (p0 > 0), file + p1).c_str());

    if (file) {
        unsigned char *data = 0;
        unsigned size = 0;

        if (!getFileData(file, data, size)) {
            fprintf(stderr, "    Unable to open %s\n", file);
            return;
        }
        unpackLrt(data, size);
    }
    lrt_inf.path = (!p0) ? "" : std::string(file, file + p0);
    lrt_inf.name = std::string(file + p0 + (p0 > 0), file + p1);
}

///Unpacks LBRT info from LBRT data
void unpackLrt(unsigned char *in, const unsigned length) {
    lrt_inf = {};
    if (!in || length < 36) return;

    const unsigned char *in_beg = in, *in_end = in + length;

    auto get_fcc = [&in]() -> unsigned {
        unsigned out = 0;
        for (int i = 0; i < 4; ++i) {
            out = (out << 8) | *(in++);
        }
        return out;
    };
    auto get_int = [&in](unsigned length) -> signed {
        signed out = 0;
        for (int i = length; i > 0; --i) {
            out = (out << 8) | in[i - 1];
        }
        in += length;
        return out;
    };

    if (get_fcc() != FOURCC_LBRT) {
        fprintf(stderr, "This is not an LBRT file\n");
        lrt_inf = {}; return;
    }
    lrt_inf.soff = get_int(4);
    lrt_inf.tpc = get_int(4);
    lrt_inf.ppqn = get_int(4);
    
    try {
        lrt_inf.trks.resize(get_int(4));
        if (lrt_inf.trks.empty()) throw std::exception();
        
        if (lrt_debug) {
            fprintf(stderr, "    Sequence data at position: 0x%08X\n", lrt_inf.soff);
            fprintf(stderr, "    Clock ticks per click: %u\n", lrt_inf.tpc);
            fprintf(stderr, "    Pulses per quarter note: %u\n", lrt_inf.ppqn);
            fprintf(stderr, "    Total tracks: %u\n", lrt_inf.trks.size());
        }
        
        for (auto &trk : lrt_inf.trks) {
            trk.id = get_int(4);
            trk.unk0 = get_int(4);
            trk.msgs.resize(get_int(4));
            trk.qrts.resize(get_int(4));
            if (trk.msgs.empty() || trk.qrts.empty()) throw std::exception();
            
            if (lrt_debug) {
                fprintf(stderr, "        Sequence id: %u\n", trk.id);
                fprintf(stderr, "        Total events: %u\n", trk.msgs.size());
                fprintf(stderr, "        Total quarter events: %u\n", trk.qrts.size());
            }
            
            //Set quarter events
            for (auto &q : trk.qrts) {
                q = get_int(4);
                if (lrt_debug) fprintf(stderr, "            Quarter event ID: %04u\n", q);
            }
        }
    }
    catch (std::exception &e) {
        fprintf(stderr, "There are no events in this LBRT file\n");
        lrt_inf = {}; return;
    }

    //Set messages
    if (lrt_debug) fprintf(stderr, "    Retrieving LBRT messages\n");

    in = (unsigned char*)in_beg + lrt_inf.soff;
    for (auto &trk : lrt_inf.trks) {
        for (auto &msg : trk.msgs) {
            msg.id = get_int(4);
            msg.dtim = get_int(4);
            msg.tval = get_int(4);
            msg.val0 = get_int(4);
            msg.val1 = get_int(2);
            msg.val2 = get_int(2);
            msg.velon = get_int(2);
            msg.bndon = get_int(2);
            msg.chn = get_int(1);
            msg.stat = get_int(1);
            msg.veloff = get_int(1);
            msg.bndoff = get_int(1);

            if (lrt_debug) {
                fprintf(stderr, "        Current event: %u\n", &msg - trk.msgs.data());
                fprintf(stderr, "            Quarter Event ID: %u\n", msg.id);
                fprintf(stderr, "            Delta time: %ums\n", msg.dtim);
                fprintf(stderr, "            Time value: %u\n", msg.tval);
                fprintf(stderr, "            Channel event value 1: %u\n", msg.val0);
                fprintf(stderr, "            Channel event value 2: %u\n", msg.val1);
                fprintf(stderr, "            Channel event value 3: %u\n", msg.val2);
                fprintf(stderr, "            Note On velocity: %u\n", msg.velon);
                fprintf(stderr, "            Note On pitch bend: %u\n", msg.bndon);
                fprintf(stderr, "            Channel: %u\n", msg.chn);
                fprintf(stderr, "            Status: 0x%02X\n", msg.stat & 0xFF);
                fprintf(stderr, "            Note Off velocity: %u\n", msg.veloff);
                fprintf(stderr, "            Note Off pitch bend: %u\n", msg.bndoff);
            }
        }
    }
}


///Extracts MIDI from LBRT info
void extractLrt(const char *folder) {
    if (lrt_inf == lbrtinfo{}) return;
    if (folder && folder[0]) lrt_inf.path = folder;
    if (lrt_inf.path.find_last_of("\\/") != lrt_inf.path.size() - 1) lrt_inf.path += "/";

    fprintf(stderr, "Extract from LBRT file to %s\n", lrt_inf.path.c_str());
    if (!createFolder(lrt_inf.path.c_str())) {
        fprintf(stderr, "Unable to create %s\n", lrt_inf.path.c_str());
        lrt_inf = {}; return;
    }

    for (const auto &trk : lrt_inf.trks) {
        midiinfo tmp;
        std::string nam = lrt_inf.name;
        if (lrt_inf.trks.size() > 1) nam += "_" + std::to_string(&trk - lrt_inf.trks.data());
        
        /* //For tracking bend values
        unsigned short bends[16] {};
        for (auto &b : bends) b = 0x40;
        */
        
        //Assign MIDI header
        if (lrt_debug) fprintf(stderr, "    Assign MIDI header\n");
        tmp.fmt = MIDI_MULTIPLE_TRACK;
        tmp.trk = 1;
        tmp.setDivision(lrt_inf.ppqn);
        tmp.msg.resize(17);
        
        //Insert global settings
        if (lrt_debug) fprintf(stderr, "    Set MIDI tracks\n");
        tmp.msg[0].emplace_back(0, META_TRACK_NAME, nam.c_str());
        tmp.msg[0].emplace_back(0, META_TIME_SIGNATURE, (unsigned char[]){4, 2, lrt_inf.tpc, 8});
        
        //Set messages
        for (int e = 1, abs = 0, fabs = 0; e < trk.msgs.size(); ++e) {
            auto chn = trk.msgs[e].chn;
            short stat = trk.msgs[e].stat;

            if (stat != STAT_RESET) stat = stat & 0xF0;
            else { stat = (stat << 8) | trk.msgs[e].val2; chn = -1; }
            
            abs += trk.msgs[e].dtim;
            if (stat != STAT_NONE) fabs = abs + trk.msgs[e].tval;

            if (stat == STAT_NONE) {
                //Theoretically shouldn't do anything
                if (lrt_debug) fprintf(stderr, "        RUNNING STATUS\n");
            }
            else if (stat == STAT_NOTE_OFF) {
                //NOTE_OFF message found with NOTE_ON
            }
            else if (stat == STAT_NOTE_ON) {
                if (lrt_debug) fprintf(stderr, "        NOTE ON/OFF EVENT\n");
                    if (tmp.msg[chn+1].empty()) {
                    tmp.msg[chn+1].emplace_back(0, STAT_PROGRAMME_CHANGE | chn, (unsigned char[]){chn});
                }
                
                tmp.msg[chn+1].emplace_back(abs, STAT_NOTE_ON | chn, (unsigned char[]){trk.msgs[e].val0,trk.msgs[e].velon});
                tmp.msg[chn+1].emplace_back(fabs, STAT_NOTE_OFF | chn, (unsigned char[]){trk.msgs[e].val0,trk.msgs[e].veloff});

                /* Either these are NOT pitch bend values or the bending range is different... ugh
                   OR these relate to portamento somehow... double UGH
                if (bends[chn] != trk.msgs[e].bndon) {
                    bends[chn] = trk.msgs[e].bndon;
                    tmp.msg[chn+1].push_back({abs, STAT_PITCH_WHEEL | chn, {bends[chn],bends[chn]>>8}});
                }
                if (bends[chn] != trk.msgs[e].bndoff) {
                    bends[chn] = trk.msgs[e].bndoff;
                    tmp.msg[chn+1].push_back({fabs, STAT_PITCH_WHEEL | chn, {bends[chn],bends[chn]>>8}});
                }
                */
            }
            else if (stat == STAT_KEY_PRESSURE) {
                //Never seen KEY PRESSURE, likely never will
                if (lrt_debug) fprintf(stderr, "        KEY PRESSURE\n");
            }
            else if (stat == STAT_CONTROLLER) {
                //Sequence uses CC 99 for looping
                //Will use CC 116/117 instead
                //Additionally use Final Fantasy style just because
                if (trk.msgs[e].val2 == LBRT_PSX_LOOP) {
                    if (trk.msgs[e].val0 == LBRT_PSX_LOOPSTART) {
                        if (lrt_debug) fprintf(stderr, "        LOOP START EVENT\n");
                        tmp.msg[0].emplace_back(abs, STAT_CONTROLLER, (unsigned char[]){CC_XML_LOOPSTART, CC_XML_LOOPINFINITE});
                        tmp.msg[0].emplace_back(abs, META_MARKER, "loopStart");
                    }
                    else if (trk.msgs[e].val0 == LBRT_PSX_LOOPEND) {
                        if (lrt_debug) fprintf(stderr, "        LOOP STOP EVENT\n");
                        tmp.msg[0].emplace_back(abs, STAT_CONTROLLER, (unsigned char[]){CC_XML_LOOPEND, CC_XML_LOOPRESERVED});
                        tmp.msg[0].emplace_back(abs, META_MARKER, "loopEnd");
                    }
                }
                else {
                    if (lrt_debug) fprintf(stderr, "        CC %u EVENT\n", trk.msgs[e].val2);
                    tmp.msg[chn+1].emplace_back(abs, stat, (unsigned char[]){trk.msgs[e].val2, trk.msgs[e].val0});
                }
            }
            else if (stat == STAT_PROGRAMME_CHANGE) {
                //Never seen PROGRAMME CHANGE, likely never will
                if (lrt_debug) fprintf(stderr, "        PROGRAMME CHANGE\n");
            }
            else if (stat == STAT_CHANNEL_PRESSURE) {
                //Never seen CHANNEL PRESSURE, likely never will
                if (lrt_debug) fprintf(stderr, "        CHANNEL PRESSURE\n");
            }
            else if (stat == STAT_PITCH_WHEEL) {
                //Never seen PITCH WHEEL message, likely found with NOTE_ON
                if (lrt_debug) fprintf(stderr, "        PITCH WHEEL\n");
            }
            else if (stat == META_END_OF_SEQUENCE) {
                if (lrt_debug) fprintf(stderr, "        END OF TRACK\n");
                for (auto &tr : tmp.msg) {
                    if (!tr.empty()) tr.emplace_back(fabs, META_END_OF_SEQUENCE);
                }
                break;
            }
            else if (stat == META_TEMPO) {
                if (lrt_debug) fprintf(stderr, "        TEMPO CHANGE TO %g BPM\n", 60000000.0 / trk.msgs[e].tval);
                tmp.msg[0].emplace_back(
                    abs,
                    META_TEMPO,
                    (unsigned char[]){trk.msgs[e].tval>>16, trk.msgs[e].tval>>8, trk.msgs[e].tval>>0}
                );
            }
        }
        
        //Remove empty tracks
        std::erase_if(tmp.msg, [](const auto &t) { return t.empty(); });

        //Sort tracks
        if (lrt_debug) fprintf(stderr, "    Sort MIDI tracks\n");
        for (auto &trk : tmp.msg) std::sort(trk.begin(), trk.end());

        //Update number tracks
        tmp.trk = tmp.msg.size();

        //Write MIDI to file
        if (true) {
            if (lrt_debug) fprintf(stderr, "    Write MIDI file\n");
            auto out = packMidi(tmp);

            if (!createFile((lrt_inf.path + nam + ".mid").c_str(), out.data(), out.size())) {
                fprintf(stderr, "    Unable to extract %s.mid\n", nam.c_str());
                continue;
            }
        }

        //Write MIDI to CSV if applicable
        if (lrt_midicsv) {
            if (lrt_debug) fprintf(stderr, "    Write MIDICSV file\n");
            auto out = packCsv(tmp);

            if (!createFile((lrt_inf.path + nam + ".csv").c_str(), out.data(), out.size())) {
                fprintf(stderr, "    Unable to extract %s.csv\n", nam.c_str());
            }
        }
    }
    
    lrt_inf = {};
}
