#ifndef PLAYMIDI_HPP
#define PLAYMIDI_HPP

#include <cstdint>
#include <vector>
#include "minisdl_audio.h"
#include "tsf.h"
#include "tml.h"


class playmidi {
    public:
        playmidi();
        playmidi(bool isDebug);

        int setBank(std::string file);
        int setBank(char* data, int len);

        int setSequence(std::string file);
        int setSequence(char* data, int len);

        int playSequence();

    private:
        int setAudioOutput();

        bool isDebug,
             hasSF2;

        std::string sequence_file;              // Sequence file
        SDL_AudioSpec outAudioSpec;             // Audio output format
};


#endif
