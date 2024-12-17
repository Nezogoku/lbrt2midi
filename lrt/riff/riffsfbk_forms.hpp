#ifndef RIFFSFBK_FORMS_HPP
#define RIFFSFBK_FORMS_HPP

//SFBK form
#define RIFF_sfbk   0x7366626B

//SFBK common chunks
//#define SFBK_LIST RIFF_LIST   // List Chunk

//LIST common chunks
//#define LIST_INFO LIST_INFO   // Supplemental Info
#define LIST_sdta   0x73647461  // Sample Binary Data Chunk
#define LIST_pdta   0x70647461  // Header Data Chunk

//INFO common types
#define INFO_ifil   0x6966696C  // SoundFont Version
#define INFO_isng   0x69736E67  // Target Sound Engine
//#define INFO_INAM INFO_INAM   // Title
#define INFO_irom   0x69726F6D  // Sound ROM Name
#define INFO_iver   0x69766572  // Sound ROM Version
//#define INFO_ICRD INFO_ICRD   // Creation Date
//#define INFO_IENG INFO_IENG   // Engineer(s)
//#define INFO_IPRD INFO_IPRD   // Target Product
//#define INFO_ICOP INFO_ICOP   // Copyright Information
//#define INFO_ICMT INFO_ICMT   // General Comment(s)
//#define INFO_ISFT INFO_ISFT   // Software Package

//SDTA common types
#define SDTA_smpl   0x736D706C  // Sample Binary Data (16bits)
#define SDTA_sm24   0x736D3234  // Sample Binary Data (8bits)

//PDTA common types
#define PDTA_phdr   0x70686472  // Preset Header
#define PDTA_pbag   0x70626167  // Preset Index List
#define PDTA_pmod   0x706D6F64  // Preset Modulator List
#define PDTA_pgen   0x7067656E  // Preset Generator List
#define PDTA_inst   0x696E7374  // Instrument Header
#define PDTA_ibag   0x69626167  // Instrument Index List
#define PDTA_imod   0x696D6F64  // Instrument Modulator List
#define PDTA_igen   0x6967656E  // Instrument Generator List
#define PDTA_shdr   0x73686472  // Sample Header


#endif
