#ifndef MID_TYPES_HPP
#define MID_TYPES_HPP

#include <vector>

///MIDI time measurement types
enum MidiTimeType : bool {
    TIME_PPQN   = 0,
    TIME_MTC,
};

///MIDI formats
enum MidiFormat : unsigned short {
    MIDI_SINGLE_TRACK   = 0,
    MIDI_MULTIPLE_TRACK,
    MIDI_MULTIPLE_SONG,
};

///MIDI statuses
enum MidiStatus : unsigned short {
    STAT_NONE                   = 0x0000,
    //Voice messages
    STAT_NOTE_OFF               = 0x0080,
    STAT_NOTE_ON                = 0x0090,
    STAT_KEY_PRESSURE           = 0x00A0,
    STAT_CONTROLLER             = 0x00B0,
    STAT_PROGRAMME_CHANGE       = 0x00C0,
    STAT_CHANNEL_PRESSURE       = 0x00D0,
    STAT_PITCH_WHEEL            = 0x00E0,
    //System Common messages
    STAT_SYSTEM_EXCLUSIVE       = 0x00F0,
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
    STAT_RESET,
    //Meta messages
    META_SEQUENCE_ID            = 0xFF00,
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
    META_CHANNEL_PREFIX         = 0xFF20,
    META_PORT,
    META_END_OF_SEQUENCE        = 0xFF2F,
    META_MLIVE_MARKER           = 0xFF4B,
    META_TEMPO                  = 0xFF51,
    META_SMPTE                  = 0xFF54,
    META_TIME_SIGNATURE         = 0xFF58,
    META_KEY_SIGNATURE,
    META_SEQUENCER_EXCLUSIVE    = 0xFF7F,
    META_NONE                   = 0xFFFF,
};

///MIDI Custom Meta-event values
enum MidiCustomMeta : unsigned char {
    //M-Live (FF 4B) tags
    TT_GENRE        = 1,
    TT_ARIST,
    TT_COMPOSER,
    TT_DURATION,
    TT_BPM,
};


///MIDI file header
struct midi_mthd {
    //const char mthd[4] {'M','T','h','d'};
    //const unsigned mthd_size = 0x06;
    unsigned short mthd_frmt;
    unsigned short mthd_trks;
    unsigned short mthd_divi;

    ~midi_mthd() = default;
    midi_mthd() = default;
    midi_mthd(unsigned short t_f, unsigned short t_t, unsigned short t_d) :
        mthd_frmt(t_f), mthd_trks(t_t), mthd_divi(t_d) {}
    midi_mthd(const midi_mthd &h) = default;
    midi_mthd(midi_mthd &&h) = default;
    
    midi_mthd& operator=(const midi_mthd &h) = default;
    midi_mthd& operator=(midi_mthd &&h) = default;
};

///MIDI track header
struct midi_mtrk {
    //const char mtrk[4] {'M','T','r','k'};
    unsigned mtrk_size;
};

///MIDI message
struct midi_mesg {
    unsigned mmsg_time;
    unsigned short mmsg_stat;
    std::vector<unsigned char> mmsg_data;

    ~midi_mesg() = default;
    midi_mesg() = default;
    midi_mesg(const unsigned t_t, const unsigned short t_s) :
        mmsg_time(t_t), mmsg_stat(t_s) {}
    midi_mesg(const unsigned t_t, const unsigned short t_s, const unsigned char t_d) :
        mmsg_time(t_t), mmsg_stat(t_s) { mmsg_data.push_back(t_d); }
    midi_mesg(const unsigned t_t, const unsigned short t_s, const int t_z, const unsigned char *t_d) :
        mmsg_time(t_t), mmsg_stat(t_s) { if (t_z > 0) mmsg_data.assign(t_d, t_d + t_z); }
    midi_mesg(const unsigned t_t, const unsigned short t_s, const int t_z, const char *t_d) :
        midi_mesg{t_t, t_s, t_z, (const unsigned char*)t_d} {}
    template <int t_Z>
    midi_mesg(const unsigned t_t, const unsigned short t_s, const unsigned char (&t_d)[t_Z]) :
        midi_mesg{t_t, t_s, t_Z, t_d} {}
    midi_mesg(const midi_mesg &m) = default;
    midi_mesg(midi_mesg &&m) = default;
    
    midi_mesg& operator=(const midi_mesg &m) = default;
    midi_mesg& operator=(midi_mesg &&m) = default;
    
    //To make sorting in ascending order easier
    bool operator<(const midi_mesg &m) const {
        bool is_less = (mmsg_time < m.mmsg_time);
        if (!is_less) {
            if (mmsg_time == m.mmsg_time) {
                if (((short)mmsg_stat < (short)m.mmsg_stat)) {
                    is_less = (mmsg_stat == META_CHANNEL_PREFIX);
                    if (!is_less) is_less = (mmsg_stat == STAT_CONTROLLER);
                    if (!is_less) is_less = (mmsg_stat != META_END_OF_SEQUENCE);
                }
                else if (((short)mmsg_stat == (short)m.mmsg_stat)) {
                    is_less = (mmsg_data < m.mmsg_data);
                }
            }
        }
        return is_less;
    }
    
    //To make comparisons easier
    bool operator==(const midi_mesg &m) const {
        bool is_equl = (mmsg_time == m.mmsg_time);
        if (is_equl) is_equl = (mmsg_stat == m.mmsg_stat);
        if (is_equl) is_equl = (mmsg_data == m.mmsg_data);
        return is_equl;
    }
    
    //To make comparisons easier
    bool operator>(const midi_mesg &m) const {
        bool is_great = (mmsg_time > m.mmsg_time);
        if (!is_great) {
            if (mmsg_time == m.mmsg_time) {
                if (((short)mmsg_stat > (short)m.mmsg_stat)) {
                    is_great = (mmsg_stat == META_END_OF_SEQUENCE);
                    if (!is_great) is_great = (mmsg_stat == STAT_CONTROLLER);
                    if (!is_great) is_great = (mmsg_stat != META_CHANNEL_PREFIX);
                }
                else if (((short)mmsg_stat == (short)m.mmsg_stat)) {
                    is_great = (mmsg_data > m.mmsg_data);
                }
            }
        }
        return is_great;
    }
};


#endif
