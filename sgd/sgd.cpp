#include <algorithm>
#include <cmath>
#include <iostream>
#include <fstream>
#include "sgd.hpp"
#include "decode/vag.hpp"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    #include <winsock2.h>
#else
    #include <arpa/inet.h>
#endif


using std::fstream;
using std::ifstream;
using std::cout;
using std::cerr;
using std::dec;
using std::hex;
using std::endl;
using std::ios;
using std::remove;
using std::string;
using std::to_string;
using std::vector;


sgd::sgd() {
    isDebug = false;
}

sgd::sgd(bool isDebug) {
    this->isDebug = isDebug;
}


int sgd::setInfo() {
    ifstream inFile(fileName, ios::binary);
    if (!inFile.is_open()) {
        cerr << "Unable to open this file" << endl;
        return 0;
    }

    numRiff = 0;
    uint32_t working_offset, chunk;

    //Get chunk type SGXD
    working_offset = 0x0;
    inFile.seekg(working_offset);
    inFile.read(reinterpret_cast<char*>(&chunk), sizeof(uint32_t));
    sgxdHead.chunktype = htonl(chunk);

    if (sgxdHead.chunktype != SGXD) {
        cerr << "This is not an sgd file" << endl;
        inFile.close();
        return 0;
    }

    hasRGND = false;
    hasSEQD = false;
    hasWAVE = false;
    hasNAME = false;
    hasRIFF = false;

    //Get offset of bank name
    working_offset += 0x4;
    inFile.seekg(working_offset);
    inFile.read(reinterpret_cast<char*>(&sgxdHead.name_offset), sizeof(uint32_t));
    if (isDebug) cout << "Offset to Bank name: 0x" << hex << sgxdHead.name_offset << dec << endl;

    //Get offset of data
    working_offset += 0x4;
    inFile.seekg(working_offset);
    inFile.read(reinterpret_cast<char*>(&sgxdHead.data_offset), sizeof(uint32_t));
    if (isDebug) cout << "Offset to Bank audio: 0x" << hex << sgxdHead.data_offset << dec << endl;

    //Get size of data + 0x80000000
    working_offset += 0x4;
    inFile.seekg(working_offset);
    inFile.read(reinterpret_cast<char*>(&sgxdHead.length), sizeof(uint32_t));
    if (isDebug) cout << "Size of Bank audio (padding): 0x" << hex << sgxdHead.length << dec << endl;
    if (isDebug) cout << "Size of Bank audio (no padding): 0x" << hex << sgxdHead.length - 0x80000000 << dec << endl;

    working_offset += 0x4;

    //Search file for specific types
    while (working_offset < sgxdHead.length - 0x80000000) {
        inFile.seekg(working_offset);
        inFile.read(reinterpret_cast<char*>(&chunk), sizeof(uint32_t));
        chunk = htonl(chunk);

        if (chunk == RGND) {
            hasRGND = true;
            rgndHead.chunktype = chunk;
            if (isDebug) cout << "\nHas RGND section" << endl;

            //Get size of chunk RGND
            working_offset += 0x4;
            inFile.seekg(working_offset);
            inFile.read(reinterpret_cast<char*>(&rgndHead.length), sizeof(uint32_t));
            if (isDebug) cout << "Size of RGND section: 0x" << hex << rgndHead.length << dec << endl;

            //Get reserved bytes
            working_offset += 0x4;
            inFile.seekg(working_offset);
            inFile.read(reinterpret_cast<char*>(&rgndHead.reserved), sizeof(uint32_t));
            rgndOffset = working_offset;

            //Get amount of pre-definitions of RGND
            working_offset += 0x4;
            inFile.seekg(working_offset);
            inFile.read(reinterpret_cast<char*>(&rgndHead.amount_pre), sizeof(uint32_t));
            if (isDebug) cout << "Number of RGND bank programs: " << rgndHead.amount_pre << endl;

            //Get address of pre-definitions of RGND
            working_offset += 0x4;
            rgndHead.addr_pre_start = working_offset;
            if (isDebug) cout << "Offset to RGND bank programs: 0x" << hex << rgndHead.addr_pre_start << dec << endl;

            //Calculate address of definitions of RGND
            working_offset = rgndHead.addr_pre_start + (rgndHead.amount_pre * 0x8);
            rgndHead.addr_start = working_offset;
            if (isDebug) cout << "Offset to RGND bank definitions: 0x" << hex << rgndHead.addr_start << dec << endl;

            //Calculate amount of definitions of RGND
            rgndHead.amount = (rgndOffset + rgndHead.length) - rgndHead.addr_start;
            rgndHead.amount = std::floor(double(rgndHead.amount) / 0x38);
            if (isDebug) cout << "Number of RGND bank definitions: " << rgndHead.amount << endl;
            rgndBank.resize(rgndHead.amount);

            working_offset = rgndOffset + rgndHead.length;
        }
        else if (chunk == SEQD) {
            hasSEQD = true;
            seqdHead.chunktype = chunk;
            if (isDebug) cout << "\nHas SEQD section" << endl;

            //Get size of chunk SEQD
            working_offset += 0x4;
            inFile.seekg(working_offset);
            inFile.read(reinterpret_cast<char*>(&seqdHead.length), sizeof(uint32_t));
            if (isDebug) cout << "Size of SEQD section: 0x" << hex << seqdHead.length << dec << endl;

            //Get reserved bytes
            working_offset += 0x4;
            inFile.seekg(working_offset);
            inFile.read(reinterpret_cast<char*>(&seqdHead.reserved), sizeof(uint32_t));
            seqdOffset = working_offset;

            //Get address of first set of instructions
            working_offset += 0x4;
            seqdHead.addr_start = working_offset;
            if (isDebug) cout << "Offset to SEQD definitions: 0x" << hex << seqdHead.addr_start << dec << endl;

            working_offset = seqdOffset + seqdHead.length;
        }
        else if (chunk == WAVE && !hasRIFF) {
            hasWAVE = true;
            waveHead.chunktype = chunk;
            if (isDebug) cout << "\nHas WAVE section" << endl;

            //Get size of chunk WAVE
            working_offset += 0x4;
            inFile.seekg(working_offset);
            inFile.read(reinterpret_cast<char*>(&waveHead.length), sizeof(uint32_t));
            if (isDebug) cout << "Size of WAVE section: 0x" << hex << waveHead.length << dec << endl;

            //Get reserved bytes
            working_offset += 0x4;
            inFile.seekg(working_offset);
            inFile.read(reinterpret_cast<char*>(&waveHead.reserved), sizeof(uint32_t));
            waveOffset = working_offset;

            //Get amount of audio definitions of WAVE
            working_offset += 0x4;
            inFile.seekg(working_offset);
            inFile.read(reinterpret_cast<char*>(&waveHead.amount), sizeof(uint32_t));
            waveBank.resize(waveHead.amount);
            vagBank.resize(waveHead.amount);
            pcmLoopData.resize(waveHead.amount);
            nameBank.resize(waveHead.amount);
            if (isDebug) cout << "Number of WAVE definitions: " << waveHead.amount << endl;

            //Get address of audio definitions of WAVE
            working_offset += 0x4;
            waveHead.addr_start = working_offset;
            if (isDebug) cout << "Offset to WAVE definitions: 0x" << hex << waveHead.addr_start << dec << endl;

            working_offset = waveOffset + waveHead.length;
        }
        else if (chunk == NAME) {
            hasNAME = true;
            nameHead.chunktype = chunk;
            if (isDebug) cout << "\nHas NAME section" << endl;

            //Get size of chunk NAME
            working_offset += 0x4;
            inFile.seekg(working_offset);
            inFile.read(reinterpret_cast<char*>(&nameHead.length), sizeof(uint32_t));
            if (isDebug) cout << "Size of NAME section: 0x" << hex << nameHead.length << dec << endl;

            //Get reserved bytes
            working_offset += 0x4;
            inFile.seekg(working_offset);
            inFile.read(reinterpret_cast<char*>(&nameHead.reserved), sizeof(uint32_t));
            nameOffset = working_offset;

            //Get amount of definitions/names of NAME
            working_offset += 0x4;
            inFile.seekg(working_offset);
            inFile.read(reinterpret_cast<char*>(&nameHead.amount), sizeof(uint32_t));
            if (isDebug) cout << "Number of NAME definitions: " << nameHead.amount << endl;

            //Get address of definitions of NAME
            working_offset += 0x4;
            nameHead.addr_start = working_offset;
            if (isDebug) cout << "Offset to NAME definitions: 0x" << hex << nameHead.addr_start << dec << endl;
        }
        else if (chunk == RIFF) {
            hasRIFF = true;
            if (isDebug) cout << "\nHas RIFF section" << endl;
            riffOffsets.push_back(working_offset);
            working_offset += 0x4;
            numRiff += 1;
        }
        else working_offset += 0x4;
    }

    if (isDebug) cout << endl;

    if (!hasRGND) {
        cerr << "This sgd file is not a sound bank" << endl;
        inFile.close();
        return 0;
    }
    else if (!hasNAME or !hasWAVE) {
        cerr << "This sgd file is not compatible with the current programme" << endl;
        inFile.close();
        return 0;
    }

    setRGND(inFile);
    if (hasSEQD) setSEQD(inFile);
    setBankName(inFile);

    for (int i = 0; i < waveHead.amount; ++i) {
        setWAVE(inFile, i);
        setNAME(inFile, i);
        setDATA(inFile, i);
    }

    inFile.close();

    return 1;
}

