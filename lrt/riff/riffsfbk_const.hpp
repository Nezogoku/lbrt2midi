#ifndef RIFFSFBK_CONST_HPP
#define RIFFSFBK_CONST_HPP

#define SFBK_SMPL_PAD   46
#define SFBK_SMPL_MIN   SFBK_SMPL_PAD + 2
#define SFBK_LOOP_MIN   32
#define SFBK_MOD_SIZE   10
#define SFBK_GEN_SIZE   4
#define SFBK_BAG_SIZE   4
#define SFBK_NAME_MAX   20
#define SFBK_PHDR_SIZE  SFBK_NAME_MAX + 18
#define SFBK_IHDR_SIZE  SFBK_NAME_MAX + 2
#define SFBK_SHDR_SIZE  SFBK_NAME_MAX + 26


///Sample Type and Location in Memory
enum SfSampleType : unsigned short {
    ST_RAM_MONO                         = (1 <<  0),
    ST_RAM_RIGHT                        = (1 <<  1),
    ST_RAM_LEFT                         = (1 <<  2),
    ST_RAM_LINKED                       = (1 <<  3),
    ST_ROM_MONO                         = (1 << 15) | ST_RAM_MONO,
    ST_ROM_RIGHT                        = (1 << 15) | ST_RAM_RIGHT,
    ST_ROM_LEFT                         = (1 << 15) | ST_RAM_LEFT,
    ST_ROM_LINKED                       = (1 << 15) | ST_RAM_LINKED,
};

///Sample Modes
enum SfSampleMode : unsigned short {
    SM_NO_LOOP,
    SM_CONTINUOUS_LOOP,
    SM_UNUSED,
    SM_DEPRESSION_LOOP,
};

///Modulator Source Continuity
enum SfModulatorSourceCnt : unsigned short {
    MS_LINEAR                           = (0 << 10),
    MS_CONCAVE                          = (1 << 10),
    MS_CONVEX                           = (2 << 10),
    MS_SWITCH                           = (3 << 10),
};

///Modulator Source Polarity
enum SfModulatorSourcePol : unsigned short {
    MS_UNIPOLAR                         = (0 <<  9),
    MS_BIPOLAR                          = (1 <<  9),
};

///Modulator Source Direction
enum SfModulatorSourceDir : unsigned short {
    MS_INCREASE                         = (0 <<  8),
    MS_DECREASE                         = (1 <<  8),
};

///Modulator Source MIDI Flag
enum SfModulatorSourceFlg : unsigned short {
    MS_GENERAL_CONTROLLER               = (0 <<  7),
    MS_MIDI_CONTROLLER                  = (1 <<  7),
};

///Modulator Source General Palette
enum SfModulatorSourceGen : unsigned short {
    MS_NONE,
    MS_NOTE_VELOCITY                    = 2,
    MS_NOTE_KEY,
    MS_POLY_PRESSURE                    = 10,
    MS_CHANNEL_PRESSURE                 = 13,
    MS_PITCH_WHEEL,
    MS_PITCH_WHEEL_SENSITIVITY          = 16,
    MS_MODULATOR_LINK                   = 127,
};

//Modulator Source MIDI Palette
//enum SfModulatorSourceMid : unsigned short {
//    CC_MODULATION_WHEEL_C             = 1,
//    CC_BREATH_CONTROLLER_C,
//    CC_FOOT_PEDAL_C                   = 4,
//    CC_PORTAMENTO_TIME_C,
//    CC_CHANNEL_VOLUME_C               = 7,
//    CC_BALANCE_C,
//    CC_PAN_C                          = 10,
//    CC_EXPRESSION_C,
//    CC_EFFECT_CONTROLLER_0_C,
//    CC_EFFECT_CONTROLLER_1_C,
//    CC_GENERAL_PURPOSE_0_C            = 16,
//    CC_GENERAL_PURPOSE_1_C,
//    CC_GENERAL_PURPOSE_2_C,
//    CC_GENERAL_PURPOSE_3_C,
//    CC_HOLD_PEDAL_0                   = 64,
//    CC_PORTAMENTO_PEDAL,
//    CC_SOSTENUTO_PEDAL,
//    CC_SOFT_PEDAL,
//    CC_LEGATO_PEDAL,
//    CC_HOLD_PEDAL_1,
//    CC_SOUND_CONTROLLER_VARIATION,
//    CC_SOUND_CONTROLLER_TIMBRE,
//    CC_SOUND_CONTROLLER_RELEASE_TIME,
//    CC_SOUND_CONTROLLER_ATTACK_TIME,
//    CC_SOUND_CONTROLLER_BRIGHTNESS,
//    CC_SOUND_CONTROLLER_DECAY_TIME,
//    CC_SOUND_CONTROLLER_VIBRATO_RATE,
//    CC_SOUND_CONTROLLER_VIBRATO_DEPTH,
//    CC_SOUND_CONTROLLER_VIBRATO_DELAY,
//    CC_SOUND_CONTROLLER_UNDEFINED,
//    CC_GENERAL_PURPOSE_4,
//    CC_GENERAL_PURPOSE_5,
//    CC_GENERAL_PURPOSE_6,
//    CC_GENERAL_PURPOSE_7,
//    CC_PORTAMENTO_CONTROLLER,
//    CC_VELOCITY_EXTENSION             = 88,
//    CC_EFFECT_DEPTH_0_REVERB          = 91,
//    CC_EFFECT_DEPTH_1_TEMOLO_DEPTH,
//    CC_EFFECT_DEPTH_2_CHORUS_DEPTH,
//    CC_EFFECT_DEPTH_3_CELESTE_DEPTH,
//    CC_EFFECT_DEPTH_4_PHASER,
//    CC_DATA_INCREMENT,
//    CC_DATA_DECREMENT,
//};

