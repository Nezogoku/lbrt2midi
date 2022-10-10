#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <iterator>
#include <vector>
#include "decode/decode.hpp"
#include "psxsgd.hpp"

using std::fstream;
using std::ifstream;
using std::cerr;
using std::cout;
using std::hex;
using std::dec;
using std::endl;
using std::ios;
using std::remove;
using std::string;
using std::to_string;
using std::vector;


sgd::sgd() {
}

sgd::sgd(bool isDebug) {
    this->isDebug = isDebug;
}


void sgd::setFile(std::string tmpFile) {
    tmpFile.erase(remove(tmpFile.begin(), tmpFile.end(), '\"'), tmpFile.end());
    fileName = tmpFile;
}

void sgd::setFile(std::string tmpHead, std::string tmpBody) {
    tmpHead.erase(remove(tmpHead.begin(), tmpHead.end(), '\"'), tmpHead.end());
    tmpBody.erase(remove(tmpBody.begin(), tmpBody.end(), '\"'), tmpBody.end());

    string tmpFile = tmpHead.substr(0, tmpHead.find_last_of('.')) + ".sgd";

    ifstream test(tmpFile);
    if (test.is_open()) {
        test.close();
    }
    else {
        char buff;

        ifstream head(tmpHead, ios::binary);
        ifstream body(tmpHead, ios::binary);
        if (!head.is_open()) {
            cerr << "Unable to open header file" << endl;
            body.close();
            return;
        }
        else if (!head.is_open()) {
            cerr << "Unable to open body file" << endl;
            head.close();
            return;
        }
        std::ofstream headBody(tmpFile, ios::binary);

        while(head.get(buff)) {
            head.get(buff);
            headBody.put(buff);
        }
        head.close();

        while(body.get(buff)) {
            body.get(buff);
            headBody.put(buff);
        }
        body.close();

        headBody.close();
    }

    fileName = tmpFile;
}

void sgd::setReverse(uint32_t &tmpInt) {
    uint32_t buffer = 0x00;
    for (int b = 0; b < 4; ++b) {
        buffer |= uint8_t((tmpInt >> (0x00 + (8 * b))) & 0xFF);
        if (b != 3) buffer <<= 8;
    }
    tmpInt = buffer;
}

