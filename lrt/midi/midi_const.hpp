#ifndef MIDI_CONST_HPP
#define MIDI_CONST_HPP

///MIDI Time Measurement Types
enum MidiTimeType : bool { TIME_PPQN, TIME_MTC };

///MIDI Formats
enum MidiFormat : unsigned short {
    MIDI_SINGLE_TRACK,
    MIDI_MULTIPLE_TRACK,
    MIDI_MULTIPLE_SONG,
};

///MIDI Statuses
enum MidiStatus : short {
    //Meta messages
    META_SEQUENCE_ID            = -256,
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
    META_CHANNEL_PREFIX         = -224,
    META_PORT,
    META_END_OF_SEQUENCE        = -209,
    META_MLIVE_MARKER           = -181,
    META_TEMPO                  = -175,
    META_SMPTE                  = -172,
    META_TIME_SIGNATURE         = -168,
    META_KEY_SIGNATURE,
    META_SEQUENCER_EXCLUSIVE    = -129,
    //Undefined messages
    META_NONE                   =   -1,
    STAT_NONE                   =    0,
    //Voice messages
    STAT_NOTE_OFF               =  128,
    STAT_NOTE_ON                =  144,
    STAT_KEY_PRESSURE           =  160,
    STAT_CONTROLLER             =  176,
    STAT_PROGRAMME_CHANGE       =  192,
    STAT_CHANNEL_PRESSURE       =  208,
    STAT_PITCH_WHEEL            =  224,
    //System Common messages
    STAT_SYSTEM_EXCLUSIVE       =  240,
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
};

///MIDI Continuous Controllers
enum MidiController : unsigned char {
    CC_BANK_SELECT_C                    = 0,
    CC_MODULATION_WHEEL_C,
    CC_BREATH_CONTROLLER_C,
    CC_FOOT_PEDAL_C                     = 4,
    CC_PORTAMENTO_TIME_C,
    CC_DATA_ENTRY_C,
    CC_CHANNEL_VOLUME_C,
    CC_BALANCE_C,
    CC_PAN_C                            = 10,
    CC_EXPRESSION_C,
    CC_EFFECT_CONTROLLER_0_C,
    CC_EFFECT_CONTROLLER_1_C,
    CC_GENERAL_PURPOSE_0_C              = 16,
    CC_GENERAL_PURPOSE_1_C,
    CC_GENERAL_PURPOSE_2_C,
    CC_GENERAL_PURPOSE_3_C,
    CC_BANK_SELECT_F                    = 32,
    CC_MODULATION_WHEEL_F,
    CC_BREATH_CONTROLLER_F,
    CC_FOOT_PEDAL_F                     = 36,
    CC_PORTAMENTO_TIME_F,
    CC_DATA_ENTRY_F,
    CC_CHANNEL_VOLUME_F,
    CC_BALANCE_F,
    CC_PAN_F                            = 42,
    CC_EXPRESSION_F,
    CC_EFFECT_CONTROLLER_0_F,
    CC_EFFECT_CONTROLLER_1_F,
    CC_HOLD_PEDAL_0                     = 64,
    CC_PORTAMENTO_PEDAL,
    CC_SOSTENUTO_PEDAL,
    CC_SOFT_PEDAL,
    CC_LEGATO_PEDAL,
    CC_HOLD_PEDAL_1,
    CC_SOUND_CONTROLLER_VARIATION,
    CC_SOUND_CONTROLLER_TIMBRE,
    CC_SOUND_CONTROLLER_RELEASE_TIME,
    CC_SOUND_CONTROLLER_ATTACK_TIME,
    CC_SOUND_CONTROLLER_BRIGHTNESS,
    CC_SOUND_CONTROLLER_DECAY_TIME,
    CC_SOUND_CONTROLLER_VIBRATO_RATE,
    CC_SOUND_CONTROLLER_VIBRATO_DEPTH,
    CC_SOUND_CONTROLLER_VIBRATO_DELAY,
    CC_SOUND_CONTROLLER_UNDEFINED,
    CC_GENERAL_PURPOSE_4,
    CC_GENERAL_PURPOSE_5,
    CC_GENERAL_PURPOSE_6,
    CC_GENERAL_PURPOSE_7,
    CC_PORTAMENTO_CONTROLLER,
    CC_VELOCITY_EXTENSION               = 88,
    CC_EFFECT_DEPTH_0_REVERB            = 91,
    CC_EFFECT_DEPTH_1_TEMOLO_DEPTH,
    CC_EFFECT_DEPTH_2_CHORUS_DEPTH,
    CC_EFFECT_DEPTH_3_CELESTE_DEPTH,
    CC_EFFECT_DEPTH_4_PHASER,
    CC_DATA_INCREMENT,
    CC_DATA_DECREMENT,
    CC_NONREGISTERED_PARAMETER_C,
    CC_NONREGISTERED_PARAMETER_F,
    CC_REGISTERED_PARAMETER_C,
    CC_REGISTERED_PARAMETER_F,
    CC_RESET_SOUND                      = 120,
    CC_RESET_CONTROLLERS,
    CC_LOCAL_CONTROL_SWITCH,
    CC_RESET_NOTES,
    CC_OMNI_OFF,
    CC_OMNI_ON,
    CC_MONOPHONIC_MODE,
    CC_POLYPHONIC_MODE,
};

