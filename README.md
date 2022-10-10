# lbrt2midi
Converts sequence files from Patapon 1 and 2 to playable midi files and readable CSV files.
Additionally takes SGD files and converts them to SF2 files (theoretically converts all mono samples).
Sequences embedded in SGD files are converted to midi files.
Patapon and common sequences can be listened to directly so long as they have an associated soundfont.

Requires gocha's SF2cute library to build.
Included stb_vorbis library.

Borrowed code from jmarti856 and vgmstream (decode/vag) with some minor changes.
Borrowed code from schellingb (mid/playmidi) with some minor changes.

Heavily inspired by owocek's LBRTPlayer (now it basically IS the LBRTPlayer, woops...).


    Usage: lbrt2midi [options] [<infile(s).sgd/sf2>] [<infile(s).lrt/mid>]
        Options:
            -h          Prints this message
            -d          Toggles debug mode
            -p          Toggles playback mode
                            Uses last (converted) SF2 and all (converted) MIDI's
