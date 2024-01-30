#ifndef MIDI_SHARED_HPP
#define MIDI_SHARED_HPP

#include <string>
#include "mid_types.hpp"


class midi {
    public:
        midi() : reset() {}
        ~midi() : reset() {}

        std::string getCsv();
        std::string getCsv(unsigned mid_size, unsigned char *mid_data);
        void getMidi(unsigned &mid_size, unsigned char *&mid_data);
        void setMidi(unsigned char *in, unsigned length);
        void setFormat(unsigned char frmt);
        void optimizeMidi();

    protected:
        bool cmpStr(unsigned char *&in0, const char *in1, int length);
        unsigned getInt(unsigned char *&in, int length);
        unsigned getVLV(unsigned char *&in);
        unsigned getLengthVLV(unsigned in);
        unsigned short getPitchBend(unsigned char *&in);
        unsigned short getPitchBend(unsigned short in);
        void setMidiRaw(unsigned char *in, unsigned length, int track_id);
        
        midi_mthd mhead;
        unsigned *amnt_mmsg;
        midi_mesg **mmsgs;

    private:
        void reset();
        template<typename... Args>
        std::string formatStr(const char *in, Args... args);
        bool statIsValid(const unsigned char stat);
        void setStr(unsigned char *&out, const char *in);
        void setStr(unsigned char *&out, const char *in, int length);
        void setStr(unsigned char *&out, unsigned char *in, int length);
        void setInt(unsigned char *&out, const unsigned int in, int length);
        void setVLV(unsigned char *&out, unsigned int in);
        void setPitchBend(unsigned char *&out, unsigned short in);
};


#endif
