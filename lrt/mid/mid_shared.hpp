#ifndef MIDI_SHARED_HPP
#define MIDI_SHARED_HPP

#include <string>
#include "mid_types.hpp"


class midi {
    public:
        midi() { reset(); }
        ~midi() { reset(); }

        std::string getCsv();
        std::string getCsv(unsigned char *mid_data, unsigned mid_size);
        void getMidi(unsigned char *&mid_data, unsigned &mid_size);
        void setMidi(unsigned char *in, unsigned length);
        void setFormat(unsigned char frmt);
        void optimizeMidi();

    protected:
        void reset();
        void setMidiRaw(unsigned char *in, unsigned length, int track_id);
        template<typename... Args>
        std::string setFstr(const char *in, Args... args);
        
        midi_mthd mhead;
        unsigned *amnt_mmsg = 0;
        midi_mesg **mmsgs = 0;

    private:
        void setPitchBend(unsigned char *&out, unsigned short in);
};


#endif