///Modulator Transforms
enum SfModulatorTransform : unsigned short {
    MT_LINEAR,
    MT_ABSOLUTE = 2,
};

///Generator Types
enum SfGenerator : unsigned short {
    GN_SAMPLE_START_FINE_OFFSET,                // Tone-exclusive
    GN_SAMPLE_END_FINE_OFFSET,                  // Tone-exclusive
    GN_LOOP_START_FINE_OFFSET,                  // Tone-exclusive
    GN_LOOP_END_FINE_OFFSET,                    // Tone-exclusive
    GN_SAMPLE_START_COARSE_OFFSET,              // Tone-exclusive
    GN_MODULATION_LFO_FINE_PITCH,
    GN_VIBRATO_LFO_FINE_PITCH,
    GN_MODULATION_ENV_FINE_PITCH,
    GN_INITIAL_FILTER_FC,
    GN_INITIAL_FILTER_RESONANCE,
    GN_MODULATION_LFO_FINE_FILTER_FC,
    GN_MODULATION_ENV_FINE_FILTER_FC,
    GN_SAMPLE_END_COARSE_OFFSET,                // Tone-exclusive
    GN_MODULATION_LFO_FINE_VOLUME,
    GN_CHORUS_EFFECTS_SEND              = 15,
    GN_REVERB_EFFECTS_SEND,
    GN_DRY_PAN,
    GN_MODULATION_LFO_DELAY             = 21,
    GN_MODULATION_LFO_FREQUENCY,
    GN_VIBRATO_LFO_DELAY,
    GN_VIBRATO_LFO_FREQUENCY,
    GN_MODULATION_ENV_DELAY,
    GN_MODULATION_ENV_ATTACK,
    GN_MODULATION_ENV_HOLD,
    GN_MODULATION_ENV_DECAY,
    GN_MODULATION_ENV_SUSTAIN,
    GN_MODULATION_ENV_RELEASE,
    GN_KEY_MODULATION_ENV_HOLD,
    GN_KEY_MODULATION_ENV_DECAY,
    GN_VOLUME_ENV_DELAY,
    GN_VOLUME_ENV_ATTACK,
    GN_VOLUME_ENV_HOLD,
    GN_VOLUME_ENV_DECAY,
    GN_VOLUME_ENV_SUSTAIN,
    GN_VOLUME_ENV_RELEASE,
    GN_KEY_VOLUME_ENV_HOLD,
    GN_KEY_VOLUME_ENV_DECAY,
    GN_INSTRUMENT_ID,
    GN_KEY_RANGE                        = 43,
    GN_VELOCITY_RANGE,
    GN_LOOP_START_COARSE_OFFSET,                // Tone-exclusive
    GN_KEY_VALUE,                               // Tone-exclusive
    GN_VELOCITY_VALUE,                          // Tone-exclusive
    GN_INITIAL_ATTENUATION,
    GN_LOOP_END_COARSE_OFFSET           = 50,   // Tone-exclusive
    GN_PITCH_COARSE_TUNE,
    GN_PITCH_FINE_TUNE,
    GN_SAMPLE_ID,
    GN_SAMPLE_MODE,                             // Tone-exclusive
    GN_PITCH_SCALE_TUNE                 = 56,
    GN_SAMPLE_EXCLUSIVE_CLASS,                  // Tone-exclusive
    GN_SAMPLE_OVERRIDE_ROOT,                    // Tone-exclusive
    GN_END_OPERATION                    = 60,
};


#endif
