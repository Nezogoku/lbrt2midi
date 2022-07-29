# lbrt2midi
Converts sequence files from Patapon 1 and 2 to playable midi files.
Additionally takes SGD files and converts them to SF2 files (currently works on most SGD files).
Sequences embedded in SGD files are also converted to midi files.

Requires gocha's SF2cute library to build.

Borrowed code from jmarti856 (decode) with some minor changes.

Heavily inspired by owocek's LBRTPlayer.

Have to pin down where volume of played notes are stored (trial and error it out).
