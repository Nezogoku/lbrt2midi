#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <memory>
#include <fstream>
#include <string>
#include <vector>
#include "sf2cute.hpp"
#include "../psx_sgd/psxsgd.hpp"
#include "sf2.hpp"

using namespace sf2cute;


struct InstrumentZones {
    std::string name;
    int bank, preset;
    std::vector<SFInstrumentZone> zones;

    InstrumentZones(std::string n, int b, int p) : name(n), bank(b), preset(p) {}
    bool operator==(const std::string &nam) const { return name == nam; }
};


int setSF2(std::string &sgd_file, bool isDebug) {
    return setSF2(sgd_file, "", isDebug);
}

int setSF2(std::string &sgh_file, std::string sgb_file, bool isDebug) {
    bool success = false;

    std::string title;
    int numvag, numprg, numton, numseq;

    sgd pataSGD(isDebug);
    pataSGD.setFile(sgh_file, sgb_file);
    if (!pataSGD.setInfo()) return success;

    title = pataSGD.getBankName();
    numvag = pataSGD.getAmountAudio();
    numprg = pataSGD.getAmountPrograms();
    numton = pataSGD.getAmountTones();
    numseq = pataSGD.getAmountSequence();


    SoundFont sf2;
    //sf2.set_...meh
    sf2.set_sound_engine("E-mu 10K1");
    sf2.set_bank_name(title);

    fprintf(stdout, "\nSoundfont Bank Name: %s\n", sf2.bank_name().c_str());
    fprintf(stdout, "Soundfont Sound Engine: %s\n", sf2.sound_engine().c_str());
    fprintf(stdout, "Soundfont Samples: %i\n", numvag);
    fprintf(stdout, "Soundfont Tones: %i\n", numton);
    fprintf(stdout, "Soundfont Programs: %i\n", numprg);
    fprintf(stdout, "Embedded Sequences: %i\n", numseq);


    if (isDebug) fprintf(stdout, "\nCollect samples\n");

    std::vector<std::shared_ptr<SFSample>> samples(numvag, nullptr);
    for (int v = 0; v < numvag; ++v) {
        int v_size = pataSGD.getSampleSize(v);
        std::vector<int16_t> wav_data(v_size, 0);

        if (!v_size || !pataSGD.getSample(v, wav_data.data())) samples[v] = nullptr;
        else {
            std::string sampName = pataSGD.getSampleName(v);
            uint32_t loopStart = pataSGD.getSampleLpStrt(v),
                     loopEnd = pataSGD.getSampleLpTrm(v),
                     sampleRate = pataSGD.getSampleSamplerate(v);
            uint8_t root = 60;
            int8_t correction = 0;

            //Adding sample to collection
            if (isDebug) fprintf(stdout, "    Adding %s to soundfont\n", sampName.c_str());
            samples[v] = sf2.NewSample(sampName,        // Name
                                       wav_data,        // Sample data
                                       loopStart,       // Loop start
                                       loopEnd,         // Loop end
                                       sampleRate,      // Sample rate
                                       root,            // Root key
                                       correction);     // Microtuning
        }

        wav_data.clear();
    }

    //Add ROM-based sample until I figure out how the noise works
    if (true) {
        if (isDebug) fprintf(stdout, "    Adding ROM-based sample to soundfont\n");

        std::string n_noise = "NOISE";
        samples.push_back(sf2.NewSample(n_noise, std::vector<int16_t> {}, 0, 0, 44100, 60, 0));
    }


    if (isDebug) fprintf(stdout, "\nCollect programs\n");

    std::vector<InstrumentZones> tone_zones;
    std::vector<std::shared_ptr<SFInstrument>> tones;
    std::vector<std::shared_ptr<SFPreset>> programs;
    for (int t = 0, p = 0; t < numton && p < numprg; ++p) {
        int amnt_tone = pataSGD.getAmountProgramTones(p),
            strt_tone = tone_zones.size();

        if (isDebug) fprintf(stdout, "    Current program and tones: %d with %d tones\n", p, amnt_tone);
        for (int i = 0; i < amnt_tone; ++i, ++t) {
            uint8_t bank = pataSGD.getToneBank(p, i);
            uint8_t preset = p;
            uint8_t tone_id = pataSGD.getToneID(p, i);
            uint8_t lowRange = pataSGD.getToneLow(p, i);
            uint8_t highRange = pataSGD.getToneHigh(p, i);
            uint8_t rootKey = pataSGD.getToneRoot(p, i);
            int8_t tuneSemi = pataSGD.getToneTuneSemi(p, i);
            int8_t tuneFine = pataSGD.getToneTuneFine(p, i);
            int16_t m_attack = pataSGD.getToneAttack(p, i);
            int16_t m_decay = pataSGD.getToneDecay(p, i);
            int16_t m_sustain = pataSGD.getToneSustain(p, i);
            int16_t m_release = pataSGD.getToneRelease(p, i);
            int16_t m_pan = pataSGD.getTonePan(p, i);
            uint16_t m_porttime = pataSGD.getTonePortTime(p, i);
            int32_t sample_id = pataSGD.getToneSampleID(p, i);

            if (sample_id == -1) {
                sample_id = numvag;
            }
            else if ((sample_id < 0 || sample_id >= numvag) || !samples[sample_id]) {
                if (isDebug) fprintf(stderr, "            Sample %i doesn't exist\n", sample_id);
                continue;
            }

            uint16_t loop_type = pataSGD.getSampleLpType(sample_id);
            loop_type = (uint16_t)((loop_type == 0) ? SampleMode::kNoLoop :
                                   (loop_type == 1) ? SampleMode::kLoopEndsByKeyDepression :
                                   (loop_type == 2) ? SampleMode::kLoopContinuously : SampleMode::kUnusedNoLoop);

            std::vector<SFGeneratorItem> tone_gens {SFGeneratorItem(SFGenerator::kKeyRange, RangesType(lowRange, highRange)),
                                                    SFGeneratorItem(SFGenerator::kOverridingRootKey, rootKey),
                                                    SFGeneratorItem(SFGenerator::kCoarseTune, tuneSemi),
                                                    SFGeneratorItem(SFGenerator::kFineTune, tuneFine),
                                                    SFGeneratorItem(SFGenerator::kAttackVolEnv, m_attack),
                                                    SFGeneratorItem(SFGenerator::kDecayVolEnv, m_decay),
                                                    SFGeneratorItem(SFGenerator::kSustainVolEnv, m_sustain),
                                                    SFGeneratorItem(SFGenerator::kReleaseVolEnv, m_release),
                                                    SFGeneratorItem(SFGenerator::kPan, m_pan),
                                                    SFGeneratorItem(SFGenerator::kSampleModes, loop_type)};
            std::vector<SFModulatorItem> tone_mods {SFModulatorItem(SFModulator(SFMidiController::kPortamento,
                                                                                SFControllerDirection::kIncrease,
                                                                                SFControllerPolarity::kUnipolar,
                                                                                SFControllerType::kLinear),
                                                                    SFGenerator::kFineTune,
                                                                    m_porttime,
                                                                    SFModulator(0),
                                                                    SFTransform::kLinear)};

            std::string toneIdx = "0000" + std::to_string(tone_id),
                        bankIdx = "0000" + std::to_string(bank),
                        prstIdx = "0000" + std::to_string(preset),
                        toneName = "tone_";

            toneName += toneIdx.substr(toneIdx.size() - 3) + '.';
            toneName += bankIdx.substr(bankIdx.size() - 3) + '.';
            toneName += prstIdx.substr(prstIdx.size() - 3);

            auto iter_ton = std::find(tone_zones.begin(), tone_zones.end(), toneName);
            if (iter_ton < tone_zones.end()) {
                iter_ton->zones.push_back(SFInstrumentZone(samples[sample_id], tone_gens, tone_mods));
            }
            else {
                if (isDebug) fprintf(stdout, "        Added Tone %s to SoundFont\n",
                                     toneName.c_str());
                tone_zones.push_back(InstrumentZones(toneName, bank, preset));
                tone_zones.back().zones.push_back(SFInstrumentZone(samples[sample_id], tone_gens, tone_mods));
            }
            if (isDebug) fprintf(stdout, "            Added Sample %s to Tone\n",
                                 samples[sample_id]->name().c_str());
        }

        for (auto it = tone_zones.begin() + strt_tone; it < tone_zones.end(); ++it) {
            uint16_t bnk = it->bank, prt = it->preset;

            std::string bankIdx = "0000" + std::to_string(bnk),
                        prstIdx = "0000" + std::to_string(prt),
                        prgmName = "program_";

            prgmName += bankIdx.substr(bankIdx.size() - 3) + '.';
            prgmName += prstIdx.substr(prstIdx.size() - 3);

            tones.push_back(sf2.NewInstrument(it->name, it->zones));
            if (isDebug) fprintf(stdout, "        Added Tone %s with %u definitions to SoundFont\n",
                                 it->name.c_str(), it->zones.size());

            programs.push_back(sf2.NewPreset(prgmName, prt, bnk, std::vector<SFPresetZone> { SFPresetZone(tones.back()) }));
            if (isDebug) fprintf(stdout, "        Added Patch %s Bank %u Preset %u to SoundFont\n",
                                 prgmName.c_str(), bnk, prt);
        }
    }


    //Writing soundfont file
    title += ".sf2";
    sgh_file = sgh_file.substr(0, sgh_file.find_last_of("\\/") + 1) + title;

    if (isDebug) fprintf(stdout, "\nWriting to %s\n", title.c_str());
    try {
        sf2.Write(sgh_file);
        success = true;
    }
    catch (const std::logic_error & lerr) {
        fprintf(stderr, "\nUnable to write to %s\n", title.c_str());
        fprintf(stderr, "    %s\n", lerr.what());
        //return 0;
    }
    catch (const std::ios_base::failure & ierr) {
        fprintf(stderr, "\nUnable to write to %s\n", title.c_str());
        fprintf(stderr, "    %s\n", ierr.what());
        //return 0;
    }

    fprintf(stdout, "\nSuccessfully wrote to %s\n", title.c_str());

    if (numseq > 0) {
        if (isDebug) fprintf(stdout, "\nExtracting sequences\n");

        pataSGD.writeSequences();
    }

    if (isDebug) fprintf(stdout, "\nReached end of SF2 function\n");

    return success;
}
