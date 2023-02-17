#ifndef PSXSGD_HPP
#define PSXSGD_HPP

#include <cstdint>
#include <string>
#include "psxsgd_types.hpp"

class sgd {
    public:
        sgd();
        sgd(bool isDebug);

        void setFile(std::string tmpFile);
        void setFile(std::string tmpHead, std::string tmpBody);
        int setInfo();

        int getAmountAudio();
        int getAmountPrograms();
        int getAmountTones();
        int getAmountSequence();

        std::string getBankName();
        std::string getSampleName(int samp);
        int getSample(int samp, int16_t *out);

        int getSampleCodec(int samp);
        int getSampleChannels(int samp);
        int getSampleSamplerate(int samp);
        int getSampleSize(int samp);

        int getAmountProgramTones(int prgm);
        uint8_t getToneBank(int prgm, int tone);
        //uint8_t getTonePreset(int prgm, int tone);
        uint8_t getToneID(int prgm, int tone);
        uint8_t getToneLow(int prgm, int tone);
        uint8_t getToneHigh(int prgm, int tone);
        uint8_t getToneRoot(int prgm, int tone);
        int8_t getToneTuneSemi(int prgm, int tone);
        int8_t getToneTuneFine(int prgm, int tone);
        int16_t getToneAttack(int prgm, int tone);
        int16_t getToneDecay(int prgm, int tone);
        int16_t getToneSustain(int prgm, int tone);
        int16_t getToneRelease(int prgm, int tone);
        int16_t getTonePan(int prgm, int tone);
        uint16_t getTonePortTime(int prgm, int tone);
        int32_t getToneSampleID(int prgm, int tone);

        uint8_t getSampleLpType(int samp);
        uint32_t getSampleLpStrt(int samp);
        uint32_t getSampleLpTrm(int samp);

        void writeSequences();

    private:
        bool isDebug,
             hasRGND,
             hasSEQD,
             hasWAVE,
             hasNAME;

        void resetData();
        int openSGXD();

        void setBankName(unsigned char *in);
        void setNAME(unsigned char *in);
        void setRGND(unsigned char *in);
        void setSEQD(unsigned char *in);
        void setWAVE(unsigned char *in);
        void setDATA(unsigned char *in);

        unsigned char *sgxdData;
        unsigned int sgxdSize;

        sgxd_head sgxdHead;
        std::string sgdName, sgbName, pathName;
        std::string bankName;

        rgnd_head rgndHead;
        seqd_head seqdHead;
        wave_head waveHead;
        name_head nameHead;

        s_rgnd *rgndBank;
        s_wave *waveBank;
        s_seqd *seqdBank;
        s_name *nameBank;
};


#endif
