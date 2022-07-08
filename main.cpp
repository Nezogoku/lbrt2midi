#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
//#include "fluidsynth.h"
#include "sgd/sgd.hpp"
#include "midicsv/csvmidi.h"
#include "sf2cute.hpp"

// SF2cute library from gocha
// Based off of codes in lbrt from owocek
//      Used for comparison stuffs
// Base codes in csvmidi from John Walker
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
using std::to_string;
using std::vector;

using namespace sf2cute;


bool debug = false,
     play_result = false;

int setSF2(string sgd_file) {
    string title, sf2_path;
    int numvag,
        numdef;

    sgd pataSGD(debug);

    pataSGD.setFile(sgd_file);
    if (debug) cout << "Set SGD file" << endl;
    pataSGD.setInfo();
    if (debug) cout << "Set SGD info" << endl;

    title = pataSGD.getBankName();
    numvag = pataSGD.getAmountAudio();
    numdef = pataSGD.getAmountDefinitions();

    SoundFont sf2;
    sf2.set_sound_engine("E-mu 10K1");
    sf2.set_bank_name(title);

    if (debug) cout << "Soundfont sound engine: " << sf2.sound_engine() << endl;
    if (debug) cout << "Soundfont bank name: " << sf2.bank_name() << endl;
    if (debug) cout << endl;


    vector<std::shared_ptr<SFSample>> samples;
    vector<SFInstrumentZone> instrument_zones;
    vector<std::shared_ptr<SFInstrument>> instruments;
    vector<SFPresetZone> preset_zones;
    vector<string> preset_zone_names;

    for (int i = 0; i < numvag; ++i) {
        //Collect all data from an audio sample in sgd file
        vector<int16_t> wav_data = pataSGD.getPCM(i);

        if (wav_data.empty()) {
            samples.push_back(SoundFont().NewSample());
            instruments.push_back(SoundFont().NewInstrument());
            preset_zones.push_back(SFPresetZone());
            preset_zone_names.push_back("NULL");
            continue;
        }


        if (debug) cout << "Collected data from sgd file" << endl;
        string sampName = pataSGD.getSampleName(i);
        uint32_t loopStart = pataSGD.getSampleLpStrt(i),
                 loopEnd = pataSGD.getSampleLpTrm(i),
                 sampleRate = pataSGD.getSampleSamplerate(i);
        uint8_t root = 60;
        int8_t correction = 0;

        //Adding sample to collection
        if (debug) cout << "Adding " << sampName << " to soundfont" << endl;
        samples.push_back(sf2.NewSample(sampName,       // Name
                                        wav_data,       // Sample data
                                        loopStart,      // Loop start
                                        loopEnd,        // Loop end
                                        sampleRate,     // Sample rate
                                        root,           // Root key
                                        correction));   // Microtuning

        vector<int16_t> ().swap(wav_data);
        if (debug) cout << endl;
    }

    if (debug) cout << endl;

    for (int i = 0; i < numdef; ++i) {
        string instrName = "instr_" + to_string(i);
        int sampID = pataSGD.getSampleID(i);
        uint8_t sampRoot = pataSGD.getSampleRoot(i),
                lowRange = pataSGD.getSampleLowRange(i),
                highRange = pataSGD.getSampleHighRange(i),
                sampSustain = pataSGD.getSampleVal5(i);
        uint16_t isLoop = (pataSGD.sampleIsLoop(sampID)) ?
                           uint16_t(SampleMode::kLoopContinuously) :
                           uint16_t(SampleMode::kNoLoop);

        vector<SFGeneratorItem> tempInstrZnGen {SFGeneratorItem(SFGenerator::kOverridingRootKey, sampRoot),
                                                SFGeneratorItem(SFGenerator::kKeyRange, RangesType(lowRange, highRange)),
                                                SFGeneratorItem(SFGenerator::kSustainVolEnv, sampSustain),
                                                SFGeneratorItem(SFGenerator::kSampleModes, isLoop)};

        //Making instrument zone
        if (debug) cout << "Adding " << instrName << " to soundfont" << endl;
        instrument_zones.push_back(SFInstrumentZone(samples[sampID], tempInstrZnGen,
                                                    vector<SFModulatorItem> {}));

        //Adding instrument to soundfont
        instruments.push_back(sf2.NewInstrument(instrName,
                                                vector<SFInstrumentZone> {instrument_zones[i]}));


        string prgm_name = "prgm_";
        int bank = pataSGD.getSampleBank(i),
            preset = pataSGD.getSamplePreset(i);

        prgm_name += to_string(bank);
        prgm_name += '.';
        prgm_name += to_string(preset);

        vector<SFGeneratorItem> tempPrstZnGen {SFGeneratorItem(SFGenerator::kKeyRange, RangesType(lowRange, highRange))};

        //Adding preset zone to soundfont
        if (debug) cout << "Adding " << prgm_name << " zone to soundfont" << endl;
        preset_zone_names.push_back(prgm_name);
        preset_zones.push_back(SFPresetZone(instruments[i], tempPrstZnGen,
                                            vector<SFModulatorItem> {}));

        vector<SFGeneratorItem> ().swap(tempInstrZnGen);
        vector<SFGeneratorItem> ().swap(tempPrstZnGen);
        if (debug) cout << endl;
    }


    //Building soundfont
    vector<vector<vector<SFPresetZone>>> prgms(128, vector<vector<SFPresetZone>> (128));
    vector<vector<string>> prgm_names(128, vector<string> (128, "NULL"));

    for (int v = 0; v < numdef; ++v) {
        if (preset_zone_names[v] == "NULL") continue;

        int bank = pataSGD.getSampleBank(v),
            preset = pataSGD.getSamplePreset(v);

        prgms[bank][preset].push_back(preset_zones[v]);
        prgm_names[bank][preset] = preset_zone_names[v];

        //Apparently, Channel 9 is researved for percussion or whatever,
        //so just make a copy of whatever is in preset 9
        if (preset == 9) {
            prgms[127][0].push_back(preset_zones[v]);
            prgm_names[127][0] = preset_zone_names[v] + "_copy";
        }
    }

    for (int b = 0; b < prgms.size(); ++b) {
        for (int p = 0; p < prgms.size(); ++p) {
            if (prgm_names[b][p] == "NULL") continue;

            std::shared_ptr<SFPreset> finalPreset = sf2.NewPreset(prgm_names[b][p], p, b,
                                                                  vector<SFPresetZone> {prgms[b][p]});
        }
    }

    //Writing soundfont file
    title += ".sf2";

    sf2_path = sgd_file.substr(0, sgd_file.find_last_of("\\/") + 1);
    sf2_path += title;
    if (debug) cout << "SF2 file: " << title << endl;

    try {
        ofstream ofs(sf2_path, ios::binary);
        sf2.Write(ofs);
    }
    catch (const fstream::failure & e) {
        cerr << "Unable to save to " << title << ":\n\t" << e.what() << endl;
        return 0;
    }
    catch (const std::exception & e) {
        cerr << "Unable to save to " << title << ":\n\t" << e.what() << endl;
        return 0;
    }

    return 1;
}

