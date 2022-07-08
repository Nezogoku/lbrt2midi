#ifdef __cplusplus
extern "C" {
#endif


#ifndef CSVMIDI_H
#define CSVMIDI_H

#define PROG "csvmidi"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#define strcasecmp _stricmp
#endif

#include "version.h"
#include "types.h"
#include "midifile.h"
#include "midio.h"
#include "csv.h"
//#include "getopt.h"

/*  List of comment delimiters recognised in CSV files.
    These are whole-line comments which must be marked
    by one of the following characters as the first
    nonblank character of a record.  Rest-of-line comments
    are not implemented.  Since the track number always
    begins a data record, any non-numeric character may
    be used as a comment delimiter.  */

#define COMMENT_DELIMITERS  "#;"

#define FALSE	0
#define TRUE	1

#define ELEMENTS(array) (sizeof(array)/sizeof((array)[0]))

/*  Codes for control items.  */

typedef enum {
    Header,
    Start_track,
    End_of_file
} controlMessages;

/*  The following table lists all possible item codes which may
    appear in a CSV-encoded MIDI file.	These should be listed
    in order of frequency of occurrence since the list is
    searched linearly.	*/

struct mitem {
    char *name;
    int icode;
};

#define EVENT	0
#define META	0x100
#define MARKER	0x200

#define Event(x)    ((x) | EVENT)
#define Meta(x)     ((x) | META)
#define Marker(x)   ((x) | MARKER)

static struct mitem mitems[] = {
    { "Note_on_c", Event(NoteOn) },
    { "Note_off_c", Event(NoteOff) },
    { "Pitch_bend_c", Event(PitchBend) },
    { "Control_c", Event(ControlChange) },
    { "Program_c", Event(ProgramChange) },
    { "Poly_aftertouch_c", Event(PolyphonicKeyPressure) },
    { "Channel_aftertouch_c", Event(ChannelPressure) },

    { "System_exclusive", Event(SystemExclusive) },
    { "System_exclusive_packet", Event(SystemExclusivePacket) },

    { "Sequence_number", Meta(SequenceNumberMetaEvent) },
    { "Text_t", Meta(TextMetaEvent) },
    { "Copyright_t", Meta(CopyrightMetaEvent) },
    { "Title_t", Meta(TrackTitleMetaEvent) },
    { "Instrument_name_t", Meta(TrackInstrumentNameMetaEvent) },
    { "Lyric_t", Meta(LyricMetaEvent) },
    { "Marker_t", Meta(MarkerMetaEvent) },
    { "Cue_point_t", Meta(CuePointMetaEvent) },
    { "Channel_prefix", Meta(ChannelPrefixMetaEvent) },
    { "MIDI_port", Meta(PortMetaEvent) } ,
    { "End_track", Meta(EndTrackMetaEvent) },
    { "Tempo", Meta(SetTempoMetaEvent) },
    { "SMPTE_offset", Meta(SMPTEOffsetMetaEvent) },
    { "Time_signature", Meta(TimeSignatureMetaEvent) },
    { "Key_signature", Meta(KeySignatureMetaEvent) },
    { "Sequencer_specific", Meta(SequencerSpecificMetaEvent) },
    { "Unknown_meta_event", Meta(0xFF) },

    { "Header", Marker(Header) },
    { "Start_track", Marker(Start_track) },
    { "End_of_file", Marker(End_of_file) },
};

static char *progname;		      /* Program name string */
static int verbose = FALSE; 	      /* Debug output */
static int zerotol = FALSE;    	      /* Any warning terminates processing */
static int errors = 0;	    	      /* Errors and warnings detected */
static char *s = NULL;	    	      /* Dynamically expandable CSV input buffer */

/*  OUTBYTE  --  Store byte in track buffer.  */

static byte of[10];
static byte *trackbuf = NULL, *trackbufp;
static int trackbufl;
static char *f = NULL;
static int flen = 0;

#define Warn(msg) { errors++; fprintf(stderr, "%s: Error on line %d:\n    %s\n  %s.\n", PROG, lineno, s, msg); if (zerotol) { exit(1); } }

static void outbyte(const byte c);

/*  OUTEVENT  --  Output event, optimising repeats.  */

static long abstime, tabstime = 0;
static void outVarLen(const vlint value);
static int optimiseStatus = TRUE, lastStatus = -1;

static void outevent(const byte c);

/*  OUTMETA  --  Output file meta-event.  */

static void outmeta(const byte c);

/*  OUTSHORT  --  Output two-byte value to track buffer.  */

static void outshort(const short v);

/*  OUTBYTES  --  Output a linear array of bytes.  */

static void outbytes(const byte *s, const int n);

/*  OUTVARLEN  --  Output variable length number.  */

static void outVarLen(const vlint v);

/*  XFIELDS  --  Parse one or more numeric fields.  Returns FALSE if
    	    	 all fields were scanned successfully, TRUE if an
		 error occurred.  The fbias argument gives the absolute
		 field number of the first field to be parsed; it is used
		 solely to identify fields in error messages.  */

#define MAX_NFIELDS  10

static int lineno = 0;
static long nfld[MAX_NFIELDS];
static char *csvline;

static int xfields(const int n, const int fbias);

/*  XFIELDS  --  Parse one or more numeric fields.  This is a
    	    	 wrapper for xfields which specifies the default
		 starting field of 4 used by most events.  */

static int nfields(const int n);

/*  CHECKBYTES  --  Pre-parse a sequence of arguments representing
    	    	    a byte vector.  If an error is detected, return
		    TRUE.  Otherwise, reset the CSV parser to the
		    first byte of the sequence and return FALSE.  This
		    permits code which handles arbitrary length byte
		    vectors to recover from syntax errors and ignore
		    the line prior to the irreversible step of emitting
		    the event type and length. */

static int checkBytes(const int fieldno, const int length);

/*  GETCSVLINE  --  Get next line from CSV file.  Reads into a dynamically
    	    	    allocated buffer s which is expanded as required to
		    accommodate longer lines.  All standard end of line
		    sequences are handled transparently, and the line
		    is returned with the end line sequence stripped
		    with a C string terminator (zero byte) appended.  */

static int getCSVline(FILE *fp);

/*  CLAMP  --  Constrain a numeric value to be within a specified
    	       inclusive range.  If the value is outside the range
	       an error message is issued and the value is forced to
	       the closest limit of the range.  */

static void clamp(long *value, const long minval, const long maxval, const char *fieldname);

/*  Main program  */

int csvmidi(char *inCSV, char *outMID);

#endif


#ifdef __cplusplus
}
#endif
