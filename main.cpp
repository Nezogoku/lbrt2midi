#include <algorithm>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
//#include "fluidsynth.h"
#include "sf2/sf2.hpp"
#include "psx_sgd/psxsgd.hpp"

// SF2cute library from gocha
// Based off LBRPlayer from owocek
//      Used for comparison stuffs
// Base codes in decode from jmarti856 (I basically just changed the language it was written in)

using std::cerr;
using std::cout;
using std::dec;
using std::endl;
using std::fstream;
using std::hex;
using std::ifstream;
using std::ios;
using std::ofstream;
using std::string;
using std::vector;


bool debug = false,
     play_result = false;

//Array for custom ordering events
//Because I don't know a C++ alternative to R's order(match()) function
const int numStat = 5;
const uchar statOrder[] = {0xF0, 0xC0, 0x90, 0xE0, 0xB0};

//Struct for MIDI events
struct lbrt_status {
    int quarter_note_id;    //Beat in a measure
    uint32_t absol_t;       //Absolute time
    uchar status_a,         //Event type
          status_b;         //Value or sub-event type
    vector<uchar> values;   //Values of event

    lbrt_status(int qnid, uint32_t ms, uchar stata, uchar statb, vector<uchar> val) :
        quarter_note_id(qnid), absol_t(ms), status_a(stata), status_b(statb), values(val) {}

    //To make sorting in ascending order easier
    bool operator<(const lbrt_status &lrt) const {
        int s0 = std::distance(statOrder, std::find(statOrder, statOrder + numStat, (status_a & 0xF0))),
            s1 = std::distance(statOrder, std::find(statOrder, statOrder + numStat, (lrt.status_a & 0xF0)));

        return (absol_t < lrt.absol_t) || ((absol_t == lrt.absol_t) && (s0 < s1));
    }
};


void printOpt(string pName) {
    if (debug) cerr << "Not enough valid files entered\n" << endl;
    cout << "Usage: " << pName << " [options] [<infile(s).sgd>] [<infile(s).lrt>]\n"
         << "\nOptions:\n"
         << "\t-h       Prints this message\n"
         << "\t-d       Activates and deactivates debug mode\n"
         << "\t-p       Activates and deactivates playback mode {WIP}\n"
         << endl;
}

int setInput(string ext, string &temp) {
    if (temp.substr(temp.find_last_of('.') + 1) != ext) {
        return 1;
    }
    else {
        if (debug) cout << ext << " file entered is " << temp << endl;
        return 0;
    }
}

void getDeltaTime(ofstream &sequence, uint32_t &dTime) {
    uint32_t buffer = dTime & 0x7F;

    while (dTime >>= 7) {
        buffer <<= 8;
        buffer |= ((dTime & 0x7F) | 0x80);
    }
    while (true) {
        sequence << uchar(buffer);
        if (buffer & 0x80) buffer >>= 8;
        else break;
    }

    dTime = 0x00;
}

void getChar(ofstream &sequence, int seqEvent, int sizeOf) {
    for (int s = sizeOf - 1; s >= 0; --s) {
        uchar buff = ((seqEvent >> (s * 8)) & 0xFF);
        sequence << buff;
    }
}

