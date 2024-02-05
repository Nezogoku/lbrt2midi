// Base codes in playmidi from schellingb
// Based off LBRTPlayer from owocek
//      Used for comparison stuffs

#include <algorithm>
#include <cstdio>
#include <string>
#include "lrt/lrt.hpp"
#include "printpause.hpp"

#define PROGRAMME_IDENTIFIER "lbrt2midi v6.0"


void printOpt(const char *pName) {
    fprintf(stderr, "Usage: %s [options] [<infile.sf2>] [<infile(s).lrt/mid>]\n\n", pName);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "   -h          Prints this message\n");
    fprintf(stderr, "   -d          Toggles debug mode\n");
    fprintf(stderr, "   -p          Activates playback mode\n");
    fprintf(stderr, "                   Uses most recent SF2 and all (converted) MIDI's\n");
}


int main(int argc, char *argv[]) {
    bool debug = false, play_result = false;

    std::string prgm = argv[0];
    prgm.erase(std::remove(prgm.begin(), prgm.end(), '\"'), prgm.end());
    prgm = prgm.substr(prgm.find_last_of("\\/") + 1);
    prgm = prgm.substr(0, prgm.find_last_of('.'));

    if (argc < 2) { printOpt(prgm.c_str()); }
    else {
        lbrt player;
        for (int i = 1; i < argc; ++i) {
            std::string tfle, ext, fle;

            tfle = argv[i];
            tfle.erase(std::remove(tfle.begin(), tfle.end(), '\"'), tfle.end());

            if (tfle == "-h") { printOpt(prgm.c_str()); break; }
            else if (tfle == "-d") { debug = !debug; player.setDebug(debug); continue; }
            else if (tfle == "-p") { play_result = true; continue; }
            else if (tfle.find('.') == std::string::npos) continue;

            ext = tfle.substr(tfle.find_last_of('.') + 1);
            fle = tfle.substr(0, tfle.find_last_of('.'));

            fprintf(stdout, "\n");
            if (debug) fprintf(stderr, "File base name: %s\n", fle.substr(fle.find_last_of("\\/") + 1).c_str());
            if (debug) fprintf(stderr, "File extension: %s\n", ext.c_str());

            bool isSF2 = (ext.find("sf2") != std::string::npos),
                 isLRT = (ext.find("lrt") != std::string::npos),
                 isMID = (ext.find("mid") != std::string::npos) ||
                         (ext.find("smf") != std::string::npos);

            if (isLRT || isMID) {
                if (debug) fprintf(stderr, "\nThis is a sequenced file\n");

                if (isMID) {
                    player.setSequence(tfle);
                    if (play_result) player.playSequence();
                }
                else {
                    if (!player.setLRT(tfle)) continue;
                    else if (!player.writeMidi()) continue;
                    else if (!play_result) continue;
                    else player.playSequence();
                }
            }
            else if (isSF2) {
                if (debug) fprintf(stderr, "\nThis is a soundbank file\n");
                player.setBank(tfle);
            }
        }
    }

    fprintf(stdout, "\nEnd of thing\n");

    sleep(10);
    return 0;
}
