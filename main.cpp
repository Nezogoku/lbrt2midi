// Base codes in playmidi from schellingb
// Based off LBRTPlayer from owocek
//      Used for comparison stuffs

#include <algorithm>
#include <cstdio>
#include <string>
#define SOUNDBANKSGXD_IMPLEMENTATION
#define PROGRAMME_IDENTIFIER "lbrt2mid v8.0"
#include "lrt/lrt_func.hpp"
#include "lrt/sgxd_func.hpp"
#include "playmidi/playmidi_func.hpp"
#include "printpause.hpp"


void printOpt(const char *pName) {
    fprintf(stderr, "Usage: %s [-hdcp] [<infile.sf2>] [<infile(s).lrt/mid>]\n\n", pName);
    fprintf(stderr, "Options:\n");
    fprintf(stderr, "   -h          Prints this message\n");
    fprintf(stderr, "   -d          Toggles debug mode\n");
    fprintf(stderr, "   -c          Activates midicsv mode\n");
    fprintf(stderr, "                   Additionally converts MIDI's to CSV's\n");
    fprintf(stderr, "   -p          Activates playback mode\n");
    fprintf(stderr, "                   Uses most recent SF2 and all (converted) MIDI's\n");
}

int main(int argc, char *argv[]) {
    bool debug = false, play = false;

    std::string prgm = argv[0];
    prgm.erase(std::remove(prgm.begin(), prgm.end(), '\"'), prgm.end());
    prgm = prgm.substr(prgm.find_last_of("\\/") + 1);
    prgm = prgm.substr(0, prgm.find_last_of('.'));

    if (argc < 2) { printOpt(prgm.c_str()); }
    else {
        std::string sgh, sgb, tfle;
        auto get_sgd = [&](const char *s0, const char *s1 = 0) -> void {
            sgd_debug = debug;
            unpackSgxd(s0, s1);

            std::string pth = s0;
            pth = pth.substr(0, pth.find_last_of("\\/") + 1);
            extractSgxd(pth.c_str());
            
            a_tml.sf2 = pth + "@" + sgd_inf.file + "/rgnd/" + sgd_inf.file + ".sf2";
            
            tfle.clear(); sgh.clear(); sgb.clear();
        };
        
        for (int i = 1; i < argc; ++i) {
            std::string ext, fle, rot;

            tfle = argv[i];
            tfle.erase(std::remove(tfle.begin(), tfle.end(), '\"'), tfle.end());

            if (tfle == "-h") { printOpt(prgm.c_str()); break; }
            else if (tfle == "-d") { debug = !debug; continue; }
            else if (tfle == "-c") { lrt_midicsv = true; continue; }
            else if (tfle == "-p") { play = true; continue; }

            if (debug) fprintf(stderr, "\n");
            if (tfle.rfind(".") == std::string::npos) tfle += ".unknown";
            rot = tfle.substr(0, tfle.find_last_of("\\/") + 1);
            fle = tfle.substr(0, tfle.find_last_of('.'));
            fle = fle.substr(fle.find_last_of("\\/") + 1);
            ext = tfle.substr(tfle.find_last_of('.') + 1);

            if (debug) fprintf(stderr, "File root: %s\n", rot.c_str());
            if (debug) fprintf(stderr, "File base name: %s\n", fle.c_str());
            if (debug) fprintf(stderr, "File extension: %s\n", ext.c_str());

            bool isSGD = (ext.find("sgd") != std::string::npos),
                 isSGH = (ext.find("sgh") != std::string::npos),
                 isSGB = (ext.find("sgb") != std::string::npos),
                 isSF2 = (ext.find("sf2") != std::string::npos),
                 isLRT = (ext.find("lrt") != std::string::npos),
                 isMID = (ext.find("mid") != std::string::npos) ||
                         (ext.find("smf") != std::string::npos);

            if (isLRT || isMID) {
                if (debug) fprintf(stderr, "This is a sequenced file\n");

                if (isLRT) {
                    lrt_debug = debug;
                    unpackLrt(tfle.c_str());
                    extractLrt();
                    a_tml.mid.push_back(tfle.replace(tfle.rfind(ext), 4, "mid"));
                }
                else a_tml.mid.push_back(tfle);
            }
            else if (isSGD || isSGH || isSGB || isSF2) {
                if (isSF2) {
                    if (debug) fprintf(stderr, "This is a soundbank file\n");
                    a_tml.sf2 = tfle;
                    continue;
                }
                else if (isSGD) {
                    if (debug) fprintf(stderr, "This is a game data archive file\n");
                    get_sgd(tfle.c_str());
                    continue;
                }
                else if (isSGH) {
                    if (debug) fprintf(stderr, "This is a game data archive header file\n");
                    sgh = tfle;
                    if (sgb.empty()) continue;
                }
                else if (isSGB) {
                    if (debug) fprintf(stderr, "This is a game data archive body file\n");
                    sgb = tfle;
                    if (sgh.empty()) continue;
                }
                
                get_sgd(sgh.c_str(), sgb.c_str());
            }
            else if (debug) fprintf(stderr, "This is an unknown file\n");
        }

        if (play) {
            if (debug) fprintf(stderr, "\n");
            playmidi_debug = debug;
            playSequence();
        }
    }

    fprintf(stdout, "\nEnd of thing\n");

    sleep(10);
    return 0;
}
