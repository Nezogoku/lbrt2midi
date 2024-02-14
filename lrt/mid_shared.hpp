#ifndef MIDI_SHARED_HPP
#define MIDI_SHARED_HPP

#include <string>
#include <vector>
#include "mid_types.hpp"


class midi {
    public:
        midi() = default;
        ~midi() = default;

        int getCsv(std::string csv_file);
        int getMidi(std::string mid_file);
        int setMidi(std::string mid_file);
        void setFormat(unsigned char frmt);
        void optimizeMidi();

    protected:
        std::string getCsv();
        void getMidi(unsigned char *&mid_data, unsigned &mid_size);
        void setMidi(unsigned char *in, unsigned length);
        void setMidiRaw(unsigned char *in, unsigned length, int track_id);
        
        midi_mthd mhead;
        std::vector<std::vector<midi_mesg>> mmsgs;
};


#endif
