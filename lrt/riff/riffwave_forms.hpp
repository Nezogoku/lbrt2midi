#ifndef RIFFWAVE_FORMS_HPP
#define RIFFWAVE_FORMS_HPP

//WAVE form
#define RIFF_WAVE   0x57415645

//WAVE common chunks
#define WAVE_cue    0x63756520  // Cue Chunk
#define WAVE_fact   0x66616374  // Fact Chunk
#define WAVE_fmt    0x666D7420  // Format Chunk
#define WAVE_inst   0x696E7374  // Instrument Chunk
#define WAVE_plst   0x706C7374  // Playlist Chunk
#define WAVE_smpl   0x736D706C  // Sampler Chunk
#define WAVE_PEAK   0x5045414B  // Peak Chunk

//LIST common chunks
#define LIST_adtl   0x6164746C  // Associated Data List
#define LIST_wavl   0x7761766C  // Wave List

//ADTL common types
#define ADTL_labl   0x6C61626C  // Cue Point Label
#define ADTL_note   0x6E6F7465  // Cue Point Note Comment
#define ADTL_ltxt   0x6C747874  // Cue Point Labeled Text

//WAVL common types
#define WAVL_data   0x64617461  // Data Chunk
#define WAVL_slnt   0x736C6E74  // Silent Chunk


#endif
