#ifndef RIFF_FORMS_HPP
#define RIFF_FORMS_HPP

//RIFF forms
#define FOURCC_RIFF         0x52494646
#define FOURCC_RIFX         0x52494658

//RIFF common chunks
#define RIFF_LIST           0x4C495354  // List Chunk
#define RIFF_DISP           0x44495350  // Display Chunk
#define RIFF_CSET           0x43534554  // Character Set Chunk
#define RIFF_CTOC           0x43544F43  // Compound File Table of Contents Chunk
#define RIFF_CGRP           0x43475250  // Compound File Element Group Chunk
#define RIFF_JUNK           0x4A554E4B  // Junk Chunk
#define RIFF_FLLR           0x464C4C52  // Filler Chunk
#define RIFF_PAD            0x50414420  // Padding Chunk

//LIST common chunks
#define LIST_INFO           0x494E464F  // Supplemental Info

//INFO common types
#define INFO_IARL           0x4941524C  // Archival Location
#define INFO_IART           0x49415254  // Artist(s)
#define INFO_ICMS           0x49434D53  // Commissionor(s)
#define INFO_ICMT           0x49434D54  // General Comment(s)
#define INFO_ICOP           0x49434F50  // Copyright Information
#define INFO_ICRD           0x49435244  // Creation Date
#define INFO_ICRP           0x49435250  // Cropping Information
#define INFO_IDIM           0x4944494D  // Dimensions
#define INFO_IDIT           0x49444954  // Digitization Time
#define INFO_IDPI           0x49445049  // Dots Per Inch
#define INFO_IENG           0x49454E47  // Engineer(s)
#define INFO_IGNR           0x49474E52  // Genre
#define INFO_IKEY           0x494B4559  // Keyword(s)
#define INFO_ILGT           0x494C4754  // Lightness
#define INFO_IMED           0x494D4544  // Medium
#define INFO_INAM           0x494E414D  // Title
#define INFO_IPLT           0x49504C54  // Palette Setting
#define INFO_IPRD           0x49505244  // Target Product
#define INFO_ISBJ           0x4953424A  // Contents
#define INFO_ISFT           0x49534654  // Software Package
#define INFO_ISHP           0x49534850  // Sharpness
#define INFO_ISMP           0x49534D50  // SMPTE Time Code
#define INFO_ISRC           0x49535243  // Source or International Standardised Recording Code
#define INFO_ISRF           0x49535246  // Source Form
#define INFO_ITCH           0x49544348  // Technician
#define INFO_ITOC           0x49544F43  // Table of Contents
#define INFO_ITRK           0x4954524B  // Track Number

//DISP common types
#define DISP_TXTANSI        0x0001      // ANSI Text Format
#define DISP_BMP            0x0002      // Bitmap Format
#define DISP_METAPIX        0x0003      // Metafile Picture Format
#define DISP_SYMLINK        0x0004      // Symbolic Link Format
#define DISP_DIF            0x0005      // Data Interchange Format
#define DISP_TAGGEDIMAGE    0x0006      // Tagged-Image File Format
#define DISP_TXTOEM         0x0007      // OEM Text Format
#define DISP_DIB            0x0008      // Device Independent Bitmap Format
#define DISP_PAL            0x0009      // Colour Palette Format
#define DISP_PENEXTEND      0x000A      // Pen Extension Data Format
#define DISP_WAVEXTEND      0x000B      // Extended Waveform Audio Format
#define DISP_WAVSTANDARD    0x000C      // Standard Waveform Audio Format
#define DISP_TXTUNI         0x000D      // Unicode Text Format
#define DISP_ENHMETA        0x000E      // Enhanced Metafile Format
#define DISP_HDROP          0x000F      // HDROP Format
#define DISP_IDLOCALE       0x0010      // Locale Identifier Format
#define DISP_DIBV5          0x0011      // Device Independent Bitmap V5 Format
#define DISP_OWNER          0x0080      // Owner Display Chunk
#define DISP_TXTCUSTOM      0x0081      // Custom Text Format
#define DISP_BMPCUSTOM      0x0082      // Custom Bitmap Format
#define DISP_METAPIXCUSTOM  0x0083      // Custom Metafile Picture Format
#define DISP_ENHMETACUSTOM  0x008E      // Custom Enhanced Metafile Format
#define DISP_INTPRIVATE0    0x0200      // Private Integer Values Format (First)
#define DISP_INTPRIVATE1    0x02FF      // Private Integer Values Format (Last)
#define DISP_INTCUSTOM0     0x0300      // Custom Integer Values Format (First)
#define DISP_INTCUSTOM1     0x03FF      // Custom Integer Values Format (Last)


#endif
