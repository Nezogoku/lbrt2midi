#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>
#include "playmidi.hpp"
#include "directory.hpp"
#include "mid_types.hpp"
#include "mid_cc.hpp"
#include "mid_shared.hpp"
#include "lrt_types.hpp"
#include "lrt.hpp"


int lbrt::setLRT(std::string lrt_file) {
    this->lrt_path = lrt_file.substr(0, lrt_file.find_last_of("\\/") + 1);
    this->seq_name = lrt_file.substr(lrt_file.find_last_of("\\/") + 1);
    this->seq_name = this->seq_name.substr(0, this->seq_name.find_last_of('.'));
    if (this->debug) fprintf(stderr, "    Title of sequence: %s\n", this->seq_name.c_str());

    unsigned char *lrt_data = 0, *lrt_start = 0;
    unsigned lrt_size = 0, num_chan = 0;

    auto cmp_str = [&](const char *in1, int length) -> bool {
        while ((*(lrt_data++) == (unsigned char)(*(in1++))) && --length);
        return !length;
    };
    auto get_int = [&](int length) -> unsigned {
        unsigned out = 0;
        for (int i = 0; i < length; ++i)
            { out |= (unsigned)*(lrt_data++) << (8 * i); }
        return out;
    };
    
    
    if (!getFileData(lrt_file.c_str(), lrt_start, lrt_size)) {
        fprintf(stderr, "    Unable to open %s\n", this->seq_name.c_str());
        return 0;
    }
    else lrt_data = lrt_start;

    lbrt_head hedr;

    if (!cmp_str("LBRT", 4)) {
        fprintf(stderr, "    This is not an LBRT file\n");
        return 0;
    }
    hedr.soff = get_int(4);
    hedr.npqn = get_int(4);
    hedr.ppqn = get_int(4);
    hedr.trks = get_int(4);
    hedr.frmt = get_int(4);
    hedr.val0 = get_int(4);
    hedr.msgs = get_int(4);
    hedr.qrts = get_int(4);

    if ((signed)hedr.qrts <= 0) {
        fprintf(stderr, "    There are no quarter events in this LBRT file\n");
        return 0;
    }

    unsigned list_qrts[hedr.qrts] {};

    if (this->debug) {
        fprintf(stderr, "    Sequence data at position: 0x%08X\n", hedr.soff);
        fprintf(stderr, "    Notated 32nd-notes per quarter note: %u\n", hedr.npqn);
        fprintf(stderr, "    Pulses per quarter note: %u\n", hedr.ppqn);
        fprintf(stderr, "    Total tracks: %u\n", hedr.trks);
        fprintf(stderr, "    Sequence format: %u\n", hedr.frmt);
        fprintf(stderr, "    Total events: %u\n", hedr.msgs);
        fprintf(stderr, "    Total quarter events: %u\n", hedr.qrts);
    }

    for (int i = 0; i < hedr.qrts; ++i) {
        list_qrts[i] = get_int(4);
        if (this->debug) fprintf(stderr, "        Quarter event ID %04u: %04u\n", i, list_qrts[i]);
    }


    //Set sequence events
    if (this->debug) fprintf(stderr, "\n    Retrieving LBRT sequences\n");
    lbrt_mesg mesgs[hedr.msgs] = {};

    lrt_data = lrt_start + hedr.soff;
    for (int e = 0; e < hedr.msgs; ++e) {
        mesgs[e].id = get_int(4);
        mesgs[e].dtim = get_int(4);
        mesgs[e].tval = get_int(4);
        mesgs[e].eval0 = get_int(4);
        mesgs[e].eval1 = get_int(2);
        mesgs[e].cc = get_int(2);
        mesgs[e].velon = get_int(2);
        mesgs[e].bndon = get_int(2);
        mesgs[e].chn = get_int(1);
        mesgs[e].stat = get_int(1);
        mesgs[e].veloff = get_int(1);
        mesgs[e].bndoff = get_int(1);

        if (this->debug) {
            fprintf(stderr, "        Current event: %u\n", e);
            fprintf(stderr, "            Quarter Event ID: %u\n", mesgs[e].id);
            fprintf(stderr, "            Delta time: %ums\n", mesgs[e].dtim);
            fprintf(stderr, "            Time value: %u\n", mesgs[e].tval);
            fprintf(stderr, "            Channel event value 1: %u\n", mesgs[e].eval0);
            fprintf(stderr, "            Channel event value 2: %u\n", mesgs[e].eval1);
            fprintf(stderr, "            Channel Controller value: %u\n", mesgs[e].cc);
            fprintf(stderr, "            Note On velocity: %u\n", mesgs[e].velon);
            fprintf(stderr, "            Note On pitch bend: %u\n", mesgs[e].bndon);
            fprintf(stderr, "            Channel: %u\n", mesgs[e].chn);
            fprintf(stderr, "            Status: 0x%02X\n", mesgs[e].stat);
            fprintf(stderr, "            Note Off velocity: %u\n", mesgs[e].veloff);
            fprintf(stderr, "            Note Off pitch bend: %u\n", mesgs[e].bndoff);
        }

        if (mesgs[e].stat != STAT_RESET && mesgs[e].chn > num_chan) {
            num_chan = mesgs[e].chn;
        }
    }
    num_chan += 1;

    //For tracking bend values
    unsigned short bends[num_chan] {};
    for (auto &b : bends) b = 0x40;

    //Assign MIDI tracks
    if (!this->mmsgs.empty()) this->mmsgs.clear();
    std::vector<std::vector<midi_mesg>> tmsgs(num_chan + 1);
    
    //Assign MIDI header
    if (this->debug) fprintf(stderr, "\n    Assign MIDI header\n");
    this->mhead.mthd_frmt = 1;
    this->mhead.mthd_trks = num_chan + 1;
    this->mhead.mthd_divi = hedr.ppqn & 0x7FFF;

    //Insert global settings
    if (this->debug) fprintf(stderr, "\n    Set MIDI sequences\n");
    tmsgs[0].push_back({0, META_TRACK_NAME, this->seq_name.length(), this->seq_name.c_str()});
    tmsgs[0].push_back({0, META_TIME_SIGNATURE, {4, 2, hedr.npqn, 8}});
    
    //Set messages
    //Additionally, skip initial tempo event
    for (int e = 0, abs = 0, fabs = 0; e < hedr.msgs; ++e) {
        unsigned char &chn = mesgs[e].chn;
        unsigned short stat = mesgs[e].stat;
        if (stat == 0x00FF) stat = (stat << 8) | mesgs[e].cc;
        else stat = stat & 0xFFF0;
        
        abs += mesgs[e].dtim;
        if ((short)stat > 0) fabs = abs + mesgs[e].tval;
        
        if (stat == STAT_NONE) {
            //Theoretically shouldn't do anything
            if (this->debug) fprintf(stderr, "        RUNNING STATUS\n");
        }
        else if (stat == STAT_NOTE_OFF) {
            //NOTE_OFF message found with NOTE_ON
        }
        else if (stat == STAT_NOTE_ON) {
            if (this->debug) fprintf(stderr, "        NOTE ON/OFF EVENT\n");
            
            tmsgs[chn+1].push_back({abs, STAT_NOTE_ON | chn, {mesgs[e].eval0,mesgs[e].velon}});
            tmsgs[chn+1].push_back({fabs, STAT_NOTE_OFF | chn, {mesgs[e].eval0, mesgs[e].veloff}});

            /* Either these are NOT pitch bend values or the bending range is different... ugh
            if (bends[chn] != mesgs[e].bndon) {
                bends[chn] = mesgs[e].bndon;
                tmsgs[chn+1].push_back({abs, STAT_PITCH_WHEEL | chn, {bends[chn],bends[chn]>>8}});
            }
            if (bends[chn] != mesgs[e].bndoff) {
                bends[chn] = mesgs[e].bndoff;
                tmsgs[chn+1].push_back({fabs, STAT_PITCH_WHEEL | chn, {bends[chn],bends[chn]>>8}});
            }
            */
        }
        else if (stat == STAT_KEY_PRESSURE) {
            //Never seen KEY PRESSURE, likely never will
            if (this->debug) fprintf(stderr, "        KEY PRESSURE\n");
        }
        else if (stat == STAT_CONTROLLER) {
            //Sequence uses CC 99 for looping
            //Will use CC 116/117 instead
            //Additionally use Final Fantasy style just because
            if (mesgs[e].cc == CC_PSX_LOOP) {
                if (mesgs[e].eval0 == CC_PSX_LOOPSTART) {
                    if (this->debug) fprintf(stderr, "        LOOP START EVENT\n");
                    tmsgs[chn+1].push_back({abs, STAT_CONTROLLER | chn, {CC_XML_LOOPSTART, CC_XML_LOOPINFINITE}});
                    tmsgs[0].push_back({abs, META_MARKER, {'l','o','o','p','S','t','a','r','t'}});
                }
                else if (mesgs[e].eval0 == CC_PSX_LOOPEND) {
                    if (this->debug) fprintf(stderr, "        LOOP STOP EVENT\n");
                    tmsgs[chn+1].push_back({abs, STAT_CONTROLLER | chn, {CC_XML_LOOPEND, CC_XML_LOOPRESERVED}});
                    tmsgs[0].push_back({abs, META_MARKER, {'l','o','o','p','E','n','d'}});
                }
            }
            else {
                if (this->debug) fprintf(stderr, "        CC %u EVENT\n", mesgs[e].cc);
                tmsgs[chn+1].push_back({abs, stat, {mesgs[e].cc, mesgs[e].eval0}});
            }
        }
        else if (stat == STAT_PROGRAMME_CHANGE) {
            //Never seen PROGRAMME CHANGE, likely never will
            if (this->debug) fprintf(stderr, "        PROGRAMME CHANGE\n");
        }
        else if (stat == STAT_CHANNEL_PRESSURE) {
            //Never seen CHANNEL PRESSURE, likely never will
            if (this->debug) fprintf(stderr, "        CHANNEL PRESSURE\n");
        }
        else if (stat == STAT_PITCH_WHEEL) {
            //Never seen PITCH WHEEL message likely found with NOTE_ON
            if (this->debug) fprintf(stderr, "        PITCH WHEEL\n");
        }
        else if (stat == META_END_OF_SEQUENCE) {
            if (this->debug) fprintf(stderr, "        END OF TRACK\n");
            for (auto &trk : tmsgs) trk.push_back({fabs, META_END_OF_SEQUENCE});
            break;
        }
        else if (stat == META_TEMPO) {
            if (this->debug) fprintf(stderr, "        TEMPO CHANGE TO %g BPM\n", 60000000.0 / mesgs[e].tval);
            if (!e) continue;
            unsigned char tmp[] = {mesgs[e].tval>>16, mesgs[e].tval>>8, mesgs[e].tval>>0};
            tmsgs[0].push_back({abs, META_TEMPO, tmp});
        }
    }
    
    //Sort and set final messages
    this->mmsgs.resize(tmsgs.size());
    for (int t = 0; t < tmsgs.size(); ++t) {
        auto is_bend = [](const midi_mesg &m) -> bool { return m.mmsg_stat & 0xFFF0 == STAT_PITCH_WHEEL; };
        
        if (tmsgs[t].empty()) continue;
        
        std::sort(tmsgs[t].begin(), tmsgs[t].end());
        if (t) {
            auto &trk = this->mmsgs[t];
            trk.push_back({0, STAT_PROGRAMME_CHANGE | (t - 1), t - 1});
            if (std::find_if(trk.begin(), trk.end(), is_bend) != trk.end()) {
                trk.push_back({0, STAT_CONTROLLER | (t - 1), {CC_REGISTERED_PARAMETER_C, 0}});
                trk.push_back({0, STAT_CONTROLLER | (t - 1), {CC_REGISTERED_PARAMETER_F, 0}});
                trk.push_back({0, STAT_CONTROLLER | (t - 1), {CC_DATA_ENTRY_C, 2}});
                trk.push_back({0, STAT_CONTROLLER | (t - 1), {CC_DATA_ENTRY_F, 0}});
                trk.push_back({0, STAT_CONTROLLER | (t - 1), {CC_REGISTERED_PARAMETER_C, 127}});
                trk.push_back({0, STAT_CONTROLLER | (t - 1), {CC_REGISTERED_PARAMETER_F, 127}});
            }
        }
        this->mmsgs[t].insert(this->mmsgs[t].end(), tmsgs[t].begin(), tmsgs[t].end());
        tmsgs[t].clear();
    }
    
    //Remove empty tracks
    for (auto itr = this->mmsgs.begin(); itr < this->mmsgs.end();) {
        if ((*itr).empty()) itr = this->mmsgs.erase(itr);
        else itr += 1;
    }
    
    //Update number tracks
    this->mhead.mthd_trks = this->mmsgs.size();
    
    //Set MIDI to playmidi
    getMidi(lrt_start, lrt_size);
    //Reusing pointer thingy because why not?
    setSequence(lrt_start, lrt_size);

    return 1;
}