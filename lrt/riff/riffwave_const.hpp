#ifndef RIFFWAVE_CONST_HPP
#define RIFFWAVE_CONST_HPP

#include "uuid_type.hpp"

//WAVE common codecs
#define CODEC_UNKNOWN           0x0000  // Unknown
#define CODEC_PCM               0x0001  // Integer PCM
#define CODEC_ADPCM             0x0002  // Microsoft Adaptive PCM
#define CODEC_IEEE_FLOAT        0x0003  // IEEE Floating-Point PCM
#define CODEC_DVI_ADPCM         0x0011  // DVI Adaptive PCM
#define CODEC_SIERRA_ADPCM      0x0013  // Sierra Adaptive PCM
#define CODEC_G723_ADPCM        0x0014  // G.723 Adaptive PCM
#define CODEC_SONARC            0x0021  // Speech Compression
#define CODEC_DOLBY_AC2         0x0030  // Dolby Labs AC-2
#define CODEC_CRES_VQLPC        0x0034  // Control Resources Limited VQLPC
#define CODEC_G721_ADPCM        0x0040  // G.721 Adaptive PCM
#define CODEC_CREATIVE_ADPCM    0x0200  // Creative Labs Adaptive PCM
#define CODEC_CREATIVE_FSPCH8   0x0202  // Creative Labs FastSpeech 8
#define CODEC_CREATIVE_FSPCH10  0x0203  // Creative Labs FastSpeech 10
#define CODEC_FLAC              0xF1AC  // FLAC
#define CODEC_EXTENSIBLE        0xFFFE  // Extensible
#define CODEC_DEVELOPMENT       0xFFFF  // Development

//WAVE speaker masks
#define SPKR_FL                 1 << 0  // Front Left
#define SPKR_FR                 1 << 1  // Front Right
#define SPKR_FC                 1 << 2  // Front Centre
#define SPKR_LF                 1 << 3  // Low-Frequency Effects
#define SPKR_BL                 1 << 4  // Back Left
#define SPKR_BR                 1 << 5  // Back Right
#define SPKR_FLC                1 << 6  // Front Left of Centre
#define SPKR_FRC                1 << 7  // Front Right of Centre
#define SPKR_BC                 1 << 8  // Back Centre
#define SPKR_SL                 1 << 9  // Side Left
#define SPKR_SR                 1 << 10 // Side Right
#define SPKR_TC                 1 << 11 // Top Centre
#define SPKR_TFL                1 << 12 // Top Front Left
#define SPKR_TFC                1 << 13 // Top Front Centre
#define SPKR_TRC                1 << 14 // Top Front Right
#define SPKR_TBL                1 << 15 // Top Back Left
#define SPKR_TBC                1 << 16 // Top Back Centre
#define SPKR_TBR                1 << 17 // Top Back Right
#define SPKR_ALL                1 << 31 // Any Configuration

//WAVE GUID's
MAKEUUID(WAVE_GUID_PCM, 0x00000001, 0x0000, 0x0010, 0x800000AA00389B71);


#endif
