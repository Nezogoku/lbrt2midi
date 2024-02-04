#ifndef PLAYMIDI_HPP
#define PLAYMIDI_HPP

#include <string>
#include "minisdl_audio.h"
#include "tsf.h"
#include "tml.h"


class playmidi {
    public:
        ~playmidi();
        playmidi();
        playmidi(int num_seq);
        playmidi(int num_seq, bool isDebug);

        void setDebug(bool isDebug);
        void setAmountSequences(int num_seq);
        int setBank(std::string bank_file);
        int setBank(unsigned data_size, unsigned char *data);
        int setSequence(std::string midi_file, int s);
        int setSequence(unsigned data_size, unsigned char *data, std::string data_name, int s);

        int playSequence();

    private:
        bool hasDuplicate(std::string name);
        int setAudioOutput();

        bool debug;

        int num_seqs;
        std::string *seq_names;

        SDL_AudioSpec outAudioSpec;
        tsf *bank;
        tml_message **mesg;
};


#endif
