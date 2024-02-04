#include <algorithm>
#include <cstdio>
#include <cstdint>
#include <string>
#include <vector>
#include "directory.hpp"
#include "lrt_types.hpp"
#include "lrt.hpp"


unsigned int lbrt::getLeInt(unsigned char *&in, int length) {
    unsigned int out = 0;
    for (int i = 0; i < length; ++i) out = (out << 8) | *(in++);
    return out;
}


int lbrt::setLRT(std::string lrt_file) {
    const int MAX_CHANNELS = 16;

    this->basepath = lrt_file.substr(0, lrt_file.find_first_of('.'));

    std::string title;
    title = this->basepath.substr(this->basepath.find_last_of("\\/") + 1);
    if (this->debug) fprintf(stderr, "    Title of sequence: %s\n", title.c_str());

    uchar *lrt_data = 0, *lrt_start = 0;
    uint32_t lrt_size = 0;

    if (!getFileData(lrt_file.c_str(), lrt_data, lrt_size)) {
        fprintf(stderr, "    Unable to open %s\n", title.c_str());
        return 0;
    }
    else if (!cmpStr(lrt_data, LBRT_LBRT, 4)) {
        fprintf(stderr, "    This is not an LBRT file\n");
        return 0;
    }
    else lrt_start = lrt_data;

    lbrt_head hedr;
    lrt_data += 4;
    hedr.soff = getLeInt(lrt_data, 4);
    hedr.npqn = getLeInt(lrt_data, 4);
    hedr.ppqn = getLeInt(lrt_data, 4);
    hedr.trks = getLeInt(lrt_data, 4);
    hedr.frmt = getLeInt(lrt_data, 4);
    hedr.val0 = getLeInt(lrt_data, 4);
    hedr.msgs = getLeInt(lrt_data, 4);
    hedr.qrts = getLeInt(lrt_data, 4);
    hedr.qids = new uint32_t[hedr.qrts] {};

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
        hedr.qids[i] = getLeInt(lrt_data, 4);
        if (this->debug) fprintf(stderr, "        Quarter event ID %04u: %04u\n", i, hedr.qids[i]);
    }


    //Stuff for keeping track of pitch bend
    uint8_t **bends = new uint8_t*[MAX_CHANNELS] {};


    //Set sequence events
    if (this->debug) fprintf(stderr, "\n    Retrieving LBRT sequences\n");
    lbrt_mesg mesgs[hedr.msgs] = {};
    std::vector<midi_mesg> fmsgs;

    lrt_data = lrt_start + hedr.soff;
    for (int e = 0; e < hedr.msgs; ++e) {
        mesgs[e].id = getLeInt(lrt_data, 4);
        mesgs[e].dtim = getLeInt(lrt_data, 4);
        mesgs[e].tval = getLeInt(lrt_data, 4);
        mesgs[e].eval0 = getLeInt(lrt_data, 4);
        mesgs[e].eval1 = getLeInt(lrt_data, 2);
        mesgs[e].cc = getLeInt(lrt_data, 2);
        mesgs[e].velon = getLeInt(lrt_data, 2);
        mesgs[e].bndon = getLeInt(lrt_data, 2);
        mesgs[e].chn = *(lrt_data++);
        mesgs[e].stat = *(lrt_data++);
        mesgs[e].veloff = *(lrt_data++);
        mesgs[e].bndoff = *(lrt_data++);

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

        if (mesgs[e].stat != STAT_RESET && !bends[mesgs[e].chn]) {
            bends[mesgs[e].chn] = new uint8_t[1] {0x40};
        }
    }

    
    //Set final messages
    //Additionally, skip initial tempo event
    if (this->debug) fprintf(stderr, "\n    Set MIDI sequences\n");
    for (int e = 1, abs = 0, fabs = 0; e < hedr.msgs; ++e) {
        //Add delta to absolute time
        abs += mesgs[e].dtim;

        switch (mesgs[e].stat & 0xF0) {
            case STAT_NONE:
                //Theoretically shouldn't do anything
                if (this->debug) fprintf(stderr, "        RUNNING STATUS\n");
                continue;

            case STAT_NOTE_ON:
                if (this->debug) fprintf(stderr, "        NOTE ON/OFF EVENT\n");
                fabs = abs + mesgs[e].tval;

                fmsgs.push_back({abs, mesgs[e].stat, {mesgs[e].eval0, mesgs[e].velon}});
                fmsgs.push_back({fabs, STAT_NOTE_OFF | mesgs[e].chn, {mesgs[e].eval0, mesgs[e].veloff}});

                if (bends[mesgs[e].chn][0] != mesgs[e].bndon) {
                    bends[mesgs[e].chn][0] = mesgs[e].bndon;
                    fmsgs.push_back({abs, STAT_PITCH_WHEEL | mesgs[e].chn,
                                    getPitchBend(bends[mesgs[e].chn][0])});
                }
                if (bends[mesgs[e].chn][0] != mesgs[e].bndoff) {
                    bends[mesgs[e].chn][0] = mesgs[e].bndoff;
                    fmsgs.push_back({fabs, STAT_PITCH_WHEEL | mesgs[e].chn,
                                    getPitchBend(bends[mesgs[e].chn][0])});
                }
                continue;

            case STAT_CONTROLLER:
                //Sequence uses CC 99 for looping
                //Will use CC 116/117 instead
                //Additionally use Final Fantasy style just because
                if (mesgs[e].cc == CC_PSX_LOOP) {
                    if (mesgs[e].eval0 == CC_PSX_LOOPSTART) {
                        if (this->debug) fprintf(stderr, "        LOOP START EVENT\n");
                        fmsgs.push_back({abs, mesgs[e].stat, {CC_XML_LOOPSTART, CC_XML_LOOPINFINITE}});
                        fmsgs.push_back({abs, STAT_RESET, midi_mval(META_MARKER, 9, "loopStart")});
                    }
                    else if (mesgs[e].eval0 == CC_PSX_LOOPEND) {
                        if (this->debug) fprintf(stderr, "        LOOP STOP EVENT\n");
                        fmsgs.push_back({abs, mesgs[e].stat, {CC_XML_LOOPEND, CC_XML_LOOPRESERVED}});
                        fmsgs.push_back({fabs, STAT_RESET, midi_mval(META_MARKER, 7, "loopEnd")});
                    }
                }
                else {
                    if (this->debug) fprintf(stderr, "        CC %u EVENT\n", mesgs[e].cc);
                    fmsgs.push_back({abs, mesgs[e].stat, {mesgs[e].eval0, mesgs[e].eval1}});
                }
                continue;

            case (STAT_RESET & 0xF0):
                if (mesgs[e].cc == META_END_OF_SEQUENCE) {
                    if (this->debug) fprintf(stderr, "        END OF TRACK\n");
                    fmsgs.push_back({fabs, mesgs[e].stat, {mesgs[e].cc, 0, 0}});
                    break;
                }
                else if (mesgs[e].cc == META_TEMPO) {
                    if (this->debug) fprintf(stderr, "        TEMPO CHANGE TO %g BPM\n", 60000000.0 / mesgs[e].tval);
                    uint8_t tmp[] = {mesgs[e].tval>>16, mesgs[e].tval>>8, mesgs[e].tval>>0};
                    fmsgs.push_back({0, STAT_RESET, {mesgs[e].cc, tmp}});
                }
                continue;

            default:
                if (this->debug) fprintf(stderr, "        EVENT TYPE 0x%02X\n", mesgs[e].stat);
                continue;
        }
        
        //Forgot switch only breaks from... switch
        break;
    }

    //Reorganize track by absolute time and status
    if (this->debug) fprintf(stderr, "\n    Reorganize MIDI sequences\n");
    std::sort(fmsgs.begin(), fmsgs.end());
    
    //Insert global settings
    fmsgs.insert(fmsgs.begin() + 0, {0, STAT_RESET, {META_TRACK_NAME, title.size(), title.c_str()}});
    fmsgs.insert(fmsgs.begin() + 1, {0, STAT_RESET, {META_TIME_SIGNATURE, {4, 2, hedr.npqn, 8}}});
    fmsgs.insert(fmsgs.begin() + 2, {0, STAT_CONTROLLER, {CC_REGISTERED_PARAMETER_C, 0}});
    fmsgs.insert(fmsgs.begin() + 3, {0, STAT_CONTROLLER, {CC_REGISTERED_PARAMETER_F, 0}});
    fmsgs.insert(fmsgs.begin() + 4, {0, STAT_CONTROLLER, {CC_DATA_ENTRY_C, 2}});
    fmsgs.insert(fmsgs.begin() + 5, {0, STAT_CONTROLLER, {CC_DATA_ENTRY_F, 0}});
    fmsgs.insert(fmsgs.begin() + 6, {0, STAT_CONTROLLER, {CC_REGISTERED_PARAMETER_C, 127}});
    fmsgs.insert(fmsgs.begin() + 7, {0, STAT_CONTROLLER, {CC_REGISTERED_PARAMETER_F, 127}});
    for (int c = 0, m = 8; c < MAX_CHANNELS; ++c) {
        if (!bends[c]) continue;
        fmsgs.insert(fmsgs.begin() + m++, {0, (STAT_PROGRAMME_CHANGE | c), c});
        delete[] bends[c];
    }
    delete[] bends;

    //Save MIDI sequences
    if (this->debug) fprintf(stderr, "\n    Finalize MIDI header\n");
    this->format = hedr.frmt;
    this->amnt_trks = hedr.trks;
    this->division = ppqn(hedr.ppqn);

    if (this->debug) fprintf(stderr, "\n    Finalize MIDI track\n");
    if (this->trcks) { delete[] this->trcks; this->trcks = 0; }
    this->trcks = new trck_list[this->amnt_trks] {};
    this->trcks[0] = {fmsgs.size(), fmsgs.data()};

    return 1;
}

int lbrt::writeMidi() {
    bool isSuccess = false;

    std::string out_file = basepath + ".mid";

    //Set MIDI file
    unsigned mid_size = 0;
    uchar *mid_data = 0;
    getMidi(mid_size, mid_data);

    if (this->debug) fprintf(stderr, "\n    Writing to MIDI file\n");
    if (!createFile(out_file.c_str(), mid_data, mid_size)) {
        fprintf(stderr, "    Unable to save %s\n", out_file.c_str());
    }
    else {
        fprintf(stdout, "    %s successfully converted\n",
                this->basepath.substr(this->basepath.find_last_of("\\/") + 1).c_str());
        isSuccess = true;
    }

    //Set CSV file
    if (this->debug) {
        out_file = basepath + ".csv";
        std::string csv_data = getCsv();

        if (!createFile(out_file.c_str(), csv_data.c_str(), csv_data.length())) {
            fprintf(stderr, "    Unable to save %s\n", out_file.c_str());
        }
    }

    return isSuccess;
}