void sgd::setFile(std::string tmpFile) {
    tmpFile.erase(remove(tmpFile.begin(), tmpFile.end(), '\"'), tmpFile.end());
    fileName = tmpFile;
}

int sgd::setBankName(std::ifstream &tmpData) {
    char *buffer = new char[0x50];

    ///Retrieving bank name from file
    tmpData.seekg(sgxdHead.name_offset);
    tmpData.read(buffer, 0x50);
    bankName = buffer;
    bankName = bankName.substr(0, bankName.find_first_of('\0'));

    delete[] buffer;

    return 1;
}

int sgd::setRGND(std::ifstream &tmpData) {
    uint32_t working_offset;

    uint8_t bank,
            preset;

    //Retrieve pre-definitions
    if (isDebug) cout << "Number of pre definitions: " << rgndHead.amount_pre << endl;
    vector<uint32_t> offsets(rgndHead.amount_pre);

    for (int d = 0; d < offsets.size(); ++d) {
        working_offset = rgndHead.addr_pre_start + (0x08 * d);
        working_offset += 0x04;
        tmpData.seekg(working_offset);
        tmpData.read(reinterpret_cast<char*>(&offsets[d]), sizeof(uint32_t));
        if (isDebug) cout << "Offset to program section: 0x" << hex << offsets[d] << dec << endl;
    }

    for (int d = 0; d < rgndHead.amount; ++d) {
        working_offset = rgndHead.addr_start + (0x38 * d);

        for (int i = 0; i < offsets.size(); ++i) {
            if (offsets[i] == working_offset) {
                bank = 0x00;
                preset = i;

                if (isDebug) cout << "Bank and Preset of region " << d << ": " << int(rgndBank[d].program.bank) << ' ' << int(rgndBank[d].program.preset) << endl;
                break;
            }
        }

        ///Retrieving region info from vector
        tmpData.seekg(working_offset + 0x00);
        tmpData.read(reinterpret_cast<char*>(&rgndBank[d].determinator), sizeof(uint32_t));

        tmpData.seekg(working_offset + 0x18);
        tmpData.read(reinterpret_cast<char*>(&rgndBank[d].range_low), sizeof(uint8_t));

        tmpData.seekg(working_offset + 0x19);
        tmpData.read(reinterpret_cast<char*>(&rgndBank[d].range_high), sizeof(uint8_t));

        tmpData.seekg(working_offset + 0x1C);
        tmpData.read(reinterpret_cast<char*>(&rgndBank[d].root_key), sizeof(uint8_t));

        tmpData.seekg(working_offset + 0x1D);
        tmpData.read(reinterpret_cast<char*>(&rgndBank[d].value5), sizeof(uint16_t));

        tmpData.seekg(working_offset + 0x34);
        tmpData.read(reinterpret_cast<char*>(&rgndBank[d].sample_id), sizeof(uint32_t));

        rgndBank[d].program.bank = bank;
        rgndBank[d].program.preset = preset;
    }

    return 1;
}