int sgd::setInfo() {
    ifstream inFile(fileName, ios::binary);
    if (!inFile.is_open()) {
        cerr << "Unable to open this file" << endl;
        return 0;
    }

    uint32_t working_offset, chunk;
    char buff;

    //Get chunk type SGXD
    working_offset = 0x0;
    inFile.seekg(working_offset);
    inFile.read((char*)(&sgxdHead.chunktype), sizeof(uint32_t));
    setReverse(sgxdHead.chunktype);

    if (sgxdHead.chunktype != PSX_SGXD) {
        cerr << "This is not an sgd file" << endl;
        inFile.close();
        return 0;
    }

    hasRGND = false;
    hasSEQD = false;
    hasWAVE = false;
    hasNAME = false;

    rgndHead.amount = 0;
    seqdHead.amount = 0;
    waveHead.amount = 0;
    nameHead.amount = 0;

    //Get offset of bank name
    working_offset += 0x04;
    inFile.seekg(working_offset);
    inFile.read((char*)(&sgxdHead.name_offset), sizeof(uint32_t));
    if (isDebug) cout << "Offset to Bank name: 0x" << hex << sgxdHead.name_offset << dec << endl;

    //Get offset of data
    working_offset += 0x04;
    inFile.seekg(working_offset);
    inFile.read((char*)(&sgxdHead.data_offset), sizeof(uint32_t));
    if (isDebug) cout << "Offset to Bank audio: 0x" << hex << sgxdHead.data_offset << dec << endl;

    //Get size of data + 0x80000000
    working_offset += 0x04;
    inFile.seekg(working_offset);
    inFile.read((char*)(&sgxdHead.length), sizeof(uint32_t));
    if (isDebug) cout << "Size of Bank audio (padding): 0x" << hex << sgxdHead.length << dec << endl;
    sgxdHead.length %= 0x80000000;
    if (isDebug) cout << "Size of Bank audio (no padding): 0x" << hex << sgxdHead.length << dec << endl;

    working_offset += 0x04;

    //Search file for specific types
    while (inFile.get(buff)) {
        inFile.seekg(working_offset);
        inFile.read((char*)(&chunk), sizeof(uint32_t));
        setReverse(chunk);

        if (chunk == PSX_RGND) {
            hasRGND = true;
            rgndHead.chunktype = chunk;
            if (isDebug) cout << "\nHas RGND section" << endl;

            //Get size of chunk RGND
            working_offset += 0x04;
            inFile.seekg(working_offset);
            inFile.read((char*)(&rgndHead.length), sizeof(uint32_t));
            if (isDebug) cout << "Size of RGND section: 0x" << hex << rgndHead.length << dec << endl;

            //Get start of RGND chunk
            working_offset += 0x04;
            rgndHead.addr_start = working_offset;

            //Get amount of pre-definitions of RGND
            working_offset += 0x04;
            inFile.seekg(working_offset);
            inFile.read((char*)(&rgndHead.amount_pre), sizeof(uint32_t));
            if (isDebug) cout << "Number of RGND bank programs: " << rgndHead.amount_pre << endl;

            //Get address of pre-definitions of RGND
            working_offset += 0x04;
            rgndHead.addr_pre_start = working_offset;
            if (isDebug) cout << "Offset to RGND bank programs: 0x" << hex << rgndHead.addr_pre_start << dec << endl;

            //Calculate address of definitions of RGND
            working_offset = rgndHead.addr_pre_start + (rgndHead.amount_pre * 0x8);
            rgndHead.addr_def_start = working_offset;
            if (isDebug) cout << "Offset to RGND bank definitions: 0x" << hex << rgndHead.addr_def_start << dec << endl;

            //Calculate amount of definitions of RGND
            rgndHead.amount = (rgndHead.addr_start + rgndHead.length) - rgndHead.addr_def_start;
            rgndHead.amount = std::floor(double(rgndHead.amount) / 0x38);
            if (isDebug) cout << "Number of RGND bank definitions: " << rgndHead.amount << endl;
            rgndBank.resize(rgndHead.amount);

            working_offset = rgndHead.addr_start + rgndHead.length;
        }
        else if (chunk == PSX_SEQD) {
            hasSEQD = true;
            seqdHead.chunktype = chunk;
            if (isDebug) cout << "\nHas SEQD section" << endl;

            //Get size of chunk SEQD
            working_offset += 0x04;
            inFile.seekg(working_offset);
            inFile.read((char*)(&seqdHead.length), sizeof(uint32_t));
            if (isDebug) cout << "Size of SEQD section: 0x" << hex << seqdHead.length << dec << endl;

            //Get start of SEQD chunk
            working_offset += 0x04;
            seqdHead.addr_start = working_offset;

            working_offset = seqdHead.addr_start + seqdHead.length;
        }
        else if (chunk == PSX_WAVE) {
            hasWAVE = true;
            waveHead.chunktype = chunk;
            if (isDebug) cout << "\nHas WAVE section" << endl;

            //Get size of chunk WAVE
            working_offset += 0x04;
            inFile.seekg(working_offset);
            inFile.read((char*)(&waveHead.length), sizeof(uint32_t));
            if (isDebug) cout << "Size of WAVE section: 0x" << hex << waveHead.length << dec << endl;

            //Get start of WAVE chunk
            working_offset += 0x4;
            waveHead.addr_start = working_offset;

            //Get amount of audio definitions of WAVE
            working_offset += 0x04;
            inFile.seekg(working_offset);
            inFile.read((char*)(&waveHead.amount), sizeof(uint32_t));
            if (isDebug) cout << "Number of WAVE definitions: " << waveHead.amount << endl;
            waveBank.resize(waveHead.amount);

            //Get address of audio definitions of WAVE
            working_offset += 0x04;
            waveHead.addr_wave_start = working_offset;
            if (isDebug) cout << "Offset to WAVE definitions: 0x" << hex << waveHead.addr_wave_start << dec << endl;

            working_offset = waveHead.addr_start + waveHead.length;
        }
        else if (chunk == PSX_NAME) {
            hasNAME = true;
            nameHead.chunktype = chunk;
            if (isDebug) cout << "\nHas NAME section" << endl;

            //Get size of chunk NAME
            working_offset += 0x04;
            inFile.seekg(working_offset);
            inFile.read((char*)(&nameHead.length), sizeof(uint32_t));
            if (isDebug) cout << "Size of NAME section: 0x" << hex << nameHead.length << dec << endl;

            //Get start of NAME chunk
            working_offset += 0x04;
            nameHead.addr_start = working_offset;

            //Get amount of names of NAME
            working_offset += 0x04;
            inFile.seekg(working_offset);
            inFile.read((char*)(&nameHead.amount), sizeof(uint32_t));
            if (isDebug) cout << "Number of NAME definitions: " << nameHead.amount << endl;

            //Get address of definitions of NAME
            working_offset += 0x04;
            nameHead.addr_name_start = working_offset;
            if (isDebug) cout << "Offset to NAME definitions: 0x" << hex << nameHead.addr_name_start << dec << endl;
            break;
        }
        else working_offset += 0x01;
    }
    if (isDebug) cout << endl;


    if (!hasRGND) {
        cerr << "This sgd file is not a sound bank" << endl;
        inFile.close();
        return 0;
    }
    else if (!hasNAME || !hasWAVE) {
        cerr << "This sgd file is not compatible with the current programme" << endl;
        inFile.close();
        return 0;
    }

    setBankName(inFile);
    setNAME(inFile);
    setRGND(inFile);
    if (hasSEQD) setSEQD(inFile);
    setWAVE(inFile);
    setDATA(inFile);

    inFile.close();

    return 1;
}

