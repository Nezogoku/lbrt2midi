# lbrt2midi
Converts sequence files from Patapon 1 and 2 to playable midi files and readable CSV files.
SGD converter currently being moved to sgd_extractor and is no longer included until further notice.
Patapon and common sequences can be listened to directly so long as they have an associated soundfont (SF2) file.

Included TinySoundFont library.

Heavily inspired by owocek's LBRTPlayer (now it basically IS the LBRTPlayer, woops...).

## How to use

`lbrt2midi` and `lbrt2midi -h` displays a help message

`lbrt2midi -d ...` toggles debug mode

`lbrt2midi -c infile(s).lrt` activates midicsv mode (following .mid are additionally converted to .csv)

`lbrt2midi -p infile.sf2 infile(s).lrt/mid` activates playback mode (uses .sf2 and .lrt/mid files for playback)