///MIDI Custom Continuous Controller values
enum MidiCustomController : unsigned char {
    //XML-style loops
    CC_XML_LOOPSTART        = 0x74,
    CC_XML_LOOPEND          = 0x75,
    //XML-style loop values
    CC_XML_LOOPINFINITE     = 0x00,
    CC_XML_LOOPRESERVED     = 0x7F,

    //EMIDI-style loops
    CC_EMD_LOOPSTART        = 0x76,
    CC_EMD_LOOPEND          = 0x77,
};

///MIDI Custom Meta-Event Values
enum MidiCustomMeta : unsigned char {
    //M-Live (FF 4B) tags
    TT_GENRE        = 1,
    TT_ARIST,
    TT_COMPOSER,
    TT_DURATION,
    TT_BPM,
};

///MIDI System exclusive manufacturer ID's for MIDI messages
enum MidiSysexID : unsigned char {
    SEID_COMBINED       = 0,
    //MMA-assigned List
    SEID_EDUCATIONAL    = 125,
    SEID_NONREAL,
    SEID_REAL,
};

///MIDI Non-real time system exclusive events for MIDI messages
enum MidiSysexNonreal : unsigned char {
    SENR_SAMPLE_DUMP_HEADER         = 1,
    SENR_SAMPLE_DATA_PACKET,
    SENR_SAMPLE_DUMP_REQUEST,
    SENR_TIME_CODE_N,
    SENR_SAMPLE_DUMP_EXTENSIONS,
    SENR_GENERAL_INFORMATION,
    SENR_FILE_DUMP,
    SENR_TUNING_STANDARD_N,
    SENR_GENERAL,
    SENR_DLS,
    SENR_FILE_REFERENCE,
    SENR_VISUAL_CONTROL,
    SENR_CAPABILITY_INQUIRY,
    SENR_EOF                        = 123,
    SENR_WAIT,
    SENR_CANCEL,
    SENR_NAK,
    SENR_ACK,
};

///MIDI Non-real time system exclusive sub events for MIDI messages
enum MidiSysexNonrealSub : unsigned char {
    //Time Code (Non-real Time)
    TCN_SPECIAL                         = 0,
    TCN_PUNCH_IN,
    TCN_PUNCH_OUT,
    TCN_DELETE_PUNCH_IN,
    TCN_DELETE_PUNCH_OUT,
    TCN_EVENT_START,
    TCN_EVENT_STOP,
    TCN_EVENT_START_ADD,
    TCN_EVENT_STOP_ADD,
    TCN_DELETE_EVENT_START,
    TCN_DELETE_EVENT_STOP,
    TCN_CUE,
    TCN_CUE_ADD,
    TCN_DELETE_CUE,
    TCN_EVENT_NAME,
    //Sample Dump Extensions
    SDE_LOOPS_TRANSMISSION              = 1,
    SDE_LOOPS_REQUEST,
    SDE_SAMPLE_NAME_TRANSMISSION,
    SDE_SAMPLE_NAME_REQUEST,
    SDE_EXTENDED_DUMP_HEADER,
    SDE_EXTENDED_DUMP_TRANSMISSION,
    SDE_EXTENDED_DUMP_REQUEST,
    //General Information
    GI_IDENTITY_REQUEST                = 1,
    GI_IDENTITY_REPLY,
    //File Dump
    FD_HEADER                          = 1,
    FD_DATA_PACKET,
    FD_REQUEST,
    //Tuning Standard (Non-real Time)
    TSN_BULK_DUMP_REQUEST              = 0,
    TSN_BULK_DUMP_REPLY,
    TSN_TUNE_DUMP_REQUEST              = 3,
    TSN_KEY_TUNE_DUMP,
    TSN_SCALE_TUNE_DUMP_C,
    TSN_SCALE_TUNE_DUMP_S,
    TSN_NOTE_TUNE_BANK_SELECT,
    TSN_SCALE_TUNE_C,
    TSN_SCALE_TUNE_S,
    //General
    G_SYS0_ON                          = 1,
    G_SYS_OFF,
    G_SYS1_ON,
    //Downloadable Sounds
    DLS_ON                             = 1,
    DLS_OFF,
    DLS_VOICE_ALLOCATION_OFF,
    DLS_VOICE_ALLOCATION_ON,
    //File Reference
    FR_OPEN                            = 1,
    FR_SELECT,
    FR_OPEN_SELECT,
    FR_CLOSE,
};

