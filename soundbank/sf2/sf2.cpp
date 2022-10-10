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


int setSF2(std::string sgh_file, std::string sgb_file, std::string &sf2_file, bool isDebug) {
    sgd pataSGD(isDebug);
    pataSGD.setFile(sgh_file, sgb_file);

    sgh_file.replace(sgh_file.find_last_of('.'), 4, ".sgd");
    return setSF2(sgh_file, sf2_file, isDebug);
}

int setSF2(string sgd_file, std::string &sf2_file, bool isDebug) {
    bool success = false;

    string title, mid_path;
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
    vector<vector<SFInstrumentZone>> instrument_zones;
    vector<std::shared_ptr<SFInstrument>> instruments;
    vector<vector<vector<SFPresetZone>>> program_zones;

    vector<vector<vector<int>>> inst_correction;

    for (int i = 0; i < numvag; ++i) {
        //Collect all data from an audio sample in sgd file
        vector<int16_t> wav_data = pataSGD.getSample(i);

        if (isDebug) cout << "Collected data from sgd file" << endl;
        string sampName = pataSGD.getSampleName(i);
        uint32_t loopStart = pataSGD.getSampleLpStrt(i),
                 loopEnd = pataSGD.getSampleLpTrm(i),
                 sampleRate = pataSGD.getSampleSamplerate(i);
        uint8_t root = 60;
        int8_t correction = 0;

        //Adding sample to collection
        if (wav_data.empty()) {
            samples.push_back(nullptr);
        }
        else {
            if (isDebug) cout << "Adding " << sampName << " to soundfont" << endl;
            samples.push_back(sf2.NewSample(sampName,       // Name
                                            wav_data,       // Sample data
                                            loopStart,      // Loop start
                                            loopEnd,        // Loop end
                                            sampleRate,     // Sample rate
                                            root,           // Root key
                                            correction));   // Microtuning

            wav_data.clear();
        }

        if (isDebug) cout << endl;
    }
    if (isDebug) cout << endl;

    //Set instrument and program zones
    for (int i = 0; i < numdef; ++i) {
        int sampID = pataSGD.getSampleID(i),
            sampPrgm = i;
        uint8_t sampCodec = pataSGD.getSampleCodec(sampID),
                sampRoot = pataSGD.getSampleRoot(i),
                bank = pataSGD.getSampleBank(i),
                preset = pataSGD.getSamplePreset(i),
                lowRange = pataSGD.getSampleLow(i),
                highRange = pataSGD.getSampleHigh(i);

        int16_t sampTuneFine = pataSGD.getSampleTuningFine(i),
                sampTuneSemi = pataSGD.getSampleTuningSemi(i),
                //sampHold = pataSGD.getSampleMod2(i)/* * 10*/,
                //sampSustain = pataSGD.getSampleMod3(i)/* * 10*/,
                sampRelease = pataSGD.getSampleMod2(i)/* * 10*/,
                //sampReverb = pataSGD.getSampleMod3(i)/* * 10*/,
                sampPan = 50 * pataSGD.getSamplePanPercent(i);

        uint16_t isLoop = (pataSGD.sampleIsLoop(sampID)) ?
                           uint16_t(SampleMode::kLoopContinuously) :
                           uint16_t(SampleMode::kNoLoop);

        //Check if sample is empty
        if (samples[sampID] == nullptr) {
            if (isDebug) cout << "Sample " << sampID << " is empty" << endl;
            continue;
        }

        vector<SFGeneratorItem> tempInstrZnGen {SFGeneratorItem(SFGenerator::kOverridingRootKey, sampRoot),
                                                SFGeneratorItem(SFGenerator::kKeyRange, RangesType(lowRange, highRange)),
                                                SFGeneratorItem(SFGenerator::kFineTune, sampTuneFine),
                                                SFGeneratorItem(SFGenerator::kCoarseTune, sampTuneSemi),
                                                //SFGeneratorItem(SFGenerator::kHoldVolEnv, sampHold * 10),
                                                //SFGeneratorItem(SFGenerator::kSustainVolEnv, sampSustain * 10),
                                                SFGeneratorItem(SFGenerator::kReleaseVolEnv, sampRelease * 10),
                                                //SFGeneratorItem(SFGenerator::kReverbEffectsSend, sampReverb * 10),
                                                SFGeneratorItem(SFGenerator::kPan, sampPan),
                                                SFGeneratorItem(SFGenerator::kSampleModes, isLoop)};

        vector<SFGeneratorItem> tempPrstZnGen {SFGeneratorItem(SFGenerator::kKeyRange, RangesType(lowRange, highRange))};

        //Making instrument zone
        if (sampPrgm >= instrument_zones.size()) instrument_zones.resize(sampPrgm + 1);
        if (sampPrgm >= instruments.size()) instruments.resize(sampPrgm + 1);

        if (isDebug) cout << "Adding to instrument zone " << sampPrgm << endl;
        instrument_zones[sampPrgm].push_back(SFInstrumentZone(samples[sampID], tempInstrZnGen,
                                                              vector<SFModulatorItem> {}));

        //Making preset zone
        if (bank >= program_zones.size()) { program_zones.resize(bank + 1); inst_correction.resize(bank + 1); }
        if (preset >= program_zones[bank].size()) { program_zones[bank].resize(preset + 1); inst_correction[bank].resize(preset + 1); }

        if (isDebug) cout << "Adding to program zone " << int(bank) << '.' << int(preset) << endl;
        program_zones[bank][preset].push_back(SFPresetZone(instruments[sampPrgm], tempPrstZnGen,
                                                           vector<SFModulatorItem> {}));
        inst_correction[bank][preset].push_back(sampPrgm);

        tempInstrZnGen.clear();
        tempPrstZnGen.clear();
        if (isDebug) cout << endl;
    }
    if (isDebug) cout << endl;


    //Building soundfont
    for (int i = 0; i < instrument_zones.size(); ++i) {
        if (instrument_zones[i].empty()) continue;

        //Adding instrument and zones to soundfont
        string instrName = "0000";
        instrName += to_string(i);
        instrName = "instr_" + instrName.substr(instrName.size() - 4);

        if (isDebug) cout << "Adding " << instrName << " to soundfont" << endl;
        instruments[i] = sf2.NewInstrument(instrName,
                                           instrument_zones[i]);
    }
    if (isDebug) cout << endl;

    for (int b = 0; b < program_zones.size(); ++b) {
        for (int p = 0; p < program_zones[b].size(); ++p) {
            if (program_zones[b][p].empty()) continue;

            //Adding program and zones to soundfont
            string prgm_name = "prgm_";
            prgm_name += to_string(b);
            prgm_name += '.';
            prgm_name += to_string(p);

            if (isDebug) cout << "Adding program to bank " << b
                              << " patch " << p << endl;
            for (int z = 0; z < program_zones[b][p].size(); ++z) {
                program_zones[b][p][z].set_instrument(instruments[inst_correction[b][p][z]]);
            }
            std::shared_ptr<SFPreset> program = sf2.NewPreset(prgm_name, p, b,
                                                              program_zones[b][p]);
        }
    }
    if (isDebug) cout << endl;


    //Writing soundfont file
    title += ".sf2";
    sf2_file = sgd_file.substr(0, sgd_file.find_last_of("\\/") + 1) + title;

    if (isDebug) cout << "SF2 file: " << title << endl;

    try {
        ofstream ofs(sf2_file, ios::binary);
        sf2.Write(ofs);
        success = true;
    }
    catch (const fstream::failure & e) {
        cerr << "Unable to save to " << title << ":\n    " << e.what() << endl;
        //return 0;
    }
    catch (const std::exception & e) {
        cerr << "Something went wrong saving to " << title << ":\n    " << e.what() << endl;
        //return 0;
    }

    for (int m = 0; m < numseq; ++m) pataSGD.writeSequence(m);

    if (isDebug) cout << "\nReached end of SF2 function" << endl;

    return success;
}
