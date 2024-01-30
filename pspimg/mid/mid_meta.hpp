#ifndef MID_META_HPP
#define MID_META_HPP

#include <cstdint>

///MIDI Meta-events
enum MidiMeta : uint8_t {
    META_SEQUENCE_ID            = 0,
    META_TEXT,
    META_COPYRIGHT,
    META_TRACK_NAME,
    META_INSTRUMENT_NAME,
    META_LYRICS,
    META_MARKER,
    META_CUE,
    META_PATCH_NAME,
    META_PORT_NAME,
    META_MISC_TEXT_A,
    META_MISC_TEXT_B,
    META_MISC_TEXT_C,
    META_MISC_TEXT_D,
    META_MISC_TEXT_E,
    META_MISC_TEXT_F,
    META_CHANNEL_PREFIX         = 32,
    META_PORT,
    META_END_OF_SEQUENCE        = 47,
    META_MLIVE_MARKER           = 75,
    META_TEMPO                  = 81,
    META_SMPTE                  = 84,
    META_TIME_SIGNATURE         = 88,
    META_KEY_SIGNATURE,
    META_SEQUENCER_EXCLUSIVE    = 127,
    META_NONE                   = 255,
};

///MIDI Custom Meta-event values
enum MidiCustomMeta : uint8_t {
    //M-Live (FF 4B) tags
    TT_GENRE        = 1,
    TT_ARIST,
    TT_COMPOSER,
    TT_DURATION,
    TT_BPM,
};


#endif