int writeLRT(string &lrt_file) {
    if (debug) cout << std::setfill(' ') << std::left;

    string title;
    //Allows for multiple tracks based on associated channel
    vector<vector<lbrt_status>> lbrtSequence(16, vector<lbrt_status>{});
    vector<uint32_t> change_ids;
    bool isSuccess = false;

    title = lrt_file.substr(lrt_file.find_last_of("\\/") + 1);
    title = title.substr(0, title.find_first_of('.'));
    if (debug) cout << "Title of sequence is " << title << endl;

    ifstream lbrt_file(lrt_file, ios::binary);
    if (!lbrt_file.is_open()) {
        cerr << "Unable to open " << title << endl;
        return 0;
    }


    uint32_t start_offset,
             clocks,
             ppqn,
             mid_format,
             entries,
             num_changes,
             entry_id,
             num_tracks = 0;

    lbrt_file.seekg(0x04);
    lbrt_file.read((char*)(&start_offset), sizeof(uint32_t));
    if (debug) cout << "\nStart offset is 0x" << hex << start_offset << dec << endl;

    lbrt_file.seekg(0x08);
    lbrt_file.read((char*)(&clocks), sizeof(uint32_t));
    if (debug) cout << "The number of notated 32nd-notes per quarter note is " << clocks << endl;

    lbrt_file.seekg(0x0C);
    lbrt_file.read((char*)(&ppqn), sizeof(uint32_t));
    if (debug) cout << "The PPQN of the current sequence is " << ppqn << endl;

    lbrt_file.seekg(0x10);
    lbrt_file.read((char*)(&mid_format), sizeof(uint32_t));
    if (debug) cout << "The MIDI format is " << mid_format << endl;

    lbrt_file.seekg(0x1C);
    lbrt_file.read((char*)(&entries), sizeof(uint32_t));
    if (debug) cout << "The number of entries is " << entries << endl;

    lbrt_file.seekg(0x20);
    lbrt_file.read((char*)(&num_changes), sizeof(uint32_t));
    if (debug) cout << "The number of segment changes is " << num_changes << endl;

    for (int in = 0; in < num_changes; ++in) {
        lbrt_file.seekg(0x24 + (0x04 * in));
        lbrt_file.read((char*)(&entry_id), sizeof(uint32_t));

        if (entry_id == 0x00) continue;

        change_ids.push_back(entry_id);
    }

    if (debug) for (const auto ch_ID : change_ids) cout << "\tEntry of next segment is " << ch_ID << endl;

    uint32_t entry_offset,
             event_id,
             event_time_val_0,
             event_time_val_1,
             event_chan_val;
    uint16_t event_type,
             event_val_0,
             event_val_1;
    uint8_t  event_status,
             status_byte,
             event_default_val_0,
             event_default_val_1;

    //For checking whether sequence is looped
    //Or if note in channel has pitch bend
    bool isLooped = false,
         isBent[] = {false, false, false, false,
                     false, false, false, false,
                     false, false, false, false,
                     false, false, false, false};

    int curr_entry = 0,
        absolute_time = 0;

    //Set sequence name
    lbrtSequence[0].push_back(lbrt_status(0, 0x00,
                                          0xFF, 0x03,
                                          vector<uchar>{title.begin(), title.end()}));
    //Set time signature
    lbrtSequence[0].push_back(lbrt_status(0, 0x00,
                                          0xFF, 0x58,
                                          vector<uchar>{0x04, 0x02, clocks, 0x08}));
    //Set sequence events
    //BIG BRAIN move, skip the first event
    //It's setting up the initial tempo, but I'm going to pretend it doesn't exist
    for(uint32_t i = 1; i < entries; ++i) {
        if (debug) cout << endl;
        if (debug) cout << "Current entry is " << i << endl;

        entry_offset = start_offset + (i * 0x1C);

        lbrt_file.seekg(entry_offset + 0x00);
        lbrt_file.read((char*)(&event_id), sizeof(uint32_t));
        if (debug) cout << "Current segment: " << event_id << endl;

        lbrt_file.seekg(entry_offset + 0x04);
        lbrt_file.read((char*)(&event_time_val_0), sizeof(uint32_t));
        if (debug) cout << "Current delta time: " << event_time_val_0 << "ms" << endl;

        lbrt_file.seekg(entry_offset + 0x08);
        lbrt_file.read((char*)(&event_time_val_1), sizeof(uint32_t));
        if (debug) cout << "Current time value: " << event_time_val_1 << endl;

        lbrt_file.seekg(entry_offset + 0x0C);
        lbrt_file.read((char*)(&event_chan_val), sizeof(uint32_t));
        if (debug) cout << "Current channel value: " << event_chan_val << endl;

        lbrt_file.seekg(entry_offset + 0x12);
        lbrt_file.read((char*)(&event_type), sizeof(uint16_t));
        if (debug) cout << "Current base event type: " << event_type << endl;

        lbrt_file.seekg(entry_offset + 0x14);
        lbrt_file.read((char*)(&event_val_0), sizeof(uint16_t));
        if (debug) cout << "Current event value 1: " << event_val_0 << endl;

        lbrt_file.seekg(entry_offset + 0x16);
        lbrt_file.read((char*)(&event_val_1), sizeof(uint16_t));
        if (debug) cout << "Current event value 2: " << event_val_1 << endl;

        lbrt_file.seekg(entry_offset + 0x18);
        lbrt_file.read((char*)(&event_status), sizeof(uint8_t));
        if (debug) cout << "Current event status: " << int(event_status) << endl;

        lbrt_file.seekg(entry_offset + 0x19);
        lbrt_file.read((char*)(&status_byte), sizeof(uint8_t));
        if (debug) cout << "Current status byte: 0x" << hex << int(status_byte) << dec << endl;

        lbrt_file.seekg(entry_offset + 0x1A);
        lbrt_file.read((char*)(&event_default_val_0), sizeof(uint8_t));
        if (debug) cout << "Current default event value 1: " << int(event_default_val_0) << endl;

        lbrt_file.seekg(entry_offset + 0x1B);
        lbrt_file.read((char*)(&event_default_val_1), sizeof(uint8_t));
        if (debug) cout << "Current default event value 2: " << int(event_default_val_1) << endl;


        //Add delta to absolute time
        absolute_time += event_time_val_0;

        //Next sequence chunk has been reached
        if (curr_entry < change_ids.size() && i == change_ids[curr_entry]) {
            if (debug) cout << "REACHED QUARTER NOTE" << endl;
            curr_entry += 1;
        }

        if ((status_byte & 0xF0) == 0xB0) {                     // Controller event
            if (event_type == 0x63) {                           // Loop event
                isLooped = true;
                uint8_t first_byte, second_byte;
                string meta;
                if (debug) cout << "LOOP EVENT" << endl;

                //Sequence uses CC 99 for looping
                //Will use CC 116/117 instead
                //Additionally use Final Fantasy style just because
                if (event_chan_val == 0x14) {                   // Start loop
                    //Use CC 116 as loop start (EMIDI/XMI style)
                    //0x00 is infinite loop, 0x01-0xFF is finite looping
                    first_byte = 0x74;
                    second_byte = 0x00;
                    meta = "loopStart";
                }
                else if (event_chan_val == 0x1E) {              // End loop
                    //Use CC 117 as loop end (EMIDI/XMI style)
                    //Always 0x7F
                    first_byte = 0x75;
                    second_byte = 0x7F;
                    meta = "loopEnd";
                }

                lbrtSequence[event_status].push_back(lbrt_status(event_id, absolute_time,
                                                                 status_byte, first_byte,
                                                                 vector<uchar>{second_byte}));

                lbrtSequence[event_status].push_back(lbrt_status(event_id, absolute_time,
                                                                 0xFF, 0x06,
                                                                 vector<uchar>{meta.begin(), meta.end()}));
            }
            else {                                              // Any controller event
                lbrtSequence[event_status].push_back(lbrt_status(event_id, absolute_time,
                                                                 status_byte, event_type,
                                                                 vector<uchar>{event_chan_val}));
            }
        }
        else if ((status_byte & 0xF0) == 0xF0) {                // Meta event
            if (event_type == 0x2F) {                           // End of track
                if (debug) cout << "END OF TRACK" << endl;

                //Add End of track to all used tracks
                for (int c = 0; c < 16; ++c) {
                    if (lbrtSequence[c].empty()) continue;

                    num_tracks += 1;
                    lbrtSequence[c].push_back(lbrt_status(event_id, absolute_time,
                                                          status_byte, event_type,
                                                          vector<uchar>{}));
                }

                break;
            }
            else if (event_type == 0x51) {                      // Tempo change
                if (debug) cout << "TEMPO CHANGE" << endl;
                lbrtSequence[0].push_back(lbrt_status(event_id, absolute_time,
                                                      status_byte, event_type,
                                                      vector<uchar>{(event_time_val_1 >> 0x10) & 0xFF,
                                                                    (event_time_val_1 >> 0x08) & 0xFF,
                                                                    (event_time_val_1 >> 0x00) & 0xFF}));
            }
        }
        else if ((status_byte & 0xF0) == 0x90) {                // Note on/off event
            if (debug) cout << "NOTE EVENT" << endl;
            if (debug) cout << "\tCurrent entry offset: 0x" << hex << entry_offset << dec << endl;
            if (debug) cout << "\tCurrent program ID: " << event_type << endl;
            if (debug) cout << "\tCurrent channel ID: " << int(event_status) << endl;
            if (debug) cout << "\tCurrent pitch: " << event_chan_val << endl;
            if (debug) cout << "\tCurrent pitch bend: 0x" << hex
                            << std::setfill('0') << std::right << std::setw(4)
                            << event_val_1
                            << std::setfill(' ') << std::left << dec << endl;
            if (debug) cout << "\tReleased after: " << event_time_val_1 << "ms" << endl;

            //Assign program to channel
            lbrtSequence[event_status].push_back(lbrt_status(event_id, absolute_time,
                                                            (0xC0 | event_status), event_type,
                                                             vector<uchar>{}));

            //Assign pitch bend to channel if applicable
            //Pitch bend value is 14-bit number (Least 7 bits per byte)
            if (event_val_1 != 0x40) isBent[event_status] = true;
            if (isBent[event_status]) {
                lbrtSequence[event_status].push_back(lbrt_status(event_id, absolute_time + event_time_val_1,
                                                                (0xE0 | event_status), (event_val_1 >> 0x08) & 0xFF,
                                                                 vector<uchar>{event_val_1 & 0xFF}));

                if (event_val_1 == 0x40) isBent[event_status] = false;
            }


            //Play note
            //Value of zero also releases note
            lbrtSequence[event_status].push_back(lbrt_status(event_id, absolute_time,
                                                             status_byte, event_chan_val,
                                                             vector<uchar>{event_val_0}));

            //Release note after specified time
            //Sort through events later
            lbrtSequence[event_status].push_back(lbrt_status(event_id, absolute_time + event_time_val_1,
                                                             status_byte, event_chan_val,
                                                             vector<uchar>{0x00}));
        }
        else {                                                  // Running status?
            if (debug) cout << "RUNNING STATUS" << endl;        // Technically a marker
            //Theoretically shouldn't do anything
            //Only occurs at quarter note mark
        }
    }
    lbrt_file.close();
    change_ids.clear();

    if (debug) cout << "The total number of tracks is " << num_tracks << endl;

    //Reorganize all tracks by absolute time and edit end time
    //At this point, the quarter note id's no longer matter
    //Do NOT mess with the terminator
    for (auto &events : lbrtSequence) {
        if (events.empty()) continue;
        std::sort(events.begin(), events.end() - 1);

        //Change all times over final time to final time if looped
        //Otherwise, change end time to previous event's time
        uint32_t fin_absol_t = events.back().absol_t;
        if (isLooped) for (auto &e : events) if (e.absol_t > fin_absol_t) e.absol_t = fin_absol_t;
        else events.back().absol_t = events[events.size() - 2].absol_t;
    }

    if (debug) if (isLooped) cout << "This sequence is looped" << endl;
    if (debug) cout << endl;

    string mid_file = lrt_file.substr(0, lrt_file.find_last_of('.') + 1) + "mid";
    if (debug) cout << "Path of new MID file is " << mid_file << endl;
    if (debug) cout << endl;

    int curr_track = 0;

    ofstream mid(mid_file, ios::out | ios::binary);
    try {
        mid.seekp(0x00);
        mid << "MThd";              //0x00
        getChar(mid, 0x06, 4);      //0x04
        getChar(mid, mid_format, 2);//0x08
        getChar(mid, num_tracks, 2);//0x0A
        getChar(mid, ppqn, 2);      //0x0C

        for (auto &events : lbrtSequence) {
            if (events.empty()) continue;

            uint32_t trackSize = 0x00;
            int prev_time = 0;

            mid << "MTrk";
            getChar(mid, trackSize, 4);
            uint32_t eventStart = mid.tellp();

            for (auto &event : events) {
                uint32_t temp_delta = event.absol_t - prev_time;

                //Put event to file
                getDeltaTime(mid, temp_delta);
                mid << event.status_a;
                mid << event.status_b;
                if ((event.status_a & 0xF0) == 0xF0) mid << uchar(event.values.size());
                for (auto &val : event.values) mid << val;
                prev_time = event.absol_t;
            }

            trackSize = mid.tellp();
            if (debug) cout << "End of track " << ++curr_track << ": 0x" << hex << trackSize << dec << endl;
            trackSize -= eventStart;
            if (debug) cout << "Size of track " << curr_track << ": 0x" << hex << trackSize << dec << endl;

            mid.seekp(eventStart - 0x04);
            getChar(mid, trackSize, 4);

            mid.seekp(eventStart + trackSize);
        }

        isSuccess = true;
    }
    catch (const fstream::failure & e) {
        cerr << e.what() << endl;
    }
    catch (const std::exception & e) {
        cerr << e.what() << endl;
    }
    mid.close();

    if (debug) {
        curr_track = 0;

        string csv_file = lrt_file.substr(0, lrt_file.find_last_of('.') + 1) + "csv";
        cout << "Path of new CSV file is " << csv_file << endl;
        cout << endl;

        ofstream csv(csv_file, ios::out);
        try {
            csv.seekp(0x00);
            csv << "0, 0, HEADER, " << mid_format
                << ", " << num_tracks
                << ", " << ppqn << '\n';

            for (auto &events : lbrtSequence) {
                if (events.empty()) continue;

                csv << ++curr_track << ", 0, START_TRACK\n";
                for (auto &event : events) {
                    uint8_t statA = event.status_a,
                            statB = event.status_b;

                    csv << curr_track << ", " << event.absol_t << ", ";
                    if ((statA & 0xF0) == 0x90) {
                        csv << "NOTE_ON_C, " << int(event.status_a & 0x0F)
                            << ", " << int(statB)
                            << ", " << int(event.values[0]);
                    }

                    else if ((statA & 0xF0) == 0xB0) {
                        csv << "CONTROL_C, " << int(event.status_a & 0x0F)
                            << ", " << int(statB)
                            << ", " << int(event.values[0]);
                    }

                    else if ((statA & 0xF0) == 0xC0) {
                        csv << "PROGRAM_C, " << int(event.status_a & 0x0F)
                            << ", " << int(statB);
                    }

                    else if ((statA & 0xF0) == 0xE0) {
                        uint16_t pb = uint16_t(event.values[0] & 0x7F) << 0x07 |
                                      uint16_t(event.status_b & 0x7F);

                        csv << "PITCH_BEND_C, " << int(event.status_a & 0x0F)
                            << ", " << pb;
                    }

                    else if (statA == 0xFF) {
                        if (statB == 0x03) {
                            csv << "TITLE_T, \""
                                << string(event.values.begin(), event.values.end())
                                << "\"";
                        }

                        else if (statB == 0x06) {
                            csv << "MARKER_T, \""
                                << string(event.values.begin(), event.values.end())
                                << "\"";
                        }

                        else if (statB == 0x2F) {
                            csv << "END_TRACK";
                        }

                        else if (statB == 0x51) {
                            uint32_t ms = 0x00;
                            for (auto &t : event.values) { ms <<= 8; ms |= t; }
                            csv << "TEMPO, " << ms;
                        }

                        else if (statB == 0x58) {
                            csv << "TIME_SIGNATURE";
                            for (auto &s : event.values) { csv << ", " << int(s); }
                        }
                    }
                    csv << '\n';
                }
            }

            csv << "0, 0, END_OF_FILE\n";
        }
        catch (const fstream::failure & e) {
            cerr << e.what() << endl;
        }
        catch (const std::exception & e) {
            cerr << e.what() << endl;
        }
        csv.close();
    }

    lbrtSequence.clear();

    return isSuccess;
}


