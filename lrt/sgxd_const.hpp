#ifndef SGXD_CONST_HPP
#define SGXD_CONST_HPP

//SGXD common codecs
#define SGXD_CODEC_PCM16LE          0x00 // 16bit Little Endian Integer PCM
#define SGXD_CODEC_PCM16BE          0x01 // 16bit Big Endian Integer PCM
#define SGXD_CODEC_OGG_VORBIS       0x02 // Ogg Vorbis
#define SGXD_CODEC_SONY_ADPCM       0x03 // Sony Adaptive PCM
#define SGXD_CODEC_SONY_ATRAC3PLUS  0x04 // Sony ATRAC3+
#define SGXD_CODEC_SONY_SHORT_ADPCM 0x05 // Sony Short Adaptive PCM
#define SGXD_CODEC_DOLBY_AC_3       0x06 // ATSC A/52
#define SGXD_CODEC_UNKNOWN0         0x0A // Unknown Codec
#define SGXD_CODEC_UNKNOWN1         0x0B // Unknown Codec

///SGXD Region Flags
enum SgxdRgndFlag : unsigned {
    RGND_VOLUME                 = 1 << 0,
    RGND_PITCH                  = 1 << 1,
    RGND_SAMPLEID               = 1 << 2,
};

///SGXD Sequence Flags
enum SgxdSeqdFlag : unsigned {
    SEQD_LOOPED                 = 1 << 0,
};

///SGXD Sequence Type
enum SgxdSeqdType : short { SEQD_REQUEST = 0, SEQD_RAWMIDI };

///SGXD Sequence Request Statuses
enum SgxdSeqdStatus : char {
    SEQD_SUB_START              = -80,
    SEQD_SUB_STOP,
    SEQD_SUB_STOPREL,
    SEQD_SUB_GETSTAT,
    SEQD_CONTROL                = -64,
    SEQD_ADSR,
    SEQD_BEND,
    SEQD_ADSR_DIRECT,
    SEQD_START                  = -48,
    SEQD_STOP,                          // Instantly stop played sample
    SEQD_STOPREL,                       // Slowly decrease volume of played sample
    SEQD_GETPORTSTAT,
    SEQD_STARTSMPL,
    SEQD_STARTNOISE,
    SEQD_SYSREG_INIT            = -32,
    SEQD_SYSREG_ADD,
    SEQD_SYSREG_MINUS,
    SEQD_SYSREG_MULT,
    SEQD_SYSREG_DIVI,
    SEQD_SYSREG_MODU,
    SEQD_SYSREG_AND,
    SEQD_SYSREG_OR,
    SEQD_SYSREG_XOR,
    SEQD_NULL                   = -16,
    SEQD_WAIT,
    SEQD_JUMP,
    SEQD_LOOPBEG,
    SEQD_LOOPEND,
    SEQD_JUMPNEQ,
    SEQD_JUMP3,
    SEQD_JUMPGEQ,
    SEQD_JUMPLES,
    SEQD_CALLMKR,
    SEQD_LOOPBREAK,
    SEQD_PRINT                  = -2,
    SEQD_EOR,
    SEQD_POSITIVE,                      // 0X; 0 is hexstring rep. of positive number, X is size
    SEQD_NEGATIVE,                      // 1X; 1 is hexstring rep. of negative number, X is size
    SEQD_RANDOM,                        // 2X; 2 is random number between range, X is overall size
    SEQD_SYSREG,
    SEQD_REG,
    SEQD_SKIP,
};

///SGXD Sequence CC Values
enum SgxdSeqdCC : unsigned char {
    //PSX-style controllers
    SEQD_CC_UNKNOWN0            = 0x03,
    SEQD_CC_PSX_LOOP            = 0x63,
    SEQD_CC_UNKNOWN1            = 0x76,
    SEQD_CC_SONGEVENT,
    //PSX-style loop values
    SEQD_CC_PSX_LOOPSTART       = 0x14,
    SEQD_CC_PSX_LOOPEND         = 0x1E,
};

///SGXD Waveform Flags
enum SgxdWaveFlag : unsigned {
    WAVE_MONO                   = 1 << 0,
    WAVE_WSUR_REQUEST           = 1 << 1,
    WAVE_WMKR_REQUEST           = 1 << 2,
};

///SGXD Name Request Values
enum SgxdNameRequest : unsigned char {
    NAME_STREAM,
    NAME_NOTE,
    NAME_SEQUENCE,
    NAME_SAMPLE,
    NAME_SETUP,
    NAME_EFFECT,
    NAME_CONFIG,
};


#endif
