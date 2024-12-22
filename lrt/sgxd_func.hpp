#ifndef SGXD_FUNC_HPP
#define SGXD_FUNC_HPP

#include <string>
#include <vector>
#include "sgxd_types.hpp"

#ifdef ALLSGXD_IMPLEMENTATION
#define UNPACKBUSS_IMPLEMENTATION
#define SOUNDBANKSGXD_IMPLEMENTATION
#define ALLSGXDAUDIO_IMPLEMENTATION
#define UNPACKWSUR_IMPLEMENTATION
#define UNPACKWMKR_IMPLEMENTATION
#define UNPACKCONF_IMPLEMENTATION
#define UNPACKTUNE_IMPLEMENTATION
#define UNPACKADSR_IMPLEMENTATION
#define UNPACKNAME_IMPLEMENTATION
#endif

#ifdef SOUNDBANKSGXD_IMPLEMENTATION
#define UNPACKRGND_IMPLEMENTATION
#define UNPACKSEQD_IMPLEMENTATION
#define UNPACKWAVE_IMPLEMENTATION
#define DECODESONYADPCM_IMPLEMENTATION
#define DECODEOGG_IMPLEMENTATION
#endif

#ifndef PROGRAMME_IDENTIFIER
#define PROGRAMME_IDENTIFIER "sgd_extractor v5.1"
#endif


inline extern bool sgd_debug = false, sgd_text = false;
inline extern const unsigned char *sgd_beg = 0, *sgd_dat_beg = 0, *sgd_dat_end = 0;
inline extern sgxdinfo sgd_inf = {};

void unpackSgxd(const char *file0, const char *file1 = 0);
void unpackSgxd(unsigned char *in, const unsigned length);
void extractSgxd(const char *folder = 0);

#ifdef UNPACKBUSS_IMPLEMENTATION
void unpackBuss(unsigned char *in, const unsigned length);
std::string extractBuss();
#endif

#ifdef UNPACKRGND_IMPLEMENTATION
void unpackRgnd(unsigned char *in, const unsigned length);
std::vector<unsigned char> rgndToSfbk();
std::string extractRgnd();
#endif

#ifdef UNPACKSEQD_IMPLEMENTATION
void unpackSeqd(unsigned char *in, const unsigned length);
std::vector<unsigned char> seqdToMidi(const int &grp, const int &seq);
std::string extractSeqd();
#endif

#ifdef UNPACKWAVE_IMPLEMENTATION
void unpackWave(unsigned char *in, const unsigned length);
std::vector<unsigned char> waveToWave(const int &wav);
std::string extractWave();
#endif

#ifdef UNPACKWSUR_IMPLEMENTATION
void unpackWsur(unsigned char *in, const unsigned length);
std::string extractWsur();
#endif

#ifdef UNPACKWMKR_IMPLEMENTATION
void unpackWmkr(unsigned char *in, const unsigned length);
std::string extractWmkr();
#endif

#ifdef UNPACKCONF_IMPLEMENTATION
void unpackConf(unsigned char *in, const unsigned length);
std::string extractConf();
#endif

#ifdef UNPACKTUNE_IMPLEMENTATION
void unpackTune(unsigned char *in, const unsigned length);
std::string extractTune();
#endif

#ifdef UNPACKADSR_IMPLEMENTATION
void unpackAdsr(unsigned char *in, const unsigned length);
std::string extractAdsr();
#endif

#ifdef UNPACKNAME_IMPLEMENTATION
void unpackName(unsigned char *in, const unsigned length);
std::string extractName();
#endif


#endif