int getLRT(string &lrt_file, vector<string> &csv_final) {
    vector<string> csv_header(5),
                   csv_event;

    string tempCSV, title;
    bool hasLoop = false;
    vector<uint8_t> used_channel(16, 0xFF);
    vector<uint32_t> change_ids;

    //LRT processing thing I understand none of
    title = lrt_file.substr(lrt_file.find_last_of("\\/") + 1);

    if (debug) cout << "Name of LRT file is " << title << endl;

    ifstream lbrt_file(lrt_file, ios::binary);
    if (!lbrt_file.is_open()) {
        if (debug) cout << "Unable to open " << title << endl;
        return 0;
    }

    title = title.substr(0, title.find_last_of('.'));
    if (debug) cout << "Title of song is " << title << endl;

    uint32_t start_offset,
             notes,
             speed,
             tracks,
             entries,
             num_changes,
             current_track,
             entry_id;

    lbrt_file.seekg(0x04);
    lbrt_file.read(reinterpret_cast<char*>(&start_offset), sizeof(uint32_t));
    if (debug) cout << "\nStart offset is 0x" << hex << start_offset << dec << endl;

    lbrt_file.seekg(0x08);
    lbrt_file.read(reinterpret_cast<char*>(&notes), sizeof(uint32_t));
    if (debug) cout << "The note type being used is " << notes/4 << "th" << endl;

    lbrt_file.seekg(0x0C);
    lbrt_file.read(reinterpret_cast<char*>(&speed), sizeof(uint32_t));
    if (debug) cout << "The speed of the current sequence is " << speed << endl;

    lbrt_file.seekg(0x10);
    lbrt_file.read(reinterpret_cast<char*>(&tracks), sizeof(uint32_t));
    if (debug) cout << "The number of tracks is " << tracks << endl;

    lbrt_file.seekg(0x1C);
    lbrt_file.read(reinterpret_cast<char*>(&entries), sizeof(uint32_t));
    if (debug) cout << "The number of entries is " << entries << endl;

    lbrt_file.seekg(0x20);
    lbrt_file.read(reinterpret_cast<char*>(&num_changes), sizeof(uint32_t));
    if (debug) cout << "The number of segment changes is " << num_changes << endl;
    for (int in = 0; in < num_changes; ++in) {
        lbrt_file.seekg(0x24 + (0x04 * in));
        lbrt_file.read(reinterpret_cast<char*>(&entry_id), sizeof(uint32_t));

        if (entry_id == 0x00 && in == 0) continue;

        change_ids.push_back(entry_id);
    }

    for (const auto ch_ID : change_ids) if (debug) cout << "\tEntry of next segment is " << ch_ID << endl;

    uint32_t entry_offset,
             absolutetime,
             event_id,
             deltatime,
             event_val_tempo;
    uint16_t event_true_val,
             event_type,
             event_val_0,
             event_val_1;
    uint8_t  event_val_X,
             status_byte,
             event_default_val_0,
             event_default_val_1;

    absolutetime = 0;
    current_track = 0;
    int curr_entry = 0;
    vector<vector<int>> channels (16);

    for(uint32_t i = 0; i < entries; ++i) {
        uint32_t real_sample, rgnd_valueA, rgnd_valueB, pitch, pan;
        float factor, lpos, rpos;

        if (debug) cout << endl;
        if (debug) cout << "Current entry is " << i << endl;

        entry_offset = start_offset + (i * 0x1C);

        lbrt_file.seekg(entry_offset + 0x00);
        lbrt_file.read(reinterpret_cast<char*>(&event_id), sizeof(uint32_t));
        if (debug) cout << "Current segment: " << event_id << endl;

        lbrt_file.seekg(entry_offset + 0x04);
        lbrt_file.read(reinterpret_cast<char*>(&deltatime), sizeof(uint32_t));
        if (debug) cout << "Current delta time: " << deltatime << "ms" << endl;

        lbrt_file.seekg(entry_offset + 0x08);
        lbrt_file.read(reinterpret_cast<char*>(&event_val_tempo), sizeof(uint32_t));
        if (debug) cout << "Current tempo value: " << event_val_tempo << endl;

        lbrt_file.seekg(entry_offset + 0x0C);
        lbrt_file.read(reinterpret_cast<char*>(&event_true_val), sizeof(uint16_t));
        if (debug) cout << "Current true event value: " << event_true_val << endl;

        lbrt_file.seekg(entry_offset + 0x12);
        lbrt_file.read(reinterpret_cast<char*>(&event_type), sizeof(uint16_t));
        if (debug) cout << "Current base event type: " << event_type << endl;

        lbrt_file.seekg(entry_offset + 0x14);
        lbrt_file.read(reinterpret_cast<char*>(&event_val_0), sizeof(uint16_t));
        if (debug) cout << "Current event value 1: " << event_val_0 << endl;

        lbrt_file.seekg(entry_offset + 0x16);
        lbrt_file.read(reinterpret_cast<char*>(&event_val_1), sizeof(uint16_t));
        if (debug) cout << "Current event value 2: " << event_val_1 << endl;

        lbrt_file.seekg(entry_offset + 0x18);
        lbrt_file.read(reinterpret_cast<char*>(&event_val_X), sizeof(uint8_t));
        if (debug) cout << "Current event value X: " << int(event_val_X) << endl;

        lbrt_file.seekg(entry_offset + 0x19);
        lbrt_file.read(reinterpret_cast<char*>(&status_byte), sizeof(uint8_t));
        if (debug) cout << "Current status byte: 0x" << hex << int(status_byte) << dec << endl;

        lbrt_file.seekg(entry_offset + 0x1A);
        lbrt_file.read(reinterpret_cast<char*>(&event_default_val_0), sizeof(uint8_t));
        if (debug) cout << "Current default event value 1: " << int(event_default_val_0) << endl;

        lbrt_file.seekg(entry_offset + 0x1B);
        lbrt_file.read(reinterpret_cast<char*>(&event_default_val_1), sizeof(uint8_t));
        if (debug) cout << "Current default event value 2: " << int(event_default_val_1) << endl;


        absolutetime += deltatime;
        tempCSV = to_string(current_track) + ", " + to_string(absolutetime);


        if (curr_entry < change_ids.size() && i == change_ids[curr_entry]) {
            for (int cur_chn = 0; cur_chn < channels.size(); ++cur_chn) {
                for (auto cur_note : channels[cur_chn]) {
                    csv_event.push_back(to_string(current_track) + ", " +
                                        to_string(absolutetime) + ", NOTE_ON_C, " +
                                        to_string(cur_chn) + ", " +
                                        to_string(cur_note) + ", 0");
                }

                vector<int> ().swap(channels[cur_chn]);
            }

            curr_entry += 1;

            if (debug) cout << "NEXT SEQUENCE CHUNK" << endl;
        }

        if (status_byte >= 0xB0 && status_byte <= 0xBF) {       // Controller event
            if (event_type == 0x63) {                           // Loop event
                if (debug) cout << "LOOP EVENT" << endl;
                if (event_true_val == 0x14) {                   // Start loop
                    tempCSV += ", MARKER_T, \"START LOOP\"";
                }

                else if (event_true_val == 0x1E) {              // End loop
                    tempCSV += ", MARKER_T, \"END LOOP\"";
                }

                csv_event.push_back(tempCSV);
            }
        }

        else if (status_byte == 0xFF) {                         // Meta event
            if (event_type == 0x2F) {                           // End of track
                if (debug) cout << "END OF TRACK" << endl;

                csv_event.push_back(to_string(current_track) + ", " + to_string(absolutetime) + ", END_TRACK");

                absolutetime = 0;
                csv_event.push_back("0, 0, END_OF_FILE");
                break;
            }

            if (event_id == 0x00) {
                csv_header[0] = to_string(current_track) + ", " + to_string(absolutetime) + ", HEADER, 1, " + to_string(tracks) + ", " + to_string(speed);
                current_track += 1;

                csv_header[1] = to_string(current_track) + ", " + to_string(absolutetime) + ", START_TRACK";
                csv_header[2] = to_string(current_track) + ", " + to_string(absolutetime) + ", TITLE_T, \"" + title + "\"";
                csv_header[3] = to_string(current_track) + ", " + to_string(absolutetime) + ", TIME_SIGNATURE, 4, 2, 24, 8";
                csv_header[4] = to_string(current_track) + ", " + to_string(absolutetime) + ", TEMPO, 500000";
            }

            if (event_type == 0x51) {                           // Tempo change
                if (debug) cout << "TEMPO CHANGE" << endl;
                csv_event.push_back(to_string(current_track) + ", " + to_string(absolutetime) + ", TEMPO, " + to_string(event_val_tempo));
                //I'm pretty sure it should stay at around 120bpm
            }
        }

        else if (status_byte >= 0x90 && status_byte <= 0x9F) {  // Note on/off event
            if (debug) cout << "Current entry offset: 0x" << hex << entry_offset << dec << endl;
            if (debug) cout << "Current channel ID: " << int(event_val_X) << endl;

            if (debug) cout << "Current pitch: 0x" << hex << pitch << dec << endl;
            if (debug) cout << "Current absolute time: " << absolutetime << "ms" << endl;
            if (debug) cout << "NOTE EVENT" << endl;

            //Take note of channel
            used_channel[event_val_X] = event_val_X;

            //Play note
            //Value of zero also releases note
            tempCSV += ", NOTE_ON_C, " + to_string(event_val_X) + ", "
                    + to_string(event_true_val) + ", " + to_string(event_val_0);
            csv_event.push_back(tempCSV);
        }

        else {                                                  // Running status
            if (debug) cout << "RUNNING STATUS" << endl;
        }
    }
    lbrt_file.close();

    if (debug) cout << endl;

    for (int i = 0; i < used_channel.size(); ++i) {
        string tempProCon = "1, 0, PROGRAM_C, " + to_string(used_channel[i]) + ", " + to_string(used_channel[i]);
        if (debug && used_channel[i] != 0xFF) cout << "Channel " << int(used_channel[i]) << " is used" << endl;

        if (used_channel[i] != 0xFF) csv_header.push_back(tempProCon);
    }
    if (debug) cout << endl;

    csv_final.insert(csv_final.end(), csv_header.begin(), csv_header.end());
    csv_final.insert(csv_final.end(), csv_event.begin(), csv_event.end());


    lrt_file = lrt_file.substr(0, lrt_file.find_last_of('.') + 1) + "csv";
    title = lrt_file.substr(lrt_file.find_last_of("\\/") + 1);
    if (debug) cout << "Name of CSV file is " << title << endl;
    if (debug) cout << endl;

    ofstream csv_file(lrt_file);

    if (!csv_file.is_open()) {
        cout << "Unable to save to " << title;
        return 0;
    }

    for (const auto &line : csv_final) {
        csv_file << line << '\n';
    }
    csv_file.close();

    vector<string> ().swap(csv_header);
    vector<string> ().swap(csv_event);
    vector<uint8_t> ().swap(used_channel);
    vector<uint32_t> ().swap(change_ids);

    return 1;
}