void sgd::setBankName(std::ifstream &tmpData) {
    char buff;
    bankName = "";

    ///Retrieving bank name from file
    tmpData.seekg(sgxdHead.name_offset);
    while (true) {
        tmpData.get(buff);
        if (buff == 0x00) break;
        bankName += buff;
    }

    if (isDebug) cout << endl;
    if (isDebug) cout << "Bank name: " << bankName << endl;
}

void sgd::setNAME(std::ifstream &tmpData) {
    if (isDebug) cout << endl;

    for (int n = 0; n < nameHead.amount; ++n) {
        uint32_t working_offset;
        char buff;
        name tempName;

        working_offset = nameHead.addr_name_start + (n * 0x8);

        ///Retrieving name info from vector
        tmpData.seekg(working_offset);
        tmpData.read((char*)(&tempName.name_id), sizeof(uint16_t));
        working_offset += 0x2;

        tmpData.seekg(working_offset);
        tmpData.read((char*)(&tempName.name_type), sizeof(uint16_t));
        working_offset += 0x2;

        if (tempName.name_type > 0x2000 && tempName.name_type < 0x3000) {
            tmpData.seekg(working_offset);
            tmpData.read((char*)(&tempName.name_offset), sizeof(uint32_t));
            working_offset = tempName.name_offset;

            tmpData.seekg(working_offset);
            tempName.name = "";

            while (true) {
                tmpData.get(buff);
                if (buff == 0x00) break;
                tempName.name += buff;
            }

            nameBank.push_back(tempName);
        }
    }


    //Reorganize names
    for (int i = 0; i < 2; ++i) {
        for (int a = 0; a < nameBank.size(); ++a) {
            for (int b = a + 1; b < nameBank.size(); ++b) {
                if (i == 0) {
                    if (nameBank[a].name_type > nameBank[b].name_type) {
                        std::swap(nameBank[a], nameBank[b]);
                    }
                }
                else {
                    if (nameBank[a].name_type == nameBank[b].name_type &&
                        nameBank[a].name_id > nameBank[b].name_id) {
                        std::swap(nameBank[a], nameBank[b]);
                    }
                }
            }
        }
    }
}