int sgd::setSEQD(std::ifstream &tmpData) {
    bool stillRunning = true;
    uint32_t working_offset, temp_offset;
    uint32_t term_offset = seqdOffset + seqdHead.length;

    working_offset = seqdOffset;

    ///Retrieving sequence data from file
    while (true) {
        working_offset += 0x04;
        tmpData.seekg(working_offset);
        tmpData.read(reinterpret_cast<char*>(&temp_offset), sizeof(uint32_t));

        if (temp_offset > nameOffset && temp_offset < (nameOffset + nameHead.length)) break;
    } //Screw it, I'm skipping all of the jumping

    while (stillRunning) {
        seqd tempSeqd;
        char *buffer = new char[0x50];

        tmpData.seekg(temp_offset);
        tmpData.read(buffer, 0x50);

        tempSeqd.name = buffer;
        tempSeqd.name = tempSeqd.name.substr(0, tempSeqd.name.find_first_of('\0'));
        tempSeqd.name_offset = temp_offset;
        if (isDebug) cout << "Name of current sequence: " << tempSeqd.name << endl;

        delete[] buffer;

        working_offset += 0x04;
        while (true) {
            char buff;

            tmpData.seekg(working_offset);
            tmpData.read(reinterpret_cast<char*>(&temp_offset), sizeof(uint32_t));
            tmpData.read(&buff, sizeof(uint8_t));

            if (htonl(temp_offset) == WAVE) {
                stillRunning = false;
                break;
            }
            else if (temp_offset > nameOffset && temp_offset < (nameOffset + nameHead.length)) {
                break;
            }
            else {
                tempSeqd.sequence.push_back(buff);
                working_offset += 0x01;
            }
        }

        seqdBank.push_back(tempSeqd);
    }

    seqdHead.amount = seqdBank.size();

    return 1;
}