int setMID(string csv_file) {
    string mid_file;

    mid_file = csv_file.substr(0, csv_file.find_last_of('.') + 1);
    mid_file += "mid";

    if (debug) cout << "Name of MID file is " << mid_file.substr(mid_file.find_last_of("\\/") + 1) << endl;

    return csvmidi(const_cast<char*>(csv_file.c_str()), const_cast<char*>(mid_file.c_str()));
}


void printOpt(string pName) {
    if (debug) cerr << "Not enough valid files entered\n" << endl;
    cout << "Usage: " << pName << " [options] <infile.sgd> [<infile(s).lrt>]\n"
         << "\nOptions:\n"
         << "\t-h       Prints this message thing\n"
         << "\t-d       Activates debug mode\n"
         << "\t-p       Activates playback mode\n"
         << endl;
}

int setInput(string ext, string &temp) {
    temp.erase(remove(temp.begin(), temp.end(), '\"'), temp.end());

    if (temp.substr(temp.find_last_of('.') + 1) != ext) {
        return 1;
    }
    else {
        if (debug) cout << ext << " file entered is " << temp << endl;
        return 0;
    }
}

int main(int argc, char *argv[]) {
    string prgm = argv[0];
    prgm.erase(remove(prgm.begin(), prgm.end(), '\"'), prgm.end());
    prgm = prgm.substr(prgm.find_last_of("\\/") + 1, prgm.find_last_of('.'));

    string tempFile;

    vector<string> lrtFile, eventList, outCSV;
    vector<int> sampleList;

    if (argc < 2) {
        printOpt(prgm);
        return 0;
    }

    else {
        for (int i = 1; i < argc; ++i) {
            if (string(argv[i]) == "-h") {
                printOpt(prgm);
                return 0;
            }
            else if (string(argv[i]) == "-d") {
                debug = !debug;
                continue;
            }
            else if (string(argv[i]) == "-p") {
                play_result = !play_result;
                continue;
            }

            tempFile = argv[i];
            if (!(tempFile.find_last_of("sgd") == string::npos) && !setInput("sgd", tempFile)) {
                if (setSF2(tempFile)) cout << "SF2 file was successfully saved" << endl;
                    continue;
                }
            else if (!(tempFile.find_last_of("lrt") == string::npos) && !setInput("lrt", tempFile)) {
                lrtFile.push_back(tempFile);
                continue;
            }
        }

        if (debug) cout << endl;
    }


    for (int i = 0; i < lrtFile.size(); ++i) {
        vector<string> ().swap(outCSV);

        if (debug) cout << endl;
        if (lrtFile[i].substr(lrtFile[i].find_last_of('.') + 1) != "lrt") continue;

        if (!getLRT(lrtFile[i], outCSV)) {
            cerr << "Unable to convert LRT file to CSV file" << endl;
            if (!debug) std::remove(lrtFile[i].c_str());
            continue;
        }

        cout << "CSV file was successfully saved" << endl;
        if (debug) cout << endl;

        if (setMID(lrtFile[i])) {
            cerr << "Unable to convert CSV file to MID file" << endl;
            if (!debug) std::remove(lrtFile[i].c_str());
            if (debug) cout << endl;
            if (debug) cout << endl;
            continue;
        }

        cout << "MID file was successfully saved" << endl;

        if (!debug) std::remove(lrtFile[i].c_str());
    }

    return 0;
}
