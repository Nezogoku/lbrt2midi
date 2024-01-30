#ifndef MID_TYPES_HPP
#define MID_TYPES_HPP

#include <cstdint>
#include <variant>

#define MIDI_MTHD       "MThd"
#define MIDI_MTHD_SIZE  6
#define MIDI_MTRK       "MTrk"

///MIDI time measurement types
enum MidiTimeType : bool {
    TIME_PPQN   = 0,
    TIME_MTC,
};

///MIDI formats
enum MidiFormat : uint8_t {
    MIDI_SINGLE_TRACK   = 0,
    MIDI_MULTIPLE_TRACK,
    MIDI_MULTIPLE_SONG,
};

///MIDI statuses
enum MidiStatus : uint8_t {
    STAT_NONE                   = 0x00,
    //Voice messages
    STAT_NOTE_OFF               = 0x80,
    STAT_NOTE_ON                = 0x90,
    STAT_KEY_PRESSURE           = 0xA0,
    STAT_CONTROLLER             = 0xB0,
    STAT_PROGRAMME_CHANGE       = 0xC0,
    STAT_CHANNEL_PRESSURE       = 0xD0,
    STAT_PITCH_WHEEL            = 0xE0,
    //System Common messages
    STAT_SYSTEM_EXCLUSIVE       = 0xF0,
    STAT_QUARTER_FRAME,
    STAT_SEQUENCE_POINTER,
    STAT_SEQUENCE_REQUEST,
    STAT_SYSTEM_UNDEFINED_4,
    STAT_SYSTEM_UNDEFINED_5,
    STAT_TUNE_REQUEST,
    STAT_SYSTEM_EXCLUSIVE_STOP,
    //System Real-time messages
    STAT_CLOCK,
    STAT_REAL_UNDEFINED_9,
    STAT_SEQUENCE_START,
    STAT_SEQUENCE_CONTINUE,
    STAT_SEQUENCE_STOP,
    STAT_REAL_UNDEFINED_D,
    STAT_ACTIVE,
    //System Real-time and Meta message
    STAT_RESET,
};

///MIDI message sizes
enum MidiMessageSize : uint8_t {
    STAT_MESSAGE_NONE       = 0x00,
    STAT_MESSAGE_SINGLE,
    STAT_MESSAGE_DOUBLE,
    STAT_MESSAGE_VARIABLE,
};


///MIDI variable data
struct midi_mval {
    uint8_t dtyp, *data;
    uint32_t dsiz;

    midi_mval(uint8_t t = -1, uint32_t s = 0, const char *d = 0) : dtyp(t), dsiz(s) {
        if (d) {
            if (s > 0) data = new uint8_t[s + 1] {};
            for (int i = 0; i < s; ++i) data[i] = d[s];
        }
    }
    template <int S>
    midi_mval(uint8_t t, const uint8_t (&d)[S]) : midi_mval(t, S, (const char*)d) {}
    midi_mval(const midi_mval &m) { copy(m); }
    midi_mval(midi_mval &&m) : midi_mval{m} { m.~midi_mval(); }
    
    ~midi_mval() { reset(); }
    
    midi_mval& operator=(const midi_mval &m) { copy(m); return *this; }
    midi_mval& operator=(midi_mval &&m) { copy((const midi_mval)m); m.~midi_mval(); return *this; }
    
    const char *get_string() const { return (dtyp > 0 && dtyp < 16 && dsiz) ? (const char*)data : 0; }
    
    bool empty() const { return dtyp == 0xFF; }
    
    bool operator==(const midi_mval &m) const {
        bool istrue = (m.dtyp == dtyp) && (m.dsiz == dsiz);
        if (istrue) {
            int i = 0;
            while ((i < m.dsiz) && (istrue = (m.data[i] == data[i]))) { i += 1; }
        }
        return istrue;
    }
    
    bool operator<(const midi_mval &m) const {
        return dtyp < m.dtyp;
    }
    
    private:
        void reset() { if (data) delete[] data; data = 0; }
        void copy(const midi_mval &m) {
            reset();
            dtyp = m.dtyp;
            dsiz = m.dsiz;
            if (dsiz) data = new uint8_t[dsiz] {};
            for (int d = 0; d < dsiz; ++d) data[d] = m.data[d];
        }
};


///MIDI file header
struct midi_mthd {
    //const char mthd[4] {'M','T','h','d'};
    //const uint32_t mthd_size = 0x06;
    uint16_t mthd_frmt;
    uint16_t mthd_trks;
    uint16_t mthd_divi;