int sgd::setWAVE(std::ifstream &tmpData, int samp) {
    uint32_t working_offset;

    working_offset = waveHead.addr_start + (0x38 * samp);

    ///Retrieving sample info from file
    tmpData.seekg(working_offset);
    tmpData.read(reinterpret_cast<char*>(&waveBank[samp].reserved0), sizeof(uint32_t));
    working_offset += 0x4;

    tmpData.seekg(working_offset);
    tmpData.read(reinterpret_cast<char*>(&waveBank[samp].name_offset), sizeof(uint32_t));
    working_offset += 0x4;

    tmpData.seekg(working_offset);
    tmpData.read(reinterpret_cast<char*>(&waveBank[samp].codec), sizeof(uint8_t));
    working_offset += 0x1;

    tmpData.seekg(working_offset);
    tmpData.read(reinterpret_cast<char*>(&waveBank[samp].channels), sizeof(uint8_t));
    working_offset += 0x1;

    tmpData.seekg(working_offset);
    tmpData.read(reinterpret_cast<char*>(&waveBank[samp].reserved1), sizeof(uint16_t));
    working_offset += 0x2;

    tmpData.seekg(working_offset);
    tmpData.read(reinterpret_cast<char*>(&waveBank[samp].sample_rate), sizeof(uint32_t));
    working_offset += 0x4;

    tmpData.seekg(working_offset);
    tmpData.read(reinterpret_cast<char*>(&waveBank[samp].info_type), sizeof(uint32_t));
    working_offset += 0x4;

    tmpData.seekg(working_offset);
    tmpData.read(reinterpret_cast<char*>(&waveBank[samp].info_value), sizeof(uint32_t));
    working_offset += 0x4;

    tmpData.seekg(working_offset);
    tmpData.read(reinterpret_cast<char*>(&waveBank[samp].reserved2), sizeof(uint32_t));
    working_offset += 0x4;

    tmpData.seekg(working_offset);
    tmpData.read(reinterpret_cast<char*>(&waveBank[samp].reserved3), sizeof(uint32_t));
    working_offset += 0x4;

    tmpData.seekg(working_offset);
    tmpData.read(reinterpret_cast<char*>(&waveBank[samp].sample_size), sizeof(uint32_t));
    working_offset += 0x4;

    tmpData.seekg(working_offset);
    tmpData.read(reinterpret_cast<char*>(&waveBank[samp].loop_start), sizeof(uint32_t));
    pcmLoopData[samp].lpStrt = waveBank[samp].loop_start;
    working_offset += 0x4;

    tmpData.seekg(working_offset);
    tmpData.read(reinterpret_cast<char*>(&waveBank[samp].loop_end), sizeof(uint32_t));
    pcmLoopData[samp].lpEnd = waveBank[samp].loop_end;
    working_offset += 0x4;

    tmpData.seekg(working_offset);
    tmpData.read(reinterpret_cast<char*>(&waveBank[samp].stream_size), sizeof(uint32_t));
    working_offset += 0x4;

    tmpData.seekg(working_offset);
    tmpData.read(reinterpret_cast<char*>(&waveBank[samp].stream_offset), sizeof(uint32_t));
    working_offset += 0x4;

    tmpData.seekg(working_offset);
    tmpData.read(reinterpret_cast<char*>(&waveBank[samp].stream_size_full), sizeof(uint32_t));
    working_offset += 0x4;

    return 1;
}