void sgd::setRGND(std::ifstream &tmpData) {
    uint32_t working_offset;

    //Retrieve program pre-definitions
    if (isDebug) cout << endl;
    if (isDebug) cout << "Number of pre definitions: " << rgndHead.amount_pre << endl;
    vector<program_data> prgm_data(rgndHead.amount_pre);

    for (int d = 0; d < prgm_data.size(); ++d) {
        working_offset = rgndHead.addr_pre_start + (0x08 * d);

        tmpData.seekg(working_offset);
        tmpData.read((char*)(&prgm_data[d].instrument), sizeof(uint32_t));

        working_offset += 0x04;

        tmpData.seekg(working_offset);
        tmpData.read((char*)(&prgm_data[d].program_offset), sizeof(uint32_t));

        prgm_data[d].bank = 0x00;
        prgm_data[d].preset = d;

        if (isDebug) cout << "Offset to program section: 0x" << hex << prgm_data[d].program_offset << dec << endl;
    }


    int index = 0;
    for (int d = 0; d < rgndHead.amount; ++d) {
        rgnd tempRgnd;
        working_offset = rgndHead.addr_def_start + (0x38 * d);

        if (isDebug) cout << endl;
        if (isDebug) cout << "Current offset: 0x" << hex << working_offset << dec << endl;
        for (int index = 0; index < prgm_data.size(); ++index) {
            if (prgm_data[index].program_offset == working_offset) {
                if (isDebug) cout << "Instrument, Bank, and Preset starting from program "
                                  << index << ": " << prgm_data[index].instrument
                                  << ' ' << int(prgm_data[index].bank)
                                  << ' ' << int(prgm_data[index].preset)
                                  << endl;

                tempRgnd.program.program_offset = working_offset;
                tempRgnd.program.instrument = prgm_data[index].instrument;
                tempRgnd.program.bank = prgm_data[index].bank;
                tempRgnd.program.preset = prgm_data[index].preset;
            }
        }


        ///Retrieving region info from vector
        tmpData.seekg(working_offset + 0x00);
        tmpData.read((char*)(&tempRgnd.priority), sizeof(uint32_t));
        if (isDebug) cout << "Priority: " << tempRgnd.priority << endl;

        tmpData.seekg(working_offset + 0x18);
        tmpData.read((char*)(&tempRgnd.range_low), sizeof(uint8_t));
        if (isDebug) cout << "Lowest range: " << int(tempRgnd.range_low) << endl;

        tmpData.seekg(working_offset + 0x19);
        tmpData.read((char*)(&tempRgnd.range_high), sizeof(uint8_t));
        if (isDebug) cout << "Highest range: " << int(tempRgnd.range_high) << endl;

        tmpData.seekg(working_offset + 0x1C);
        tmpData.read((char*)(&tempRgnd.root_key), sizeof(uint8_t));
        if (tempRgnd.root_key > 0x7F) {
            //Catch root greater than 127
            tempRgnd.semi_tune = tempRgnd.root_key % 0x7F;
            tempRgnd.root_key = 0x7F;
        }
        else { tempRgnd.semi_tune = 0x00; }
        if (isDebug) cout << "Root key: " << int(tempRgnd.root_key) << endl;
        if (isDebug) cout << "Semitone tuning: " << int(tempRgnd.semi_tune) << endl;

        tmpData.seekg(working_offset + 0x1D);
        tmpData.read((char*)(&tempRgnd.fine_tune), sizeof(uint8_t));
        if (isDebug) cout << "Fine tuning: " << int(tempRgnd.fine_tune) << endl;

        tmpData.seekg(working_offset + 0x20);
        tmpData.read((char*)(&tempRgnd.mod_0), sizeof(uint8_t));
        if (isDebug) cout << "Modifier 1: " << int(tempRgnd.mod_0) << endl;

        tmpData.seekg(working_offset + 0x21);
        tmpData.read((char*)(&tempRgnd.mod_1), sizeof(uint8_t));
        if (isDebug) cout << "Modifier 2: " << int(tempRgnd.mod_1) << endl;

        tmpData.seekg(working_offset + 0x22);
        tmpData.read((char*)(&tempRgnd.mod_2), sizeof(uint8_t));
        if (isDebug) cout << "Modifier 3: " << int(tempRgnd.mod_2) << endl;

        tmpData.seekg(working_offset + 0x23);
        tmpData.read((char*)(&tempRgnd.pan), sizeof(uint8_t));
        if (isDebug) cout << "Pan: " << int(tempRgnd.pan) << endl;

        tmpData.seekg(working_offset + 0x34);
        tmpData.read((char*)(&tempRgnd.sample_id), sizeof(uint32_t));
        if (isDebug) cout << "Sample ID: " << tempRgnd.sample_id << endl;

        rgndBank[d] = tempRgnd;
    }

    if (isDebug) cout << endl;
}

