#ifndef MIDI_FUNC_HPP
#define MIDI_FUNC_HPP

#include <string>
#include <vector>
#include "midi_types.hpp"


inline extern midiinfo mid_inf = {};

void unpackMesg(unsigned char *in, const unsigned length);
void unpackMidi(unsigned char *in, const unsigned length);
std::vector<unsigned char> packMidi();

#ifdef CHECKMIDI_IMPLEMENTATION
int checkMidi();
#endif

#ifdef PACKCSV_IMPLEMENTATION
std::string packCsv();
#endif


#endif