int sgd::setNAME(std::ifstream &tmpData, int samp) {
    uint32_t working_offset;

    string tempName;

    working_offset = waveBank[samp].name_offset;

    //Assign sample name
    if (waveBank[samp].name_offset == 0x0) {
        tempName = "samp_" + to_string(samp);
    }
    else {
        char *buffer = new char[0x50];

        tmpData.seekg(working_offset);
        tmpData.read(buffer, 0x50);
        tempName = buffer;
        tempName = tempName.substr(0, tempName.find_first_of('\0'));

        delete[] buffer;
    }

    nameBank[samp] = tempName;

    return 1;
}

int sgd::setDATA(std::ifstream &tmpData, int samp) {
    tmpData.seekg(sgxdHead.data_offset + waveBank[samp].stream_offset);

    for (int i = 0; i < waveBank[samp].stream_size; ++i) {
        char buff;
        tmpData.get(buff);
        vagBank[samp].push_back(buff);
    }

    return 1;
}


vector<int16_t> sgd::getPCM(int samp) {
    if (samp > waveHead.amount || samp < 0) {
        cerr << samp << " is outside the range of existing audio samples" << endl;
        return {};
    }
    else if (waveBank[samp].codec != 0x03) {
        cerr << samp << " is not allowed at this time" << endl;
        return {};
    }

    //Return new 16-bit PCM data
    return vagDecode(vagBank[samp], pcmLoopData[samp].lpStrt, pcmLoopData[samp].lpEnd);
}

int sgd::getAmountAudio() {
    return waveHead.amount;
}

int sgd::getAmountDefinitions() {
    return rgndHead.amount;
}

std::string sgd::getBankName() {
    return bankName;
}

std::string sgd::getSampleName(int samp) {
    return nameBank[samp];
}

int sgd::getSampleCodec(int samp) {
    return waveBank[samp].codec;
}

int sgd::getSampleChannels(int samp) {
    return waveBank[samp].channels;
}

int sgd::getSampleSamplerate(int samp) {
    return waveBank[samp].sample_rate;
}

uint32_t sgd::getSampleSampleSize(int samp) {
    return waveBank[samp].sample_size;
}

bool sgd::sampleIsLoop(int samp) {
    return !(waveBank[samp].loop_start == 0xFFFFFFFF && waveBank[samp].loop_end == 0xFFFFFFFF);
}

uint32_t sgd::getSampleLpStrt(int samp) {
    return pcmLoopData[samp].lpStrt;
}

uint32_t sgd::getSampleLpTrm(int samp) {
    return pcmLoopData[samp].lpEnd;
}

uint32_t sgd::getSampleID(int def) {
    return rgndBank[def].sample_id;
}

uint8_t sgd::getSampleRoot(int def) {
    return rgndBank[def].root_key;
}

uint8_t sgd::getSampleLowRange(int def) {
    return rgndBank[def].range_low;
}

uint8_t sgd::getSampleHighRange(int def) {
    return rgndBank[def].range_high;
}

uint8_t sgd::getSampleVal5(int def) {
    return rgndBank[def].value5 >> 0x04;
}

uint8_t sgd::getSampleBank(int def) {
    return rgndBank[def].program.bank;
}

uint8_t sgd::getSamplePreset(int def) {
    return rgndBank[def].program.preset;
}