void sgd::setSEQD(std::ifstream &tmpData) {
    uint32_t working_offset = seqdHead.addr_start,
             nameBeg = nameHead.addr_name_start,
             nameEnd = nameHead.addr_name_start + nameHead.length;

    ///Retrieving sequence data from file
    while (working_offset < (seqdHead.addr_start + seqdHead.length)) {
        seqd tempSeqd;
        char buff;

        working_offset += 0x01;
        tmpData.seekg(working_offset);
        tmpData.read((char*)(&tempSeqd.name_offset), sizeof(uint32_t));

        if (tempSeqd.name_offset > nameBeg && tempSeqd.name_offset < nameEnd) {
            bool isReset = false;
            if (isDebug) cout << endl;

            tmpData.seekg(tempSeqd.name_offset);
            tempSeqd.name = "";
            while (true) {
                tmpData.get(buff);
                if (buff == 0x00) break;
                tempSeqd.name += buff;
            }
            if (isDebug) cout << "Name of sequence " << seqdBank.size() << ": " << tempSeqd.name << endl;

            tmpData.seekg(working_offset + 0x04);
            tmpData.get(buff);

            tempSeqd.sequence_format = buff;
            if (isDebug) cout << "Format of sequence " << seqdBank.size() << ": " << int(tempSeqd.sequence_format) << endl;

            tempSeqd.sequence_offset = working_offset + 0x05;
            if (isDebug) cout << "Offset to sequence " << seqdBank.size() << ": 0x" << hex << tempSeqd.sequence_offset << dec << endl;


            working_offset = tempSeqd.sequence_offset;
            tmpData.seekg(working_offset);
            if (tempSeqd.sequence_format == 0x00) {
                tempSeqd.sequence.push_back(char(0x00));
                tempSeqd.sequence.push_back(char(0xFF));
                tempSeqd.sequence.push_back(char(0x00));
                tempSeqd.sequence.push_back(char(0x02));
                tempSeqd.sequence.push_back(char(0x00));
                tempSeqd.sequence.push_back(char(seqdBank.size()));
            }
            while (true) {
                tmpData.get(buff);
                tempSeqd.sequence.push_back(buff);

                if (buff == char(0xFF)) isReset = true;
                else if (isReset && !(buff == char(0x2F) || buff == char(0x00))) isReset = false;
                else if (isReset && (buff == char(0x2F) || buff == char(0x00))) {
                    if (tempSeqd.sequence_format == 0x00) {
                        tempSeqd.sequence.pop_back();
                        tempSeqd.sequence.pop_back();
                        tempSeqd.sequence.push_back(char(0xFF));
                        tempSeqd.sequence.push_back(char(0x2F));
                        tempSeqd.sequence.push_back(char(0x00));
                    }
                    else if (tempSeqd.sequence_format == 0x01) {
                        tempSeqd.sequence.push_back(char(0x00));
                    }

                    break;
                }
            }
            working_offset += tempSeqd.sequence.size() - 0x04;

            for (int s = 0; s < tempSeqd.sequence.size(); ++s) {
                int s0 = s,
                    s1 = s + 1,
                    s2 = s + 2;

                if ((s0 >= tempSeqd.sequence.size()) ||
                    (s1 >= tempSeqd.sequence.size()) ||
                    (s2 >= tempSeqd.sequence.size())) break;

                if ((tempSeqd.sequence[s0] & 0xF0) == 0xB0) {
                    //Sequence uses CC 99 for looping
                    //Will use CC 116/117 instead
                    //Additionally use Final Fantasy style just because
                    if ((tempSeqd.sequence[s1] & 0xFF) == 0x63) {
                        auto pos = tempSeqd.sequence.begin() + s2 + 1;
                        vector<char> ffStyle = {char(0x00), char(0xFF), char(0x06)};
                        string ffMark;

                        if ((tempSeqd.sequence[s2] & 0xFF) == 0x14) {
                            tempSeqd.sequence[s1] = 0x74;
                            tempSeqd.sequence[s2] = 0x00;

                            ffMark = "loopStart";
                        }
                        else if ((tempSeqd.sequence[s2] & 0xFF) == 0x1E) {
                            tempSeqd.sequence[s1] = 0x75;
                            tempSeqd.sequence[s2] = 0x7F;

                            ffMark = "loopEnd";
                        }
                        else continue;

                        ffStyle.push_back(char(ffMark.size()));
                        ffStyle.insert(ffStyle.end(), ffMark.begin(), ffMark.end());
                        tempSeqd.sequence.insert(pos, ffStyle.begin(), ffStyle.end());
                        ffStyle.clear();
                    }

                    //Not sure what sequence does with CC 118/119
                    //Change them to CC 3... just because
                    else if ((tempSeqd.sequence[s1] & 0xFF) == 0x76 ||
                             (tempSeqd.sequence[s1] & 0xFF) == 0x77) {
                        tempSeqd.sequence[s1] = 0x03;
                    }
                }
            }

            if (isDebug) cout << "Size of sequence " << seqdBank.size() << ": " << tempSeqd.sequence.size() << endl;
            seqdBank.push_back(tempSeqd);
        }
    }
    seqdHead.amount = seqdBank.size();
    if (isDebug) cout << endl;
}

