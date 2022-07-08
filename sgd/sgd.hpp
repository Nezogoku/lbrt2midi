/**********************************************
            SGXD Clumps of Info
**********************************************/

#ifndef SGD_HPP
#define SGD_HPP

#include <fstream>
#include <vector>
#include "sgxddef.hpp"
#include "sgxdheaders.hpp"
#include "sgxdchk.hpp"
#include "decode/vag.hpp"


class sgd {
    public:
        sgd();
        sgd(bool isDebug);

        void setFile(std::string tmpFile);
        int setInfo();

        std::vector<int16_t> getPCM(int samp);
        int getAmountAudio();
        int getAmountDefinitions();
        std::string getBankName();
        std::string getSampleName(int samp);
        int getSampleCodec(int samp);
        int getSampleChannels(int samp);
        int getSampleSamplerate(int samp);
        uint32_t getSampleSampleSize(int samp);
        bool sampleIsLoop(int samp);
        uint32_t getSampleLpStrt(int samp);
        uint32_t getSampleLpTrm(int samp);
        uint32_t getSampleID(int def);
        uint8_t getSampleRoot(int def);
        uint8_t getSampleLowRange(int def);
        uint8_t getSampleHighRange(int def);
        uint8_t getSampleVal5(int def);
        uint8_t getSampleBank(int def);
        uint8_t getSamplePreset(int def);

    private:
        bool isDebug,
             hasRGND,
             hasSEQD,
             hasWAVE,
             hasNAME,
             hasRIFF;

        int setBankName(std::ifstream &tmpData);
        int setRGND(std::ifstream &tmpData);
        int setSEQD(std::ifstream &tmpData);
        int setWAVE(std::ifstream &tmpData, int samp);
        int setNAME(std::ifstream &tmpData, int samp);
        int setDATA(std::ifstream &tmpData, int samp);

        sgxd_head sgxdHead;
        std::string fileName;
        std::string bankName;

        rgnd_head rgndHead;
        seqd_head seqdHead;
        wave_head waveHead;
        name_head nameHead;

        uint32_t rgndOffset;
        uint32_t seqdOffset;
        uint32_t waveOffset;
        uint32_t nameOffset;

        int numRiff;

        std::vector<rgnd> rgndBank;
        std::vector<wave> waveBank;
        std::vector<seqd> seqdBank;
        std::vector<uint32_t> riffOffsets;
        std::vector<finalLoop> pcmLoopData;
        std::vector<std::string> nameBank;
        std::vector<std::vector<char>> vagBank;
};


#endif
