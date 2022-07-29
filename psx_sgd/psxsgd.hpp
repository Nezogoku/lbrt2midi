#ifndef PSXSGD_HPP
#define PSXSGD_HPP

#include <fstream>
#include <vector>
#include "psxsgd_defines.hpp"


class sgd {
    public:
        sgd();
        sgd(bool isDebug);

        void setFile(std::string tmpFile);
        int setInfo();

        int getAmountAudio();
        int getAmountDefinitions();
        int getAmountSequence();

        std::string getBankName();
        std::string getSampleName(int samp);
        std::vector<int16_t> getSample(int samp);

        int getSampleCodec(int samp);
        int getSampleChannels(int samp);
        int getSampleSamplerate(int samp);
        int getSampleID(int samp);
        uint8_t getSampleRoot(int samp);
        uint8_t getSampleLow(int samp);
        uint8_t getSampleHigh(int samp);
        int getSampleAttack(int samp);
        int getSampleHold(int samp);
        int getSampleSustain(int samp);
        int getSampleRelease(int samp);
        int getSamplePan(int samp);
        uint8_t getSampleBank(int samp);
        uint8_t getSamplePreset(int samp);
        bool sampleIsLoop(int samp);
        uint32_t getSampleLpStrt(int samp);
        uint32_t getSampleLpTrm(int samp);

        void writeSequence(int seq);

    private:
        bool isDebug,
             hasRGND,
             hasSEQD,
             hasWAVE,
             hasNAME;

        void setReverse(uint32_t &tmpInt);
        void setBankName(std::ifstream &tmpData);
        void setNAME(std::ifstream &tmpData);
        void setRGND(std::ifstream &tmpData);
        void setSEQD(std::ifstream &tmpData);
        void setWAVE(std::ifstream &tmpData);
        void setDATA(std::ifstream &tmpData);

        sgxd_head sgxdHead;
        std::string fileName;
        std::string bankName;

        rgnd_head rgndHead;
        seqd_head seqdHead;
        wave_head waveHead;
        name_head nameHead;

        std::vector<rgnd> rgndBank;
        std::vector<wave> waveBank;
        std::vector<seqd> seqdBank;
        std::vector<name> nameBank;
};


#endif