int main(int argc, char *argv[]) {
    string prgm = argv[0];
    prgm.erase(std::remove(prgm.begin(), prgm.end(), '\"'), prgm.end());
    prgm = prgm.substr(prgm.find_last_of("\\/") + 1, prgm.find_last_of('.'));

    if (argc < 2) {
        printOpt(prgm);
        return 0;
    }
    else {
        string tempFile = "";

        for (int i = 1; i < argc; ++i) {
            tempFile = argv[i];
            tempFile.erase(std::remove(tempFile.begin(), tempFile.end(), '\"'), tempFile.end());
            if (debug) cout << endl;

            if (tempFile == "-h") {
                printOpt(prgm);
                return 0;
            }
            else if (tempFile == "-d") {
                debug = !debug;
                continue;
            }
            else if (tempFile == "-p") {
                play_result = !play_result;
                continue;
            }


            if (!(tempFile.find_last_of("sgd") == string::npos) && !setInput("sgd", tempFile)) {
                if (!setSF2(tempFile, debug, true)) cerr << "Unable to set SF2" << endl;
                else cout << "SF2 file was successfully saved" << endl;
                continue;
            }
            else if (!(tempFile.find_last_of("lrt") == string::npos) && !setInput("lrt", tempFile)) {
                if (!writeLRT(tempFile)) cerr << "Unable to convert LRT file to MID file" << endl;
                else cout << "MID file was successfully saved" << endl;
                continue;
            }
            else continue;
        }
    }

    return 0;
}
