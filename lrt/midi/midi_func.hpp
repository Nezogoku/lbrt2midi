#ifndef MIDI_FUNC_HPP
#define MIDI_FUNC_HPP

#include <string>
#include <vector>
#include "midi_types.hpp"

extern midiinfo unpackMidi(unsigned char *in, const unsigned length);
extern std::vector<unsigned char> packMidi(const midiinfo &midi);

extern std::vector<mesginfo> unpackMesg(unsigned char *in, const unsigned length);

#ifdef CHECKMIDI_IMPLEMENTATION
extern int checkMidi(midiinfo &midi);
#endif

#ifdef PACKCSV_IMPLEMENTATION
extern std::string packCsv(const midiinfo &midi);
#endif


#endif