///Real time system exclusive events for MIDI messages
enum MidiSysexReal : unsigned char {
    SER_TIME_CODE_R                     = 1,
    SER_SHOW_CONTROL,
    SER_NOTATION_INFORMATION,
    SER_DEVICE_CONTROL,
    SER_MTC_CUE,
    SER_MACHINE_CONTROL_COMMAND,
    SER_MACHINE_CONTROL_RESPONSE,
    SER_TUNING_STANDARD_R,
    SER_CONTROLLER_DESTINATION_SETTING,
    SER_KEY_INSTRUMENT_CONTROL,
    SER_SCALED_POLY_MIP,
    SER_MOBILE_PHONE_CONTROL,
};

///Real time system exclusive sub events for MIDI messages
enum MidiSysexRealSub : unsigned char {
    //Time Code (Real Time)
    TCR_FULL                        = 1,
    TCR_USER_BITS,
    //Notation Information
    NI_BAR_NUMBER                   = 1,
    NI_TIME_SIGNATURE,
    NI_TIME_SIGNATURE_DELAYED       = 66,
    //Device Control
    DC_MASTER_VOLUME                = 1,
    DC_MASTER_BALANCE,
    DC_MASTER_TUNE_F,
    DC_MASTER_TUNE_C,
    DC_GLOBAL_PARAMETER_CONTROL,
    //MTC Cue (Real Time)
    CR_SPECIAL                      = 0,
    CR_PUNCH_IN,
    CR_PUNCH_OUT,
    CR_EVENT_START                  = 5,
    CR_EVENT_STOP,
    CR_EVENT_START_ADD,
    CR_EVENT_STOP_ADD,
    CR_CUE                          = 11,
    CR_CUE_ADD,
    CR_EVENT_NAME                   = 14,
    //Machine Control Command
    MMC_STOP                        = 1,
    MMC_PLAY,
    MMC_PLAY_DELAYED,
    MMC_FAST_FORWARD,
    MMC_REWIND,
    MMC_RECORD_STROBE,
    MMC_RECORD_STOP,
    MMC_RECORD_PAUSE,
    MMC_PAUSE,
    MMC_EJECT,
    MMC_CHASE,
    MMC_RESET                       = 13,
    MMC_RECORD_START                = 64,
    MMC_LOCATE                      = 68,
    MMC_SHUTTLE                     = 71,
    //Tuning Standard (Real Time)
    TSR_NOTE_TUNE                   = 2,
    TSR_NOTE_TUNE_BANK_SELECT       = 7,
    TSR_SCALE_TUNE_C,
    TSR_SCALE_TUNE_S,
    //Controller Destination Setting
    CDS_CHANNEL_PRESSURE            = 1,
    CDS_POLY_KEY_PRESSURE,
    CDS_CONTROLLER,
};


///MIDI Status Prefered Order
const MidiStatus MIDISTATUS_ORDER[] {
    STAT_CLOCK, STAT_REAL_UNDEFINED_9, STAT_SEQUENCE_START, STAT_SEQUENCE_CONTINUE,
    STAT_SEQUENCE_STOP, STAT_REAL_UNDEFINED_D, STAT_ACTIVE, STAT_RESET, STAT_SYSTEM_EXCLUSIVE,
    STAT_QUARTER_FRAME, STAT_SEQUENCE_POINTER, STAT_SEQUENCE_REQUEST, STAT_SYSTEM_UNDEFINED_4,
    STAT_SYSTEM_UNDEFINED_5, STAT_TUNE_REQUEST, STAT_SYSTEM_EXCLUSIVE_STOP, META_TRACK_NAME, META_COPYRIGHT,
    META_INSTRUMENT_NAME, META_TEMPO, META_SMPTE, META_TIME_SIGNATURE, META_KEY_SIGNATURE, META_CHANNEL_PREFIX,
    META_PORT, META_SEQUENCER_EXCLUSIVE, META_SEQUENCE_ID, META_MLIVE_MARKER, META_TEXT, META_LYRICS,
    META_MARKER, META_CUE, META_PATCH_NAME, META_PORT_NAME, META_MISC_TEXT_A, META_MISC_TEXT_B,
    META_MISC_TEXT_C, META_MISC_TEXT_D, META_MISC_TEXT_E, META_MISC_TEXT_F, STAT_CONTROLLER, STAT_PROGRAMME_CHANGE,
    STAT_CHANNEL_PRESSURE, STAT_KEY_PRESSURE, STAT_PITCH_WHEEL, STAT_NOTE_OFF, STAT_NOTE_ON, META_END_OF_SEQUENCE,
};


#endif
