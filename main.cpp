#include <algorithm>
#include <cstdio>
#include <string>
#include "soundbank/sf2/sf2.hpp"
#include "soundbank/psx_sgd/psxsgd.hpp"
#include "sequence/lrt/lrt.hpp"
#include "sequence/mid/playmidi.hpp"

// SF2cute library from gocha
// TinySoundFont library and base codes in sequence/mid from schellingb
// Based off LBRTPlayer from owocek
//      Used for comparison stuffs
// Base codes in decode/vag from jmarti856 (I basically just changed the language it was written in)
//                               and vgmstream (rewritten to resemble jmarti856's)
// stb_vorbis library and base codes in decode/ogg from Sean Barrett


void printOpt(std::string pName, bool debug) {
    if (debug) fprintf(stderr, "Not enough valid files entered\n");

    fprintf(stderr, "Usage: %s [options] [<infile(s).sgd/sf2>] [<infile(s).lrt/mid>]\n\n", pName.c_str());
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "   -h          Prints this message\n");
    fprintf(stderr, "   -d          Toggles debug mode\n");
    fprintf(stderr, "   -p          Activates playback mode\n");
    fprintf(stderr, "                   Uses last (converted) SF2 and all (converted) MIDI's\n");
}


int main(int argc, char *argv[]) {
    bool debug = false,
         play_result = false;

    std::string prgm = argv[0];
    prgm.erase(std::remove(prgm.begin(), prgm.end(), '\"'), prgm.end());
    prgm = prgm.substr(prgm.find_last_of("\\/") + 1);
    prgm = prgm.substr(0, prgm.find_last_of('.'));

    std::string playables[argc] = {};

    if (argc < 2) {
        printOpt(prgm, debug);
        return 0;
    }
    else {
        std::string tempFile = "";

        for (int i = 1, p = 1; i < argc; ++i) {
            tempFile = argv[i];
            tempFile.erase(std::remove(tempFile.begin(), tempFile.end(), '\"'), tempFile.end());

            if (tempFile == "-h") {
                printOpt(prgm, debug);
                return 0;
            }
            else if (tempFile == "-d") {
                debug = !debug;
                continue;
            }
            else if (tempFile == "-p") {
                play_result = true;
                continue;
            }


            std::string ext = tempFile.substr(tempFile.size() - 4),
                        fle = tempFile.substr(0, tempFile.find_last_of('.'));

            if (debug) fprintf(stdout, "\nFile base name: %s\n", fle.substr(fle.find_last_of("\\/") + 1).c_str());
            if (debug) fprintf(stdout, "File extension: %s\n", ext.substr(ext.find_last_of('.') + 1).c_str());

            bool isSGD = (ext == ".sgd"),
                 isSGH = (ext == ".sgh"),
                 isSGB = (ext == ".sgb"),
                 isSF2 = (ext == ".sf2"),
                 isLRT = (ext == ".lrt"),
                 isMID = (ext == ".mid") || (ext == ".smf");

            if (isLRT || isMID) {
                if (debug) fprintf(stdout, "This is a sequenced file\n");

                std::string endMid;

                if (isMID) endMid = tempFile;
                else if (isLRT && readLRT(tempFile, endMid, debug)) {}
                else continue;

                int cnt = std::count(playables + 1, playables + argc, endMid);
                if (!cnt) {
                    playables[p++] = endMid;
                    fprintf(stdout, "Sequence successfully saved\n");
                }
            }
            else if (isSGD || isSGH || isSGB || isSF2) {
                if (debug) fprintf(stdout, "This is a soundbank file\n");

                std::string sgh = fle + ".sgh", sgb = fle + ".sgb";

                if (isSF2) playables[0] = tempFile;
                else if (isSGD && setSF2(tempFile, debug)) playables[0] = tempFile;
                else if ((isSGH || isSGB) && setSF2(sgh, sgb, debug)) playables[0] = sgh;
                else continue;
            }
        }


        if (play_result) {
            fprintf(stdout, "\nPlaying sequences\n");

            playmidi player[argc] {};
            for (int p = 1, s = 0; p < argc; ++p, ++s) {
                if (playables[p].empty()) continue;

                player[s] = playmidi(debug);

                if (!player[s].setBank(playables[0])) break;
                else if (!player[s].setSequence(playables[p])) continue;

                player[s].playSequence();
            }
        }
    }

    return 0;
}
