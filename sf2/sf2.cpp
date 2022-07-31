#include <cstdint>
#include <fstream>
#include <iostream>
#include <vector>
#include "sf2cute.hpp"
#include "../psx_sgd/psxsgd.hpp"
#include "sf2.hpp"

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


int setSF2(string sgd_file, bool isDebug, bool hasSeq) {
    string title, sf2_path, mid_path;
    int numvag,
        numdef,
        numseq;

    sgd pataSGD(isDebug);
    pataSGD.setFile(sgd_file);
    if (!pataSGD.setInfo()) return 0;

    title = pataSGD.getBankName();
    numvag = pataSGD.getAmountAudio();
    numdef = pataSGD.getAmountDefinitions();
    numseq = pataSGD.getAmountSequence();

    if (isDebug) cout << endl;
    if (isDebug) cout << "Number of samples: " << numvag << endl;
    if (isDebug) cout << "Number of programs: " << numdef << endl;
    if (isDebug) cout << "Number of sequences: " << numseq << endl;

    SoundFont sf2;
    sf2.set_sound_engine("E-mu 10K1");
    sf2.set_bank_name(title);

    if (isDebug) cout << endl;
    if (isDebug) cout << "Soundfont sound engine: " << sf2.sound_engine() << endl;
    if (isDebug) cout << "Soundfont bank name: " << sf2.bank_name() << endl;
    if (isDebug) cout << endl;


    vector<std::shared_ptr<SFSample>> samples;
    vector<SFInstrumentZone> instrument_zones;
    vector<std::shared_ptr<SFInstrument>> instruments;
    vector<SFPresetZone> preset_zones;
    vector<string> preset_zone_names;

    for (int i = 0; i < numvag; ++i) {
        //Collect all data from an audio sample in sgd file
        vector<int16_t> wav_data = pataSGD.getSample(i);

        if (wav_data.empty()) {
            if (isDebug) cout << "Sample " << i << " is empty" << endl;
            if (isDebug) cout << endl;
            samples.push_back(SoundFont().NewSample());
            //instruments.push_back(SoundFont().NewInstrument());
            //preset_zones.push_back(SFPresetZone());
            //preset_zone_names.push_back("NULL"+to_string(i));
            continue;
        }


        if (isDebug) cout << "Collected data from sgd file" << endl;
        string sampName = pataSGD.getSampleName(i);
        uint32_t loopStart = pataSGD.getSampleLpStrt(i),
                 loopEnd = pataSGD.getSampleLpTrm(i),
                 sampleRate = pataSGD.getSampleSamplerate(i);
        uint8_t root = 60;
        int8_t correction = 0;

        //Adding sample to collection
        if (isDebug) cout << "Adding " << sampName << " to soundfont" << endl;
        samples.push_back(sf2.NewSample(sampName,       // Name
                                        wav_data,       // Sample data
                                        loopStart,      // Loop start
                                        loopEnd,        // Loop end
                                        sampleRate,     // Sample rate
                                        root,           // Root key
                                        correction));   // Microtuning

        vector<int16_t> ().swap(wav_data);
        if (isDebug) cout << endl;
    }

    if (isDebug) cout << endl;
    for (int i = 0; i < numdef; ++i) {
        string instrName = "instr_" + to_string(i);
        int sampID = pataSGD.getSampleID(i);
        uint8_t sampRoot = pataSGD.getSampleRoot(i),
                lowRange = pataSGD.getSampleLow(i),
                highRange = pataSGD.getSampleHigh(i);

        int16_t sampTune = pataSGD.getSampleTuning(i),
                sampHold = pataSGD.getSampleMod2(i)/* * 10*/,
                sampSustain = pataSGD.getSampleMod3(i)/* * 10*/,
                sampRelease = pataSGD.getSampleMod1(i)/* * 10*/,
                sampPan = pataSGD.getSamplePan(i);

        uint16_t isLoop = (pataSGD.sampleIsLoop(sampID)) ?
                           uint16_t(SampleMode::kLoopContinuously) :
                           uint16_t(SampleMode::kNoLoop);

        vector<SFGeneratorItem> tempInstrZnGen {SFGeneratorItem(SFGenerator::kOverridingRootKey, sampRoot),
                                                SFGeneratorItem(SFGenerator::kKeyRange, RangesType(lowRange, highRange)),
                                                SFGeneratorItem(SFGenerator::kFineTune, sampTune),
                                                SFGeneratorItem(SFGenerator::kHoldVolEnv, (sampHold & 0xFFFF)),
                                                SFGeneratorItem(SFGenerator::kSustainVolEnv, (sampSustain & 0xFFFF)),
                                                SFGeneratorItem(SFGenerator::kReleaseVolEnv, (sampRelease & 0xFFFF)),
                                                SFGeneratorItem(SFGenerator::kPan, sampPan * 160),
                                                SFGeneratorItem(SFGenerator::kSampleModes, isLoop)};

        //Making instrument zone
        if (isDebug) cout << "Adding " << instrName << " zone to soundfont" << endl;
        instrument_zones.push_back(SFInstrumentZone(samples[sampID], tempInstrZnGen,
                                                    vector<SFModulatorItem> {}));

        //Adding instrument to soundfont
        if (isDebug) cout << "Adding " << instrName << " to soundfont" << endl;
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
        if (isDebug) cout << "Adding " << prgm_name << " zone to soundfont" << endl;
        preset_zone_names.push_back(prgm_name);
        preset_zones.push_back(SFPresetZone(instruments[i], tempPrstZnGen,
                                            vector<SFModulatorItem> {}));

        vector<SFGeneratorItem> ().swap(tempInstrZnGen);
        vector<SFGeneratorItem> ().swap(tempPrstZnGen);
        if (isDebug) cout << endl;
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
    mid_path = sf2_path;
    sf2_path += title;
    if (isDebug) cout << "SF2 file: " << title << endl;

    try {
        ofstream ofs(sf2_path, ios::binary);
        sf2.Write(ofs);
    }
    catch (const fstream::failure & e) {
        cerr << "Unable to save to " << title << ":\n    " << e.what() << endl;
        return 0;
    }
    catch (const std::exception & e) {
        cerr << "Unable to save to " << title << ":\n    " << e.what() << endl;
        return 0;
    }

    if (hasSeq) {
        for (int m = 0; m < numseq; ++m) {
            pataSGD.writeSequence(m);
        }
    }
    if (isDebug) cout << "Reached end of SF2 function" << endl;

    return 1;
}