void sgd::setWAVE(std::ifstream &tmpData) {
    uint32_t working_offset;

    for (int w = 0; w < waveHead.amount; ++w) {
        wave tempWave;
        working_offset = waveHead.addr_wave_start + (0x38 * w);

        ///Retrieving sample info from file
        tmpData.seekg(working_offset + 0x04);
        tmpData.read((char*)(&tempWave.name_offset), sizeof(uint32_t));
        if (isDebug) cout << hex << "Offset to sample name: 0x" << tempWave.name_offset << endl;

        tmpData.seekg(working_offset + 0x08);
        tmpData.read((char*)(&tempWave.codec), sizeof(uint8_t));
        if (isDebug) cout << "Codec: " << dec << (int)tempWave.codec << endl;

        tmpData.seekg(working_offset + 0x09);
        tmpData.read((char*)(&tempWave.channels), sizeof(uint8_t));
        if (isDebug) cout << "Channels: " << (int)tempWave.channels << endl;

        tmpData.seekg(working_offset + 0x0C);
        tmpData.read((char*)(&tempWave.sample_rate), sizeof(uint32_t));
        if (isDebug) cout << "Sample rate: " << tempWave.sample_rate << endl;

        tmpData.seekg(working_offset + 0x10);
        tmpData.read((char*)(&tempWave.info_type), sizeof(uint32_t));

        tmpData.seekg(working_offset + 0x14);
        tmpData.read((char*)(&tempWave.info_value), sizeof(uint32_t));

        tmpData.seekg(working_offset + 0x20);
        tmpData.read((char*)(&tempWave.sample_size), sizeof(uint32_t));
        if (isDebug) cout << "Sample size: 0x" << hex << tempWave.sample_size << endl;

        tmpData.seekg(working_offset + 0x24);
        tmpData.read((char*)(&tempWave.loop_start), sizeof(uint32_t));
        if (isDebug) cout << "Loop start: 0x" << tempWave.loop_start << endl;

        tmpData.seekg(working_offset + 0x28);
        tmpData.read((char*)(&tempWave.loop_end), sizeof(uint32_t));
        if (isDebug) cout << "Loop end: 0x" << tempWave.loop_end << endl;

        tmpData.seekg(working_offset + 0x2C);
        tmpData.read((char*)(&tempWave.stream_size), sizeof(uint32_t));
        if (isDebug) cout << "Size of stream or interleave (type3): 0x" << tempWave.stream_size << endl;

        tmpData.seekg(working_offset + 0x30);
        tmpData.read((char*)(&tempWave.stream_offset), sizeof(uint32_t));
        if (isDebug) cout << "Offset to stream: 0x" << tempWave.stream_offset << endl;

        tmpData.seekg(working_offset + 0x34);
        tmpData.read((char*)(&tempWave.stream_size_full), sizeof(uint32_t));
        if (isDebug) cout << "Zero-padded size of stream or interleave (type3): 0x" << tempWave.stream_size_full << endl;

        tempWave.isLoop = (tempWave.loop_start != 0xFFFFFFFF || tempWave.loop_end != 0xFFFFFFFF) ? true : false;
        if (isDebug) cout << "Sample is looped: " << ((tempWave.isLoop) ? "TRUE" : "FALSE") << endl;

        tempWave.name = "samp_" + std::to_string(w);
        if (isDebug) cout << "Name of sample: " << tempWave.name << endl;
        if (isDebug) cout << dec << endl;

        waveBank[w] = tempWave;
    }
}

