#ifndef LRT_TYPES_HPP
#define LRT_TYPES_HPP

///LBRT header
struct lbrt_head {
    //const char lbrt[4] {'L','B','R','T'};
    unsigned soff; // Offset sequence data
    unsigned npqn; // Notated 32nd-notes per quarter note
    unsigned ppqn; // Pulses per quarter note
    unsigned trks; // Amount tracks
    unsigned frmt; // MIDI format
    unsigned val0; //
    unsigned msgs; // Amount events
    unsigned qrts; // Amount quarter events
};

///LBRT messages
struct lbrt_mesg {
    uint32_t id;    // Quarter event ID
    uint32_t dtim;  // Delta time
    uint32_t tval;  // Time value
    uint32_t eval0; // Event value 1
    uint16_t eval1; // Event value 2 (?)
    uint16_t cc;    // Channel controller
    uint16_t velon; // Channel velocity (on)
    uint16_t bndon; // Channel pan (on?)
    uint8_t chn;    // Channel
    uint8_t stat;   // Status byte
    uint8_t veloff; // Channel velocity (off?)
    uint8_t bndoff; // Channel pan (off?)
};


#endif
