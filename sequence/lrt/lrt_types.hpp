#ifndef LRT_TYPES_HPP
#define LRT_TYPES_HPP

#include <cstdint>

//Struct for LBRT events
struct lbrt_status {
    uint32_t event_id;  // Event ID
    uint32_t delta_t;   // Delta time
    uint32_t time_val;  // Time value
    uint32_t ce_val;    // Value of Channel Event
    uint16_t unk_val0;  //
    uint16_t ce_typ;    // Type of Channel Controller
    uint16_t ce_vel;    // Channel velocity
    uint16_t ce_pan;    // Channel pan (?)
    uint8_t ce_chan;    // Channel of Channel Event
    uint8_t status;     // Status byte
    uint8_t unk_val3;   //
    uint8_t unk_val4;   //
};


//Enumerator and Sub-structures for MIDI events
enum EVENT_TYPE {
    META,
    PROGRAM,
    CONTROL,
    BEND,
    NOTE,
    NONE = 0xFF
};

struct meta {
    uint8_t type;
    uint8_t lnth;
    uint8_t *data;

    meta() = default;
    meta(uint8_t t, uint8_t s, uint8_t *d) : type(t), lnth(s), data(d) {}
};

struct pitchbend {
    uint16_t bend;

    pitchbend() : bend(0x40) {}
    pitchbend(uint16_t b) : bend(b) {}
};

struct program {
    uint8_t prgm;

    program() = default;
    program(uint8_t p) : prgm(p) {}
};

struct control {
    uint8_t type;
    uint8_t val;

    control() = default;
    control(uint8_t t, uint8_t v) : type(t), val(v) {}
};

struct noteonoff {
    uint16_t pan;
    uint8_t note;
    uint8_t vel;
    pitchbend bend;

    noteonoff() = default;
    noteonoff(uint8_t n, uint8_t v, uint16_t p) : note(n), vel(v), pan(p) {}
};

//Struct for MIDI events
struct midi_status {
    uint32_t absol_t;   // Absolute time
    int8_t channel;     // Channel

    uint8_t event_type;
    union event {
        meta m;
        program p;
        noteonoff n;
        pitchbend b;
        control c;

        event(meta t_m) : m(t_m) {}
        event(program t_p) : p(t_p) {}
        event(noteonoff t_n) : n(t_n) {}
        event(pitchbend t_b) : b(t_b) {}
        event(control t_c) : c(t_c) {}
    } event;

    midi_status() : absol_t(-1), channel(-1), event_type(NONE), event(event) {}
    midi_status(uint32_t t_abst, int8_t ch, meta t_m) : absol_t(t_abst), channel(ch), event_type(META), event(t_m) {}
    midi_status(uint32_t t_abst, int8_t ch, program t_p) : absol_t(t_abst), channel(ch), event_type(PROGRAM), event(t_p) {}
    midi_status(uint32_t t_abst, int8_t ch, control t_c) : absol_t(t_abst), channel(ch), event_type(CONTROL), event(t_c) {}
    midi_status(uint32_t t_abst, int8_t ch, pitchbend t_b) : absol_t(t_abst), channel(ch), event_type(BEND), event(t_b) {}
    midi_status(uint32_t t_abst, int8_t ch, noteonoff t_n) : absol_t(t_abst), channel(ch), event_type(NOTE), event(t_n) {}

    //To make sorting in ascending order easier
    bool operator<(const midi_status &lrt) const {
        return (absol_t < lrt.absol_t) || ((absol_t == lrt.absol_t) && (event_type < lrt.event_type));
    }
};


#endif
