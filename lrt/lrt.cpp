#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>
#include "directory.hpp"
#include "mid/mid_types.hpp"
#include "mid/mid_cc.hpp"
#include "mid/mid_meta.hpp"
#include "mid/mid_shared.hpp"
#include "playmidi/playmidi.hpp"
#include "lrt_types.hpp"
#include "lrt.hpp"


int lbrt::setLRT(std::string lrt_file) {
    this->lrt_path = lrt_file.substr(0, lrt_file.find_last_of("\\/"));
    this->seq_name = lrt_file.substr(lrt_file.find_last_of("\\/") + 1);
    this->seq_name = this->seq_name.substr(0, this->seq_name.find_last_of('.'));
    if (this->debug) fprintf(stderr, "    Title of sequence: %s\n", this->seq_name.c_str());

    const int MAX_CHANNELS = 16;
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
    auto make_short = [](unsigned short c0, unsigned short c1) -> unsigned short {
        return (c0 << 8) | (c1 & 0xFF);
    };


    if (!getFileData(lrt_file.c_str(), lrt_data, lrt_size)) {
        fprintf(stderr, "    Unable to open %s\n", this->seq_name.c_str());
        return 0;
    }
    else lrt_start = lrt_data;

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
    unsigned char bends[num_chan] {};
    for (auto b : bends) b = 0x40;


    //Insert global settings
    if (this->debug) fprintf(stderr, "\n    Set MIDI sequences\n");
    std::vector<midi_mesg> fmsgs;
    fmsgs.emplace_back(0, STAT_RESET, midi_mval(META_TRACK_NAME, this->seq_name.size(), this->seq_name.c_str()));
    fmsgs.emplace_back(0, STAT_RESET, midi_mval(META_TIME_SIGNATURE, {4, 2, hedr.npqn, 8}));
    fmsgs.emplace_back(0, STAT_CONTROLLER, make_short(CC_REGISTERED_PARAMETER_C, 0));
    fmsgs.emplace_back(0, STAT_CONTROLLER, make_short(CC_REGISTERED_PARAMETER_F, 0));
    fmsgs.emplace_back(0, STAT_CONTROLLER, make_short(CC_DATA_ENTRY_C, 2));
    fmsgs.emplace_back(0, STAT_CONTROLLER, make_short(CC_DATA_ENTRY_F, 0));
    fmsgs.emplace_back(0, STAT_CONTROLLER, make_short(CC_REGISTERED_PARAMETER_C, 127));
    fmsgs.emplace_back(0, STAT_CONTROLLER, make_short(CC_REGISTERED_PARAMETER_F, 127));
    for (unsigned char c = 0; c < num_chan; ++c) fmsgs.emplace_back(0, STAT_PROGRAMME_CHANGE | c, c);

    //Set final messages
    //Additionally, skip initial tempo event
    for (int e = 1, abs = 0, fabs = 0; e < hedr.msgs; ++e) {
        //Add delta to absolute time
        abs += mesgs[e].dtim;

        if (mesgs[e].stat < STAT_NOTE_OFF) {
            //Theoretically shouldn't do anything
            if (this->debug) fprintf(stderr, "        RUNNING STATUS\n");
        }
        else if (mesgs[e].stat < STAT_NOTE_ON) {
            //NOTE_OFF message found with NOTE_ON
        }
        else if (mesgs[e].stat < STAT_KEY_PRESSURE) {
            if (this->debug) fprintf(stderr, "        NOTE ON/OFF EVENT\n");
            fabs = abs + mesgs[e].tval;

            fmsgs.emplace_back(abs, mesgs[e].stat,
                               make_short(mesgs[e].eval0, mesgs[e].velon));
            fmsgs.emplace_back(fabs, STAT_NOTE_OFF | mesgs[e].chn,
                               make_short(mesgs[e].eval0, mesgs[e].veloff));

            if (bends[mesgs[e].chn] != mesgs[e].bndon) {
                bends[mesgs[e].chn] = mesgs[e].bndon;
                fmsgs.emplace_back(abs, STAT_PITCH_WHEEL | mesgs[e].chn,
                                   (unsigned short)bends[mesgs[e].chn]);
            }
            if (bends[mesgs[e].chn] != mesgs[e].bndoff) {
                bends[mesgs[e].chn] = mesgs[e].bndoff;
                fmsgs.emplace_back(fabs, STAT_PITCH_WHEEL | mesgs[e].chn,
                                   (unsigned short)bends[mesgs[e].chn]);
            }
        }
        else if (mesgs[e].stat < STAT_CONTROLLER) {
            //Never seen KEY PRESSURE, likely never will
            if (this->debug) fprintf(stderr, "        KEY PRESSURE\n");
        }
        else if (mesgs[e].stat < STAT_PROGRAMME_CHANGE) {
            //Sequence uses CC 99 for looping
            //Will use CC 116/117 instead
            //Additionally use Final Fantasy style just because
            if (mesgs[e].cc == CC_PSX_LOOP) {
                if (mesgs[e].eval0 == CC_PSX_LOOPSTART) {
                    if (this->debug) fprintf(stderr, "        LOOP START EVENT\n");
                    fmsgs.emplace_back(abs, mesgs[e].stat, make_short(CC_XML_LOOPSTART, CC_XML_LOOPINFINITE));
                    fmsgs.emplace_back(abs, STAT_RESET, midi_mval(META_MARKER, 9, "loopStart"));
                }
                else if (mesgs[e].eval0 == CC_PSX_LOOPEND) {
                    if (this->debug) fprintf(stderr, "        LOOP STOP EVENT\n");
                    fmsgs.emplace_back(abs, mesgs[e].stat, make_short(CC_XML_LOOPEND, CC_XML_LOOPRESERVED));
                    fmsgs.emplace_back(fabs, STAT_RESET, midi_mval(META_MARKER, 7, "loopEnd"));
                }
            }
            else {
                if (this->debug) fprintf(stderr, "        CC %u EVENT\n", mesgs[e].cc);
                fmsgs.emplace_back(abs, mesgs[e].stat, make_short(mesgs[e].eval0, mesgs[e].eval1));
            }
        }
        else if (mesgs[e].stat < STAT_CHANNEL_PRESSURE) {
            //Never seen PROGRAMME CHANGE, likely never will
            if (this->debug) fprintf(stderr, "        PROGRAMME CHANGE\n");
        }
        else if (mesgs[e].stat < STAT_PITCH_WHEEL) {
            //Never seen CHANNEL PRESSURE, likely never will
            if (this->debug) fprintf(stderr, "        CHANNEL PRESSURE\n");
        }
        else if (mesgs[e].stat < STAT_SYSTEM_EXCLUSIVE) {
            //Never seen PITCH WHEEL message likely found with NOTE_ON
            if (this->debug) fprintf(stderr, "        PITCH WHEEL\n");
        }
        else {
            if (mesgs[e].cc == META_END_OF_SEQUENCE) {
                if (this->debug) fprintf(stderr, "        END OF TRACK\n");
                fmsgs.emplace_back(fabs, STAT_RESET, midi_mval(META_END_OF_SEQUENCE));
                break;
            }
            else if (mesgs[e].cc == META_TEMPO) {
                if (this->debug) fprintf(stderr, "        TEMPO CHANGE TO %g BPM\n", 60000000.0 / mesgs[e].tval);
                unsigned char tmp[] = {mesgs[e].tval>>16, mesgs[e].tval>>8, mesgs[e].tval>>0};
                fmsgs.emplace_back(abs, STAT_RESET, midi_mval(META_TEMPO, tmp));
            }
        }
    }


    //Assign MIDI header
    if (this->debug) fprintf(stderr, "\n    Finalize MIDI header\n");
    reset();
    this->mhead.mthd_frmt = hedr.frmt;
    this->mhead.mthd_trks = num_chan + 1;
    this->mhead.mthd_divi = hedr.ppqn & 0x7F;

    //Assign MIDI tracks
    this->amnt_mmsg = new unsigned[this->mhead.mthd_trks] {};
    this->mmsgs = new midi_mesg*[this->mhead.mthd_trks] {};
    for (int t = -1; t < num_chan; ++t) {
        std::vector<midi_mesg> tmsgs;

        if (t < 0) {
            auto is_global = [](midi_mesg m) { return m.mmsg_stat == STAT_RESET; };

            tmsgs.resize(fmsgs.size());
            auto itc = std::copy_if(fmsgs.begin(), fmsgs.end(), tmsgs.begin(), is_global);
            tmsgs.resize(itc - tmsgs.begin());

            auto itr = std::remove_if(fmsgs.begin(), fmsgs.end(), is_global);
            fmsgs.resize(itr - fmsgs.begin());
        }
        else {
            auto is_chan_event = [&](midi_mesg m) {
                return (m.mmsg_stat < STAT_SYSTEM_EXCLUSIVE) && (m.mmsg_stat & 0x0F) == t;
            };

            tmsgs.resize(fmsgs.size());
            auto itc = std::copy_if(fmsgs.begin(), fmsgs.end(), tmsgs.begin(), is_chan_event);
            tmsgs.resize(itc - tmsgs.begin());

            auto itr = std::remove_if(fmsgs.begin(), fmsgs.end(), is_chan_event);
            fmsgs.resize(itr - fmsgs.begin());
        }

        this->amnt_mmsg[t + 1] = tmsgs.size();
        this->mmsgs[t + 1] = new midi_mesg[this->amnt_mmsg[t + 1]] {};
        std::sort(tmsgs.begin(), tmsgs.end());
        std::move(tmsgs.begin(), tmsgs.end(), this->mmsgs[t + 1]);
    }
    if (!fmsgs.empty()) fmsgs.clear();


    //Set MIDI to playmidi
    unsigned char *mid_data = 0;
    unsigned mid_size = 0;
    getMidi(mid_data, mid_size);
    setSequence(mid_data, mid_size);

    return 1;
}

int lbrt::writeMidi() {
    bool isSuccess = false;

    std::string out_file = this->lrt_path + this->seq_name;

    //Set MIDI file
    unsigned char *mid_data = 0;
    unsigned mid_size = 0;
    getMidi(mid_data, mid_size);

    if (this->debug) fprintf(stderr, "\n    Writing to MIDI file\n");
    if (!createFile((out_file + ".mid").c_str(), mid_data, mid_size)) {
        fprintf(stderr, "    Unable to save %s to MIDI file\n", this->seq_name.c_str());
    }
    else {
        fprintf(stdout, "    %s successfully written to MIDI file\n", this->seq_name.c_str());
        isSuccess = true;
    }

    //Set CSV file
    if (this->debug) {
        std::string csv_data = getCsv();

        fprintf(stderr, "\n    Writing to CSV file\n");
        if (!createFile((out_file + ".csv").c_str(), csv_data.c_str(), csv_data.length())) {
            fprintf(stderr, "    Unable to save %s to CSV file\n", this->seq_name.c_str());
        }
        else fprintf(stdout, "    %s successfully written to CSV file\n", this->seq_name.c_str());
    }

    return isSuccess;
}
