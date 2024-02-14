#ifndef MID_SYSEX_HPP
#define MID_SYSEX_HPP

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


#endif
