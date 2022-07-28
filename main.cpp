#include <algorithm>
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
// Based off of codes in lbrt from owocek
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

struct lbrt_status {
    int quarter_note_id;
    uint32_t delta_t;
    char status_a,
         status_b;
    vector<char> values;

    lbrt_status(int qnid, uint32_t ms, char stata, char statb, vector<char> val) :
        quarter_note_id(qnid), delta_t(ms), status_a(stata), status_b(statb), values(val) {}
};


void printOpt(string pName) {
    if (debug) cerr << "Not enough valid files entered\n" << endl;
    cout << "Usage: " << pName << " [options] [<infile.sgd>] <infile(s).lrt>\n"
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
        sequence << char(buffer);
        if (buffer & 0x80) buffer >>= 8;
        else break;
    }

    dTime = 0x00;
}

void getChar(ofstream &sequence, int seqEvent, int sizeOf) {
    for (int s = sizeOf - 1; s >= 0; --s) {
        char buff = ((seqEvent >> (s * 8)) & 0xFF);
        sequence << buff;
    }
}

int writeLRT(string &lrt_file) {
    if (debug) cout << std::setfill(' ') << std::left;

    string mid_file, title;
    vector<lbrt_status> lbrtSequence;
    vector<uint32_t> change_ids;

    title = lrt_file.substr(lrt_file.find_last_of("\\/") + 1);
    title = title.substr(0, title.find_first_of('.'));
    if (debug) cout << "Title of sequence is " << title << endl;

    ifstream lbrt_file(lrt_file, ios::binary);
    if (!lbrt_file.is_open()) {
        cerr << "Unable to open " << title << endl;
        return 0;
    }


    uint32_t start_offset,
             notes,
             ppqn,
             tracks,
             entries,
             num_changes,
             entry_id;

    lbrt_file.seekg(0x04);
    lbrt_file.read((char*)(&start_offset), sizeof(uint32_t));
    if (debug) cout << "\nStart offset is 0x" << hex << start_offset << dec << endl;

    lbrt_file.seekg(0x08);
    lbrt_file.read((char*)(&notes), sizeof(uint32_t));
    if (debug) cout << "The note type being used is " << notes << "th" << endl;

    lbrt_file.seekg(0x0C);
    lbrt_file.read((char*)(&ppqn), sizeof(uint32_t));
    if (debug) cout << "The PPQN of the current sequence is " << ppqn << endl;

    lbrt_file.seekg(0x10);
    lbrt_file.read((char*)(&tracks), sizeof(uint32_t));
    if (debug) cout << "The number of tracks is " << tracks << endl;

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
             deltatime,
             event_val_tempo;
    uint16_t event_note,
             event_type,
             event_val_0,
             event_val_1;
    uint8_t  event_status,
             status_byte,
             event_adjust_val_0,
             event_adjust_val_1;

    int curr_entry = 0,
        divi_id = 0;

    //Set sequence name
    lbrtSequence.push_back(lbrt_status(0, 0x00,
                                       0xFF, 0x03,
                                       vector<char>{title.begin(), title.end()}));
    //Set time signature
    lbrtSequence.push_back(lbrt_status(0, 0x00,
                                       0xFF, 0x58,
                                       vector<char>{0x04, 0x02, 0x18, 0x08}));
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
        lbrt_file.read((char*)(&deltatime), sizeof(uint32_t));
        if (debug) cout << "Current delta time: " << deltatime << "ms" << endl;

        lbrt_file.seekg(entry_offset + 0x08);
        lbrt_file.read((char*)(&event_val_tempo), sizeof(uint32_t));
        if (debug) cout << "Current tempo value: " << event_val_tempo << endl;

        lbrt_file.seekg(entry_offset + 0x0C);
        lbrt_file.read((char*)(&event_note), sizeof(uint16_t));
        if (debug) cout << "Current note value: " << event_note << endl;

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
        lbrt_file.read((char*)(&event_adjust_val_0), sizeof(uint8_t));
        if (debug) cout << "Current adjusted event value 1: " << int(event_adjust_val_0) << endl;

        lbrt_file.seekg(entry_offset + 0x1B);
        lbrt_file.read((char*)(&event_adjust_val_1), sizeof(uint8_t));
        if (debug) cout << "Current adjusted event value 2: " << int(event_adjust_val_1) << endl;


        //Next sequence chunk has been reached
        if (curr_entry < change_ids.size() && i == change_ids[curr_entry]) {
            if (debug) cout << "NEXT SEQUENCE CHUNK" << endl;

            //Only turn off notes if divi_id is 4
            //One quarter note after a previous measure
            if (divi_id == 4) {
                //Temporarily store played notes
                vector<lbrt_status> usedNotes;

                for (auto &event : lbrtSequence) {
                    if ((event.quarter_note_id == (event_id - 1)) &&
                       ((event.status_a & 0xF0) == 0x90) &&
                        (event.values[0] > 0x00)) {

                        usedNotes.push_back(event);
                        usedNotes[usedNotes.size() - 1].values[0] = 0x00;
                        usedNotes[usedNotes.size() - 1].delta_t = deltatime;
                        deltatime = 0x00;
                    }
                }

                while (usedNotes.size() > 0) {
                    char status = usedNotes[0].status_a,
                         note = usedNotes[0].status_b;
                    uint32_t d_time = usedNotes[0].delta_t;

                    if (debug) cout << "\tTurn off note " << std::setw(3) << int(note)
                                    << " on channel " << std::setw(2) << int(status & 0x0F)
                                    << " at delta time " << d_time << "ms" << endl;
                    lbrtSequence.push_back(usedNotes[0]);

                    //Remove all instances of silenced note
                    usedNotes.erase(std::remove_if(usedNotes.begin(), usedNotes.end(),
                                                   [&](const auto &event) {return (event.status_a == status) && (event.status_b == note);}),
                                                   usedNotes.end());
                }

                //Reset id counter to 0 for reasons
                divi_id = 0;
            }

            divi_id += 1;
            curr_entry += 1;
            //continue;
            //DO NOT SKIP!!! WHAT WAS I THINKING???!1!111111!
        }

        if ((status_byte & 0xF0) == 0xB0) {                     // Controller event
            if (event_type == 0x63) {                           // Loop event
                uint8_t second_byte;
                if (debug) cout << "LOOP EVENT" << endl;

                //Sequence uses CC 99 for looping
                //Will use CC 116/117 instead
                if (event_note == 0x14) {                       // Start loop
                    //Use CC 116 as loop start (EMIDI/XMI style)
                    //0x00 is infinite loop, 0x01-0xFF is finite looping
                    second_byte = 0x00;
                }
                else if (event_note == 0x1E) {                  // End loop
                    //Use CC 117 as loop end (EMIDI/XMI style)
                    //Always 0xFF
                    second_byte = 0xFF;
                }

                lbrtSequence.push_back(lbrt_status(event_id, deltatime,
                                                   status_byte, 0x74,
                                                   vector<char>{second_byte}));
            }
            else {                                              // Any controller event
                lbrtSequence.push_back(lbrt_status(event_id, deltatime,
                                                   status_byte, event_type,
                                                   vector<char>{event_note}));
            }
        }
        else if ((status_byte & 0xF0) == 0xF0) {                // Meta event
            if (event_type == 0x2F) {                           // End of track
                if (debug) cout << "END OF TRACK" << endl;
                //lrtSequence.push_back(0x00);
                lbrtSequence.push_back(lbrt_status(event_id, deltatime,
                                                   status_byte, event_type,
                                                   vector<char>{}));
                break;
            }
            else if (event_type == 0x51) {                      // Tempo change
                if (debug) cout << "TEMPO CHANGE" << endl;
                lbrtSequence.push_back(lbrt_status(event_id, deltatime,
                                                   status_byte, event_type,
                                                   vector<char>{(event_val_tempo >> 0x10) & 0xFF,
                                                                (event_val_tempo >> 0x08) & 0xFF,
                                                                (event_val_tempo >> 0x00) & 0xFF}));
            }
        }
        else if ((status_byte & 0xF0) == 0x90) {                // Note on/off event
            if (debug) cout << "NOTE EVENT" << endl;
            if (debug) cout << "\tCurrent entry offset: 0x" << hex << entry_offset << dec << endl;
            if (debug) cout << "\tCurrent channel ID: " << int(event_status) << endl;
            if (debug) cout << "\tCurrent pitch: " << event_note << endl;


            /*
            //Silence note if being played
            for (int i = lbrtSequence.size() - 1; i >= 0; --i) {
                if ((lbrtSequence[i].quarter_note_id == event_id) &&
                    (lbrtSequence[i].status_a == status_byte)) {

                    if (lbrtSequence[i].values[0] > 0x00) {
                        lbrtSequence.push_back(lbrtSequence[i]);
                        lbrtSequence[lbrtSequence.size() - 1].values[0] = 0x00;
                        lbrtSequence[lbrtSequence.size() - 1].delta_t = deltatime;
                        deltatime = 0x00;
                        break;
                    }
                    else break;
                }
            }
            */

            //Play note
            //Value of zero also releases note
            lbrtSequence.push_back(lbrt_status(event_id, deltatime,
                                               status_byte, event_note,
                                               vector<char>{event_adjust_val_1}));

            /*
            //Adjust pan
            lbrtSequence.push_back(lbrt_status(event_id, 0x00,
                                               0xB0 | event_status, 0x0A,
                                               vector<char>{event_val_1}));
            */
        }
        else {                                                  // Running status?
            if (debug) cout << "RUNNING STATUS" << endl;
            //Theoretically shouldn't do anything
            //Only occurs at quarter note mark
            lbrtSequence.push_back(lbrt_status(event_id, deltatime,
                                               0xFF, 0x07,
                                               vector<char>{}));
        }
    }
    lbrt_file.close();
    change_ids.clear();

    if (debug) cout << endl;

    mid_file = lrt_file.substr(0, lrt_file.find_last_of('.') + 1) + "mid";
    if (debug) cout << "Path of new MID file is " << mid_file << endl;
    if (debug) cout << endl;

    //Set up for specifying channels
    vector<bool> channelsUsed(16, false);
    for (auto &event : lbrtSequence) {
        if ((event.status_a & 0xF0) == 0x90) {
            if (channelsUsed[event.status_a & 0x0F]) continue;
            else channelsUsed[event.status_a & 0x0F] = true;
        }
    }

    bool isSuccess = false;
    uint32_t finalSize = 0x00;

    ofstream mid(mid_file, ios::out | ios::binary);
    try {
        mid.seekp(0x00);
        mid << "MThd";              //0x00
        getChar(mid, 0x06, 4);      //0x04
        getChar(mid, 0x01, 2);      //0x08
        getChar(mid, tracks, 2);    //0x0A
        getChar(mid, ppqn, 2);      //0x0C
        mid << "MTrk";              //0x0E
        getChar(mid, finalSize, 4); //0x12
                                    //0x16
        for (int channel = 0; channel < channelsUsed.size(); ++channel) {
            //Assign channel to program
            if (channelsUsed[channel]) {
                mid << char(0x00);
                mid << char(0xC0 | channel);
                mid << char(channel);
            }
        }
        for (auto &event : lbrtSequence) {
            //Put event to file
            getDeltaTime(mid, event.delta_t);
            mid << event.status_a;
            mid << event.status_b;
            if ((event.status_a & 0xF0) == 0xF0) mid << char(event.values.size());
            for (auto &val : event.values) mid << val;
        }

        finalSize = mid.tellp();
        if (debug) cout << "Size of file: 0x" << hex << finalSize << dec << endl;
        finalSize -= 0x16;
        if (debug) cout << "Size of sequence track: 0x" << hex << finalSize << dec << endl;

        mid.seekp(0x12);
        getChar(mid, finalSize, 4);
        isSuccess = true;
    }
    catch (const fstream::failure & e) {
        cerr << e.what() << endl;
    }
    catch (const std::exception & e) {
        cerr << e.what() << endl;
    }
    mid.close();
    channelsUsed.clear();
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
        vector<string> lrtFile;

        for (int i = 1; i < argc; ++i) {
            tempFile = argv[i];
            tempFile.erase(std::remove(tempFile.begin(), tempFile.end(), '\"'), tempFile.end());

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
                if (setSF2(tempFile, debug, true)) continue;
                else {
                    cerr << "Unable to set SF2" << endl;
                    continue;
                }
            }
            else if (!(tempFile.find_last_of("lrt") == string::npos) && !setInput("lrt", tempFile)) {
                lrtFile.push_back(tempFile);
                continue;
            }
            else continue;
        }
        if (debug) cout << endl;

        for (int i = 0; i < lrtFile.size(); ++i) {
            if (debug) cout << endl;
            if (!writeLRT(lrtFile[i])) {
                cerr << "Unable to convert LRT file to MID file" << endl;
                continue;
            }

            cout << "MID file was successfully saved" << endl;
            if (debug) cout << endl;
        }
    }

    return 0;
}
