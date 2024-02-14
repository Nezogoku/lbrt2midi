#ifndef PLAYMIDI_HPP
#define PLAYMIDI_HPP

#include <string>
#include "tsf/minisdl_audio.h"
#include "tsf/tsf.h"
#include "tsf/tml.h"


class playmidi {
    public:
        ~playmidi();
        playmidi();

        void setDebug(bool debug) { this->debug = debug; }
        int setBank(std::string bank_file);
        int setSequence(std::string midi_file);

        int playSequence();

    protected:
        int setBank(unsigned char *data, unsigned data_size);
        int setSequence(unsigned char *data, unsigned data_size);
        
        bool debug;
        std::string seq_name;
    
    private:
        int setAudioOutput();

        SDL_AudioSpec outAudioSpec;
        tsf *bank;
        tml_message *mesg;
};


#endif