    midi_mthd(uint16_t t_f = 0, uint16_t t_t = 0, uint16_t t_d = 0) :
        mthd_frmt(t_f), mthd_trks(t_t), mthd_divi(t_d) {}

    midi_mthd& operator=(const midi_mthd &h) {
        mthd_frmt = h.mthd_frmt;
        mthd_trks = h.mthd_trks;
        mthd_divi = h.mthd_divi;
        return *this;
    }
};

///MIDI track header
struct midi_mtrk {
    //const char mtrk[4] {'M','T','r','k'};
    uint32_t mtrk_size;
};

///MIDI message
struct midi_mesg {
    uint32_t mmsg_time;
    uint8_t mmsg_stat;
    std::variant<std::monostate, uint8_t, uint16_t, midi_mval> mmsg_data;

    midi_mesg() : mmsg_time(0), mmsg_stat(STAT_NONE) {}
    midi_mesg(uint32_t t_t, uint8_t t_s) : mmsg_time(t_t), mmsg_stat(t_s) {}
    midi_mesg(uint32_t t_t, uint8_t t_s, uint8_t t_o) : mmsg_time(t_t), mmsg_stat(t_s), mmsg_data(t_o) {}
    midi_mesg(uint32_t t_t, uint8_t t_s, uint16_t t_d) : mmsg_time(t_t), mmsg_stat(t_s), mmsg_data(t_d) {}
    midi_mesg(uint32_t t_t, uint8_t t_s, const uint16_t (&t_d)[2]) :
        midi_mesg(t_t, t_s, uint16_t((t_d[0] << 8) | (t_d[1] & 0xFF))) {}
    midi_mesg(uint32_t t_t, uint8_t t_s, midi_mval t_v) : mmsg_time(t_t), mmsg_stat(t_s), mmsg_data(t_v) {}
    midi_mesg(const midi_mesg &m) { copy(m); }
    midi_mesg(midi_mesg &&m) : midi_mesg{m} { m.~midi_mesg(); }
    
    midi_mesg& operator=(const midi_mesg &m) { copy(m); return *this; }
    midi_mesg& operator=(midi_mesg &&m) { copy((const midi_mesg)m); m.~midi_mesg(); return *this; }
    
    //To make getting strings easier
    const char *get_string() const {
        bool isreset = (mmsg_stat == STAT_RESET);
        return (isreset) ? (std::get<midi_mval>(mmsg_data).get_string() : 0;
    }
    
    //To make checking existance easier
    bool empty() const {
        bool isempty = (mmsg_stat == STAT_SYSTEM_EXCLUSIVE ||
                        mmsg_stat == STAT_SYSTEM_EXCLUSIVE_STOP ||
                        mmsg_stat == STAT_RESET) && (std::get<midi_mval>(mmsg_data).dtyp == 0xFF);
        return (mmsg_stat == STAT_NONE) || isempty;
    }

    //To make comparisons easier
    bool operator==(const midi_mesg &m) const {
        bool istrue = (m.mmsg_stat == mmsg_stat) && (m.mmsg_data == mmsg_data) &&
                      (m.mmsg_data.index() == mmsg_data.index());
        if (istrue) {
            switch (m.mmsg_data.index()) {
                case STAT_MESSAGE_NONE:
                    istrue = (std::get<0>(m.mmsg_data) == std::get<0>(mmsg_data));
                    break;
                case STAT_MESSAGE_SINGLE:
                    istrue = (std::get<1>(m.mmsg_data) == std::get<1>(mmsg_data));
                    break;
                case STAT_MESSAGE_DOUBLE:
                    istrue = (std::get<2>(m.mmsg_data) == std::get<2>(mmsg_data));
                    break;
                case STAT_MESSAGE_VARIABLE:
                    istrue = (std::get<3>(m.mmsg_data) == std::get<3>(mmsg_data));
                    break;
            }
        }
        return istrue;
    }

    //To make sorting in ascending order easier
    bool operator<(const midi_mesg &m) const {
        bool isend = (mmsg_stat == STAT_RESET) && (std::get<midi_mval>(mmsg_data).dtyp == 0x7F);
        return (mmsg_time < m.mmsg_time) || ((mmsg_time == m.mmsg_time) && (mmsg_stat > m.mmsg_stat && !isend));
    }

    private:
        void copy(const midi_mesg &m) {
            mmsg_time = m.mmsg_time;
            mmsg_stat = m.mmsg_stat;
            mmsg_data = m.mmsg_data;
        }
};


#endif