void sgd::setDATA(std::ifstream &tmpData) {
    if (isDebug) cout << endl;
    for (auto &wave_info : waveBank) {
        uint32_t working_offset = sgxdHead.data_offset + wave_info.stream_offset;

        if (wave_info.stream_size == 0x00) continue;        // Skip empty samples
        else if (wave_info.channels != 0x01) continue;      // And non-mono samples

        tmpData.seekg(working_offset);
        for (int i = 0; i < wave_info.stream_size; ++i) {
            char buff;
            tmpData.get(buff);
            wave_info.data.push_back(buff);
        }

        if (isDebug) cout << "Codec type: ";
        switch (wave_info.codec) {
            case 0x01:      //HEADERLESS Big Endian PCM16
                if (isDebug) cout << "Big Endian PCM16" << endl;
                wave_info.pcmle = pcmbDecode(wave_info.data.data(), wave_info.data.size(),
                                             wave_info.loop_start, wave_info.loop_end);
                continue;

            case 0x02:      //Ogg Vorbis
                if (isDebug) cout << "Ogg Vorbis" << endl;
                wave_info.pcmle = oggDecode(wave_info.data.data(), wave_info.data.size(),
                                            wave_info.channels, wave_info.sample_rate,
                                            wave_info.loop_start, wave_info.loop_end);
                continue;

            case 0x03:      //HEADERLESS Sony ADPCM
                if (isDebug) cout << "Sony ADPCM" << endl;
                wave_info.pcmle = vagDecode(wave_info.data.data(), wave_info.data.size(),
                                            wave_info.loop_start, wave_info.loop_end);
                continue;

            case 0x04:      //Sony Atrac3Plus {WIP}
                if (isDebug) cout << "Sony Atrac3Plus" << endl;
                wave_info.pcmle = atpDecode(wave_info.data.data(), wave_info.data.size(),
                                            wave_info.channels, wave_info.sample_rate);
                continue;

            case 0x05:      //HEADERLESS Sony Short ADPCM {WIP}
                if (isDebug) cout << "Sony Short ADPCM" << endl;
                wave_info.pcmle = vgsDecode(wave_info.data.data(), wave_info.data.size(),
                                            wave_info.loop_start, wave_info.loop_end);
                continue;

            case 0x06:      //HEADERLESS Sony Atrac3 {WIP}
                if (isDebug) cout << "Sony Atrac3" << endl;
                wave_info.pcmle = at3Decode(wave_info.data.data(), wave_info.data.size(),
                                            wave_info.channels, wave_info.sample_rate);
                continue;

            default:        //Unknown
                if (isDebug) cout << "Unknown type " << wave_info.codec << endl;
                continue;
        }
    }

    if (isDebug) cout << endl;
}


int sgd::getAmountAudio() {
    return waveHead.amount;
}

int sgd::getAmountPrograms() {
    return rgndHead.amount_pre;
}

int sgd::getAmountDefinitions() {
    return rgndHead.amount;
}

int sgd::getAmountSequence() {
    return seqdHead.amount;
}


std::string sgd::getBankName() {
    return bankName;
}

std::string sgd::getSampleName(int samp) {
    return waveBank[samp].name;
}

