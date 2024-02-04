#ifndef LRT_TYPES_HPP
#define LRT_TYPES_HPP

#include <cstdint>

#define LBRT_LBRT   "LBRT"


///LBRT header
struct lbrt_head {
    //const char lbrt[4] {'L','B','R','T'};
    uint32_t soff; // Offset sequence data
    uint32_t npqn; // Notated 32nd-notes per quarter note
    uint32_t ppqn; // Pulses per quarter note
    uint32_t trks; // Amount tracks
    uint32_t frmt; // MIDI format
    uint32_t val0; //
    uint32_t msgs; // Amount events
    uint32_t qrts; // Amount quarter events

    uint32_t *qids;// Quarter event id's

    lbrt_head() = default;
    ~lbrt_head() { reset(); }
    
    lbrt_head& operator=(const lbrt_head &r) {
        reset();
        soff = r.soff;
        npqn = r.npqn;
        ppqn = r.ppqn;
        trks = r.trks;
        frmt = r.frmt;
        val0 = r.val0;
        msgs = r.msgs;
        qrts = r.qrts;
        copy(r.qids, qids, r.qrts, 0);
        return *this;
    }
    
    private:
        void reset() { if (qids) delete[] qids; }
        template <typename T0, typename T1>
        void copy(const T0 *in, T1 *&out, const int S, const int a) {
            if (in) {
                if (S > 0) out = new T1[S + a] {};
                for (int i = 0; i < S; ++i) out[i] = T1(in[i]);
            }
        }
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
