# lbrt2midi
Converts sequence files from Patapon 1 and 2 to playable midi files and readable CSV files.
Patapon and common sequences can be listened to directly so long as they have an associated soundfont (SF2) file.

Included TinySoundFont library.

Borrowed code from schellingb (mid/playmidi) with some minor changes.

Heavily inspired by owocek's LBRTPlayer (now it basically IS the LBRTPlayer, woops...).


    Usage: lbrt2midi [options] [<infile.sf2>] [<infile(s).lrt/mid>]
        Options:
            -h          Prints this message
            -d          Toggles debug mode
            -p          Toggles playback mode
                            Uses most recent SF2 and all (converted) MIDI's
