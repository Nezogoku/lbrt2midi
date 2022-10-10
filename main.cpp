#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
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

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;


void printOpt(string pName, bool debug) {
    if (debug) cerr << "Not enough valid files entered\n" << endl;
    cout << "Usage: " << pName << " [options] [<infile(s).sgd/sf2>] [<infile(s).lrt/mid>]\n"
         << "\nOptions:\n"
         << "\t-h       Prints this message\n"
         << "\t-d       Toggles debug mode\n"
         << "\t-p       Toggles playback mode\n"
         << "\t             Uses last (converted) SF2 and all (converted) MIDI's\n"
         //<< "\t             If no SF2 specified, uses default soundbank\n"
         << endl;
}

int setInput(string ext, string &temp, bool debug) {
    if (temp.substr(temp.find_last_of('.') + 1) != ext) {
        return 1;
    }
    else {
        if (debug) cout << ext << " file entered is " << temp << endl;
        return 0;
    }
}

bool fileExists(string file) {
    bool exists = false;

    std::ifstream test(file);
    if (test.is_open()) {
        test.close();
        exists = true;
    }

    return exists;
}


int main(int argc, char *argv[]) {
    bool debug = false,
         play_result = false;

    string prgm = argv[0];
    prgm.erase(std::remove(prgm.begin(), prgm.end(), '\"'), prgm.end());
    prgm = prgm.substr(prgm.find_last_of("\\/") + 1);
    prgm = prgm.substr(0, prgm.find_last_of('.'));

    vector<string> playables(1);

    if (argc < 2) {
        printOpt(prgm, debug);
        return 0;
    }
    else {
        string tempFile = "";

        for (int i = 1; i < argc; ++i) {
            tempFile = argv[i];
            tempFile.erase(std::remove(tempFile.begin(), tempFile.end(), '\"'), tempFile.end());
            if (debug) cout << endl;

            if (tempFile == "-h") {
                printOpt(prgm, debug);
                return 0;
            }
            else if (tempFile == "-d") {
                debug = !debug;
                continue;
            }
            else if (tempFile == "-p") {
                play_result = !play_result;
                continue;
            }


            if (!(tempFile.find_last_of("sgd") == string::npos) && !setInput("sgd", tempFile, debug)) {
                if (!setSF2(tempFile, playables[0], debug)) cerr << "Unable to set SF2" << endl;
                else cout << "SF2 file was successfully saved" << endl;
                continue;
            }
            else if (!(tempFile.find_last_of("sgh") == string::npos) && !setInput("sgh", tempFile, debug)) {
                string body = tempFile.substr(0, tempFile.find_last_of('.')) + ".sgb",
                       headBody = tempFile.substr(0, tempFile.find_last_of('.')) + ".sgd";

                if (fileExists(headBody)) continue;

                if (!setSF2(tempFile, body, playables[0], debug)) cerr << "Unable to set SF2" << endl;
                else cout << "SF2 file was successfully saved" << endl;
                continue;
            }
            else if (!(tempFile.find_last_of("sgb") == string::npos) && !setInput("sgb", tempFile, debug)) {
                string head = tempFile.substr(0, tempFile.find_last_of('.')) + ".sgh",
                       headBody = tempFile.substr(0, tempFile.find_last_of('.')) + ".sgd";

                if (fileExists(headBody)) continue;

                if (!setSF2(head, tempFile, playables[0], debug)) cerr << "Unable to set SF2" << endl;
                else cout << "SF2 file was successfully saved" << endl;
                continue;
            }
            else if (!(tempFile.find_last_of("sf2") == string::npos) && !setInput("sf2", tempFile, debug)) {
                if (fileExists(tempFile)) {
                    playables[0] = tempFile;
                    cout << "SF2 file was successfully loaded" << endl;
                }
                continue;
            }
            else if (!(tempFile.find_last_of("lrt") == string::npos) && !setInput("lrt", tempFile, debug)) {
                string endMid = "";

                if (!readLRT(tempFile, endMid, debug)) {
                    cerr << "Unable to convert LRT file to MID file" << endl;
                }
                else {
                    cout << "MID file was successfully saved" << endl;
                    playables.push_back(endMid);
                }
                continue;
            }
            else if (!(tempFile.find_last_of("mid") == string::npos) && !setInput("mid", tempFile, debug)) {
                if (fileExists(tempFile)) {
                    playables.push_back(tempFile);
                    cout << "MID file was successfully loaded" << endl;
                }
                continue;
            }
            else continue;
        }


        if (play_result) {
            cout << endl;
            for (int p = 1; p < playables.size(); ++p) {
                playmidi player(debug);

                player.setBank(playables[0]);
                player.setSequence(playables[p]);
                player.playSequence();
            }
        }
    }

    return 0;
}