std::vector<int16_t> sgd::getSample(int samp) {
    if (samp >= waveHead.amount || samp < 0) {
        cerr << "Sample " << samp << " is outside the range of existing audio samples" << endl;
        return {};
    }
    else if (waveBank[samp].channels != 0x01) {
        cerr << "Sample " << samp << " is not supported at this time" << endl;
        return {};
    }
    else {
        return waveBank[samp].pcmle;
    }
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

int sgd::getSampleID(int samp) {
    return rgndBank[samp].sample_id;
}

uint8_t sgd::getSampleRoot(int samp) {
    return rgndBank[samp].root_key;
}

uint8_t sgd::getSampleLow(int samp) {
    return rgndBank[samp].range_low;
}

uint8_t sgd::getSampleHigh(int samp) {
    return rgndBank[samp].range_high;
}

int8_t sgd::getSampleTuningFine(int samp) {
    return rgndBank[samp].fine_tune;
}

int8_t sgd::getSampleTuningSemi(int samp) {
    return rgndBank[samp].semi_tune;
}

int8_t sgd::getSampleMod1(int samp) {
    return rgndBank[samp].mod_0;
}

int8_t sgd::getSampleMod2(int samp) {
    return rgndBank[samp].mod_1;
}

int8_t sgd::getSampleMod3(int samp) {
    return rgndBank[samp].mod_2;
}

double sgd::getSamplePanPercent(int samp) {
    return (rgndBank[samp].pan / 127.00);
}

int sgd::getSampleProgram(int samp) {
    return rgndBank[samp].program.instrument;
}

uint8_t sgd::getSampleBank(int samp) {
    return rgndBank[samp].program.bank;
}

uint8_t sgd::getSamplePreset(int samp) {
    return rgndBank[samp].program.preset;
}

bool sgd::sampleIsLoop(int samp) {
    return waveBank[samp].isLoop;
}

uint32_t sgd::getSampleLpStrt(int samp) {
    return waveBank[samp].loop_start;
}

uint32_t sgd::getSampleLpTrm(int samp) {
    return waveBank[samp].loop_end;
}


void sgd::writeSequence(int seq) {
    if (seq >= seqdHead.amount || seq < 0) {
        cerr << seq << " is outside the range of existing sequences" << endl;
        return;
    }
    else if (seqdBank[seq].sequence_format == 0x00) return;

    const char MThd[5] = "MThd";
    const uint32_t MThd_SIZE = 0x06;
    uint16_t MThd_Frmt = uint16_t(seqdBank[seq].sequence_format);
    const uint16_t MThd_TRKS = 0x01;
    //Division 1, 30 FPS, 196 Ticks per frame (?)
    //Would be 1  0011110 11000100
    const uint16_t MThd_DIVI = 0x9EC4;
    const char MTrk[5] = "MTrk";
    uint32_t MTrk_Size;

    vector<char> midSequence;
    midSequence.push_back(char(0x00));
    midSequence.push_back(char(0xFF));
    midSequence.push_back(char(0x03));
    midSequence.push_back(char(seqdBank[seq].name.length()));
    for (char &chara : seqdBank[seq].name) midSequence.push_back(chara);
    for (char &seque : seqdBank[seq].sequence) midSequence.push_back(seque);

    MTrk_Size = midSequence.size();


    string mid_path = fileName.substr(0, fileName.find_last_of("\\/") + 1);
    string midi_title = seqdBank[seq].name;
    midi_title += ".mid";
    mid_path += midi_title;

    std::ofstream mid(mid_path, ios::binary);
    try {
        mid.write(MThd, sizeof(uint32_t));
        mid.put(char((MThd_SIZE >> 0x18) & 0xFF));
        mid.put(char((MThd_SIZE >> 0x10) & 0xFF));
        mid.put(char((MThd_SIZE >> 0x08) & 0xFF));
        mid.put(char((MThd_SIZE >> 0x00) & 0xFF));
        mid.put(char((MThd_Frmt >> 0x08) & 0xFF));
        mid.put(char((MThd_Frmt >> 0x00) & 0xFF));
        mid.put(char((MThd_TRKS >> 0x08) & 0xFF));
        mid.put(char((MThd_TRKS >> 0x00) & 0xFF));
        mid.put(char((MThd_DIVI >> 0x08) & 0xFF));
        mid.put(char((MThd_DIVI >> 0x00) & 0xFF));
        mid.write(MTrk, sizeof(uint32_t));
        mid.put(char((MTrk_Size >> 0x18) & 0xFF));
        mid.put(char((MTrk_Size >> 0x10) & 0xFF));
        mid.put(char((MTrk_Size >> 0x08) & 0xFF));
        mid.put(char((MTrk_Size >> 0x00) & 0xFF));
        for (char &event : midSequence) mid.put(event);
        cout << seqdBank[seq].name << " successfully extracted" << endl;
    }
    catch (const fstream::failure & e) {
        cerr << "Unable to extract " << seqdBank[seq].name << ":\n    " << e.what() << endl;
    }
    catch (const std::exception & e) {
        cerr << "Something went wrong extracting " << seqdBank[seq].name << ":\n    " << e.what() << endl;
    }
    mid.close();
}
