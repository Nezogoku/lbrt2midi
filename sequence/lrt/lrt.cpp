#include <algorithm>
#include <cstdint>
#include <fstream>
#include <string>
#include "../../bit_byte.hpp"
#include "lrt_types.hpp"
#include "lrt.hpp"


char *getVLV(uint32_t val, uint32_t &v_size) {
    uint32_t vlv = val & 0x7F;

    while (val >>= 7) {
        vlv = (vlv << 8) | ((val & 0x7F) | 0x80);
    }

    char *dst = new char[4];
    for (v_size = 1; v_size <= 4; ++v_size) {
        dst[v_size - 1] = (uchar)(vlv);

        if (vlv & 0x80) vlv >>= 8;
        else break;
    }

    return dst;
}


int readLRT(std::string &lrt_file, std::string &mid_file, bool debug) {
    const int MAX_CHANNELS = 16, MAX_EVENTS = 3000;
    bool isSuccess = false;

    std::string title, csv_file;
    title = lrt_file.substr(lrt_file.find_last_of("\\/") + 1);
    title = title.substr(0, title.find_first_of('.'));
    if (debug) fprintf(stdout, "\nTitle of sequence: %s\n", title.c_str());

    std::ifstream lbrt_file(lrt_file, std::ios::binary);
    if (!lbrt_file.is_open()) {
        fprintf(stderr, "\nUnable to open %s\n", title.c_str());
        return isSuccess;
    }

    lbrt_file.seekg(0, std::ios::end);
    uint32_t lbrt_length = lbrt_file.tellg();
    lbrt_file.seekg(0, std::ios::beg);

    uchar *lrt_data = new uchar[lbrt_length];
    lbrt_file.read((char*)(lrt_data), lbrt_length);
    lbrt_file.close();

    if (!cmpChar(lrt_data, "LBRT", 4)) {
        fprintf(stderr, "\nThe is not an LBRT file\n");
        return isSuccess;
    }

    uchar *lrt_start = lrt_data;

    lrt_data += 0x04;
    uint32_t data_offset = getLeInt(lrt_data, 4);
    uint32_t data_clocks = getLeInt(lrt_data, 4);
    uint32_t data_ppqn = getLeInt(lrt_data, 4);
    uint32_t data_tracks = getLeInt(lrt_data, 4);
    uint32_t data_format = getLeInt(lrt_data, 4);
    uint32_t data_unk0 = getLeInt(lrt_data, 4);
    uint32_t data_events = getLeInt(lrt_data, 4);
    uint32_t data_qnotes = getLeInt(lrt_data, 4);

    if (debug) fprintf(stdout, "    Sequence data at position: 0x%08X\n", data_offset);
    if (debug) fprintf(stdout, "    Notated 32nd-notes per quarter note: %u\n", data_clocks);
    if (debug) fprintf(stdout, "    Pulses per quarter note: %u\n", data_ppqn);
    if (debug) fprintf(stdout, "    Total tracks: %u\n", data_tracks);
    if (debug) fprintf(stdout, "    Sequence format: %u\n", data_format);
    if (debug) fprintf(stdout, "    Total events: %u\n", data_events);
    if (debug) fprintf(stdout, "    Total quarter notes: %u\n", data_qnotes);

    uint32_t list_qnotes[data_qnotes] = {};
    for (auto &qn : list_qnotes) {
        qn = getLeInt(lrt_data, 4);
        if (debug) fprintf(stdout, "        Quarter note at position: 0x%08X\n", qn);
    }


    //Set up 16 tracks based off ce_chans
    //Additional track for global settings
    midi_status list_midi[MAX_CHANNELS + 1][MAX_EVENTS] = {};
    int idx[MAX_CHANNELS + 1] = {};

    //Initial global settings
    list_midi[16][idx[16]++] = midi_status(0, 0x0F, meta(0x03, title.size(), (uchar*)title.c_str()));
    list_midi[16][idx[16]++] = midi_status(0, 0x0F, meta(0x58, 4, (uchar*)getFData("\x04\x02%c\x08", data_clocks)));


    if (debug) fprintf(stdout, "\nRetrieving LBRT sequences\n");

    //Set sequence events
    lbrt_status list_lbrt[data_events] = {};
    for (int e = 0, b = 0; e < data_events; ++e) {
        lrt_data = lrt_start + data_offset + (e * 0x1C);

        list_lbrt[e].event_id = getLeInt(lrt_data, 4);
        list_lbrt[e].delta_t = getLeInt(lrt_data, 4);
        list_lbrt[e].time_val = getLeInt(lrt_data, 4);
        list_lbrt[e].ce_val = getLeInt(lrt_data, 4);
        list_lbrt[e].unk_val0 = getLeInt(lrt_data, 2);
        list_lbrt[e].ce_typ = getLeInt(lrt_data, 2);
        list_lbrt[e].ce_vel = getLeInt(lrt_data, 2);
        list_lbrt[e].ce_pan = getLeInt(lrt_data, 2);
        list_lbrt[e].ce_chan = getLeInt(lrt_data, 1);
        list_lbrt[e].status = getLeInt(lrt_data, 1);
        list_lbrt[e].unk_val3 = getLeInt(lrt_data, 1);
        list_lbrt[e].unk_val4 = getLeInt(lrt_data, 1);
    }


    if (debug) fprintf(stdout, "\nReorganizing LBRT sequences\n");

    //BIG BRAIN move, skip the first event
    //It's setting up the initial tempo, but I'm going to pretend it doesn't exist
    for (int e = 1, abs = 0; e < data_events; ++e) {
        if (debug) fprintf(stdout, "    Current event: %u\n", e);
        if (debug) fprintf(stdout, "        Event ID: %u\n", list_lbrt[e].event_id);
        if (debug) fprintf(stdout, "        Delta time: %ums\n", list_lbrt[e].delta_t);
        if (debug) fprintf(stdout, "        Time value: %u\n", list_lbrt[e].time_val);
        if (debug) fprintf(stdout, "        Channel event value: %u\n", list_lbrt[e].ce_val);
        if (debug) fprintf(stdout, "        Unknown value 1: %u\n", list_lbrt[e].unk_val0);
        if (debug) fprintf(stdout, "        Channel Controller value: %u\n", list_lbrt[e].ce_typ);
        if (debug) fprintf(stdout, "        Channel velocity: %u\n", list_lbrt[e].ce_vel);
        if (debug) fprintf(stdout, "        Channel pan: %u\n", list_lbrt[e].ce_pan);
        if (debug) fprintf(stdout, "        Channel: %u\n", list_lbrt[e].ce_chan);
        if (debug) fprintf(stdout, "        Status: 0x%02X\n", list_lbrt[e].status);
        if (debug) fprintf(stdout, "        Unknown value 4: %u\n", list_lbrt[e].unk_val3);
        if (debug) fprintf(stdout, "        Unknown value 5: %u\n", list_lbrt[e].unk_val4);

        //Add delta to absolute time
        abs += list_lbrt[e].delta_t;

        //Assign event
        int id = list_lbrt[e].event_id,
            ch = list_lbrt[e].ce_chan;
        switch(list_lbrt[e].status & 0xF0) {
            case 0x00:          // Running status (?)
                //Theoretically shouldn't do anything
                //Only occurs at quarter note mark
                if (debug) fprintf(stdout, "        RUNNING STATUS\n");
                continue;

            case 0x90:          // Note on/off event
                if (debug) fprintf(stdout, "        NOTE ON/OFF EVENT\n");

                if (true) {
                    uchar n_pitch = list_lbrt[e].ce_val,
                          n_velocity = list_lbrt[e].ce_vel;
                    uint16_t n_pan = list_lbrt[e].ce_pan;
                    uint32_t n_release = list_lbrt[e].time_val;

                    list_midi[ch][idx[ch]++] = midi_status(abs, ch, noteonoff(n_pitch, n_velocity, n_pan));
                    list_midi[ch][idx[ch]++] = midi_status(abs + n_release, ch, noteonoff(n_pitch, 0, n_pan));
                }
                continue;

            case 0xB0:          // Controller event
                if (list_lbrt[e].ce_typ == 0x63) {
                    meta ff;
                    control cc;

                    //Sequence uses CC 99 for looping
                    //Will use CC 116/117 instead
                    //Additionally use Final Fantasy style just because
                    if (list_lbrt[e].ce_val == 0x14) {
                        if (debug) fprintf(stdout, "        LOOP START EVENT\n");

                        ff = meta(0x06, 9, (uchar*)"loopStart");// Final Fantasy style
                        cc = control(0x74, 0x00);               // CC 116 (EMIDI/XMI style)
                    }
                    else if (list_lbrt[e].ce_val == 0x1E) {
                        if (debug) fprintf(stdout, "        LOOP STOP EVENT\n");

                        ff = meta(0x06, 7, (uchar*)"loopEnd");  // Final Fantasy style
                        cc = control(0x75, 0x7F);               // CC 117 (EMIDI/XMI style)
                    }
                    else {
                        if (debug) fprintf(stdout, "        UNKNOWN EVENT\n");
                        continue;;
                    }

                    list_midi[16][idx[16]++] = midi_status(abs, 0x0F, ff);
                    list_midi[16][idx[16]++] = midi_status(abs, 0x0F, cc);
                }
                else {
                    if (debug) fprintf(stdout, "        CC %u EVENT\n", list_lbrt[e].ce_typ);

                    list_midi[ch][idx[ch]++] = midi_status(abs, ch, control(list_lbrt[e].ce_typ, list_lbrt[e].ce_val));
                }
                continue;

            case 0xF0:          // Meta event
                if (list_lbrt[e].status == 0xFF) {
                    if (list_lbrt[e].ce_typ == 0x2F) {
                        if (debug) fprintf(stdout, "        END OF TRACK\n");

                        list_midi[16][idx[16]++] = midi_status(abs, 0x0F, meta(0x2F, 0, 0));
                        break;
                    }
                    else if (list_lbrt[e].ce_typ == 0x51) {
                        if (debug) fprintf(stdout, "        TEMPO CHANGE TO %g BPM\n",
                                           60000000.0 / list_lbrt[e].time_val);

                        uint32_t tmp = list_lbrt[e].time_val;
                        list_midi[16][idx[16]++] = midi_status(abs, 0x0F,
                                                               meta(0x51, 3, (uchar*)getFData("%c%c%c", tmp >> 16, tmp >> 8, tmp)));
                    }
                }
                else {
                    int reset = list_lbrt[e].status & 0x0F;
                    std::string reset_type;

                    if (reset == 0x00) reset_type = "SYSEX";
                    else if (reset == 0x01) reset_type = "QUARTER FRAME MESSAGE";
                    else if (reset == 0x02) reset_type = "SEQUENCE POINTER";
                    else if (reset == 0x03) reset_type = "SEQUENCE SELECT";
                    else if (reset == 0x06) reset_type = "TUNE REQUEST";
                    else if (reset == 0x08) reset_type = "MIDI CLOCK";
                    else if (reset == 0x0A) reset_type = "MIDI START";
                    else if (reset == 0x0B) reset_type = "MIDI CONTINUE";
                    else if (reset == 0x0C) reset_type = "MIDI CLOCK";
                    else if (reset == 0x0E) reset_type = "ACTIVE SENSE";

                    if (debug) fprintf(stdout, "        %s EVENT\n", reset_type.c_str());
                }
                continue;

            default:            // Unknown
                if (debug) fprintf(stdout, "        EVENT TYPE 0x%02X\n", list_lbrt[e].status);
                continue;
        }
    }


    int n_trck = 1;
    for (int c = 0; c < MAX_CHANNELS; ++c) {
        if (!idx[c]) continue;

        //Add initial labels and junk
        list_midi[c][idx[c]++] = midi_status(0, 0x0F, meta(0x03, 8, (uchar*)getFData("TRACK %02d", n_trck)));
        list_midi[c][idx[c]++] = midi_status(0, c, program(c));

        //Reorganize all played tracks by absolute time
        std::sort(list_midi[c], list_midi[c] + idx[c]);

        //Calculate panning
        uint16_t panC = 0x40;
        for (int e = 0, e_amnt = idx[c]; e < e_amnt; ++e) {
            if (list_midi[c][e].event_type != NOTE) continue;
            else if (list_midi[c][e].event.n.pan == panC) continue;

            panC = list_midi[c][e].event.n.pan;
            list_midi[c][idx[c]++] = midi_status(list_midi[c][e].absol_t, c, control(0x0A, panC));

            if (e == e_amnt - 1) {
                if (panC != 0x40) {
                    list_midi[c][idx[c]++] = midi_status(list_midi[c][e].absol_t, c, control(0x0A, 0x40));
                }
            }
        }

        //Reorganize all played tracks by absolute time... again
        std::sort(list_midi[c], list_midi[c] + idx[c]);

        //Add end-of-track event
        uint32_t abs = list_midi[c][idx[c] - 1].absol_t;
        list_midi[c][idx[c]++] = midi_status(abs, 0x0F, meta(0x2F, 0, 0));

        n_trck += 1;
    }


    mid_file = lrt_file.substr(0, lrt_file.find_last_of('.') + 1) + "mid";
    if (debug) fprintf(stdout, "\nPath of new MID file is %s\n", mid_file.c_str());

    std::string csv_data = "";

    std::ofstream out(mid_file, std::ios::binary);
    try {
        if (debug) fprintf(stdout, "    Writing to MIDI file\n");

        const char *MThd = "MThd",
                   *MThd_SIZE = "\x00\x00\x00\x06",
                   *MThd_FRMT = "\x00\x01",
                   *MTrk = "MTrk";

        //Output Midi header to MIDI file
        out.seekp(0x00);
        out.write(MThd, 4);                                         // Midi header
        out.write(MThd_SIZE, 4);                                    // Midi header size
        out.write(MThd_FRMT, 2);                                    // Midi format
        out.write(getFData("%c%c", n_trck >> 8, n_trck), 2);        // Number of tracks
        out.write(getFData("%c%c", data_ppqn >> 8, data_ppqn), 2);  // Pulses per quarter-note

        //Output Midi header to CSV string
        csv_data += getFData("0, 0, HEADER, 1, %d, %d\n", n_trck, data_ppqn);

        for (int ch = -1, t = 1, c; ch < MAX_CHANNELS; ++ch) {
            if (ch < 0) c = 16;
            else c = ch;

            if (!idx[c]) continue;

            uint32_t e_beg = out.tellp(), e_end, e_size;

            if (debug) fprintf(stdout, "    Track %02d\n", t);
            if (debug) fprintf(stdout, "        Start at offset: 0x%08X\n", e_beg);

            //Output Track header to MIDI file
            out.write(MTrk, 4);                                     // Track header
            out.write(getFData("0000"), 4);                         // Track size

            //Output Track header to CSV string
            csv_data += getFData("%d, 0, START_TRACK\n", t);

            //Write events
            uint32_t p_time = 0, d_size, m_size;
            for (int e = 0; e < idx[c]; ++e) {
                auto &ev = list_midi[c][e];

                char *d_time = getVLV(ev.absol_t - p_time, d_size),
                     *t_event, *s_event;

                uint8_t typ = ev.event_type;
                if (typ == META) {
                    uchar stat = 0xF0 | ev.channel,
                          type = ev.event.m.type,
                          lnth = ev.event.m.lnth,
                         *data = ev.event.m.data;
                    t_event = getFData("%c%c%c%s", stat, type, lnth, (data ? (char*)data : "\0"));
                    m_size = 3 + lnth;

                    uint32_t num = 0;
                    for (int s = 0; s < lnth; ++s) num = (num << 8) | data[s];

                    if (type == 0x2F) s_event = getFData("END_TRACK");
                    else if (type == 0x03) s_event = getFData("TITLE_T, \"%s\"", data);
                    else if (type == 0x06) s_event = getFData("MARKER_T, \"%s\"", data);
                    else if (type == 0x51) s_event = getFData("TEMPO, %d", num);
                    else if (type == 0x58) s_event = getFData("TIME_SIGNATURE, %d, %d, %d, %d",
                                                              data[0], data[1], data[2], data[3]);
                }
                else if (typ == PROGRAM) {
                    uchar stat = 0xC0 | ev.channel,
                          prgm = ev.event.p.prgm;

                    t_event = getFData("%c%c", stat, prgm);
                    m_size = 2;

                    s_event = getFData("PROGRAM_C, %d, %d", ev.channel, prgm);
                }
                else if (typ == CONTROL) {
                    uchar stat = 0xB0 | ev.channel,
                          type = ev.event.c.type,
                          valu = ev.event.c.val;

                    t_event = getFData("%c%c%c", stat, type, valu);
                    m_size = 3;

                    s_event = getFData("CONTROL_C, %d, %d, %d", ev.channel, type, valu);
                }
                else if (typ == BEND) {
                    uchar stat = 0xE0 | ev.channel,
                          ben0 = ev.event.b.bend >> 8,
                          ben1 = ev.event.b.bend >> 0;

                    t_event = getFData("%c%c%c", stat, ben0, ben1);
                    m_size = 3;

                    uint16_t num = uint16_t(ben1) << 7 | ben0;

                    s_event = getFData("PITCH_BEND_C, %d, %d", ev.channel, num);
                }
                else if (typ == NOTE) {
                    uchar stat = 0x90 | ev.channel,
                          npch = ev.event.n.note,
                          nvel = ev.event.n.vel;

                    t_event = getFData("%c%c%c", stat, npch, nvel);
                    m_size = 3;

                    s_event = getFData("NOTE_ON_C, %d, %d, %d", ev.channel, npch, nvel);
                }
                else continue;

                //Output Track event to MIDI file
                out.write(d_time, d_size);                          // Delta time
                out.write(t_event, m_size);                         // Event

                //Output Track event to CSV string
                csv_data += getFData("%d, %d, %s\n", t, ev.absol_t, s_event);

                p_time = ev.absol_t;
            }

            e_end = out.tellp();

            e_size = e_end - (e_beg + 8);
            if (debug) fprintf(stdout, "        Size of track: 0x%08X\n", e_size);

            out.seekp(e_beg + 4);
            out.write(getFData("%c%c%c%c", e_size >> 24, e_size >> 16, e_size >> 8, e_size), 4);
            out.seekp(e_end);

            t += 1;
        }

        csv_data += getFData("0, 0, END_OF_FILE\n");
        isSuccess = true;
    }
    catch (const std::fstream::failure &f) {
        fprintf(stderr, "\n%s\n", f.what());
    }
    catch (const std::exception &e) {
        fprintf(stderr, "\n%s\n", e.what());
    }
    out.close();

    if (debug) {
        csv_file = lrt_file.substr(0, lrt_file.find_last_of('.') + 1) + "csv";
        fprintf(stdout, "\nPath of new CSV file is %s\n", csv_file.c_str());

        out.open(csv_file);

        try {
            out.seekp(0x00);
            out.write(csv_data.c_str(), csv_data.size());
        }
        catch (const std::fstream::failure &f) {
            fprintf(stderr, "\n%s\n", f.what());
        }
        out.close();
    }

    return isSuccess;
}
