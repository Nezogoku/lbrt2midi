#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdint>
#include <fstream>
#include "../../bit_byte.hpp"
#include "decode/decode.hpp"
#include "psxsgd_types.hpp"
#include "psxsgd.hpp"


sgd::sgd() { sgd(false); }
sgd::sgd(bool isDebug) {
    this->isDebug = isDebug;

    sgxdData = 0;
    rgndBank = 0;
    waveBank = 0;
    seqdBank = 0;
    nameBank = 0;
}


void sgd::setFile(std::string tmpFile) {
    setFile(tmpFile, "");
}
void sgd::setFile(std::string tmpHead, std::string tmpBody) {
    tmpHead.erase(std::remove(tmpHead.begin(), tmpHead.end(), '\"'), tmpHead.end());
    tmpBody.erase(std::remove(tmpBody.begin(), tmpBody.end(), '\"'), tmpBody.end());

    sgdName = tmpHead.substr(tmpHead.find_last_of("\\/") + 1);
    sgbName = tmpBody.substr(tmpBody.find_last_of("\\/") + 1);
    pathName = tmpHead.substr(0, tmpHead.find_last_of("\\/") + 1);
}

void sgd::resetData() {
    sgxdHead = sgxd_head();
    sgxdSize = 0;

    bankName.clear();

    hasRGND = false;
    hasSEQD = false;
    hasWAVE = false;
    hasNAME = false;

    rgndHead = rgnd_head();
    seqdHead = seqd_head();
    waveHead = wave_head();
    nameHead = name_head();

    if (sgxdData) { delete[] sgxdData; sgxdData = 0; }
    if (rgndBank) { delete[] rgndBank; rgndBank = 0; }
    if (waveBank) { delete[] waveBank; waveBank = 0; }
    if (seqdBank) { delete[] seqdBank; seqdBank = 0; }
    if (nameBank) { delete[] nameBank; nameBank = 0; }
}

int sgd::openSGXD() {
    uchar *sgh_data, *sgb_data;
    int sgh_length, sgb_offset, sgb_length;

    std::ifstream sgd_file;
    sgd_file.open(pathName + sgdName, std::ios::binary);
    if (!sgd_file.is_open()) {
        fprintf(stderr, "Unable to open \"%s\" . . .\n", sgdName.c_str());
        return 0;
    }

    sgd_file.seekg(0, std::ios::end);
    sgh_length = sgd_file.tellg();
    sgd_file.seekg(0, std::ios::beg);

    sgh_data = new uchar[sgh_length];
    sgd_file.read((char*)(sgh_data), sgh_length);
    sgd_file.close();

    if (!cmpChar(sgh_data, "SGXD", 4)) {
        fprintf(stderr, "Not an SGXD file\n");
        return 0;
    }

    sgh_data += 8;
    sgb_offset = getLeInt(sgh_data, 4);
    sgb_length = getLeInt(sgh_data, 4) % 0x80000000;
    sgh_data -= 16;

    if (!sgbName.empty()) {
        sgd_file.open(pathName + sgbName, std::ios::binary);
        if (!sgd_file.is_open()) {
            fprintf(stderr, "Unable to open \"%s\"\n", sgbName.c_str());
            return 0;
        }

        sgd_file.seekg(0, std::ios::end);
        sgb_length = sgd_file.tellg();
        sgd_file.seekg(0, std::ios::beg);

        sgb_data = new uchar[sgb_length];
        sgd_file.read((char*)(sgb_data), sgb_length);
        sgd_file.close();

        sgh_length = sgb_offset;
    }
    else {
        sgb_offset = sgh_length;
        sgb_data = 0;
        sgb_length = 0;
    }

    sgxdSize = sgh_length + sgb_length;
    sgxdData = new uchar[sgxdSize];

    for (int i = 0; i < sgxdSize; ++i) {
        if (i < sgb_offset) sgxdData[i] = sgh_data[i];
        else sgxdData[i] = sgb_data[i - sgb_offset];
    }

    return 1;
}

int sgd::setInfo() {
    resetData();
    if (!openSGXD()) return 0;

    uchar *sgxdBegin = sgxdData;

    sgxdData += 0x04;
    sgxdHead.name_offset = getLeInt(sgxdData, 0x04);
    sgxdHead.data_offset = getLeInt(sgxdData, 0x04);
    sgxdHead.data_length = getLeInt(sgxdData, 0x04);
    sgxdHead.flag = (sgxdHead.data_length >> 31) & 0x01;
    sgxdHead.data_length %= 0x80000000;
    if ((sgxdHead.data_offset + sgxdHead.data_length) > sgxdSize) {
        sgxdHead.data_length = sgxdSize - sgxdHead.data_offset;
    }

    if (isDebug) fprintf(stdout, "Bank Name at position: 0x%08X\n", sgxdHead.name_offset);
    if (isDebug) fprintf(stdout, "Bank Samples at position: 0x%08X\n", sgxdHead.data_offset);
    if (isDebug) fprintf(stdout, "Bank Samples total length: 0x%08X\n", sgxdHead.data_length);
    if (isDebug) fprintf(stdout, "Bank Samples flag: %s\n", ((sgxdHead.flag) ? "TRUE": "FALSE"));

    //Search file for specific types
    while (sgxdData < (sgxdBegin + sgxdSize)) {
        if (cmpChar(sgxdData, "BUSS", 4)) {
            sgxdData += 0x04;
            unsigned int buss_length = getLeInt(sgxdData, 0x04);

            sgxdData = sgxdData + buss_length;
        }
        else if (cmpChar(sgxdData, "RGND", 4)) {
            if (isDebug) fprintf(stdout, "\nRGND found at position: 0x%08X\n", sgxdData - sgxdBegin);
            hasRGND = true;

            rgndHead.chunktype = getBeInt(sgxdData, 0x04);
            rgndHead.length = getLeInt(sgxdData, 0x04);

            rgndHead.addr_start = sgxdData - sgxdBegin;
            rgndHead.reserved = getLeInt(sgxdData, 0x04);
            rgndHead.amount_pre = getLeInt(sgxdData, 0x04);

            rgndHead.addr_pre_start = sgxdData - sgxdBegin;
            rgndHead.addr_def_start = sgxdData + (rgndHead.amount_pre * 0x08) - sgxdBegin;

            if (isDebug) fprintf(stdout, "    Number of Pre-definitions: %u\n", rgndHead.amount_pre);
            if (isDebug) fprintf(stdout, "    Definitions found at position: 0x%08X\n", rgndHead.addr_def_start);

            sgxdData = sgxdBegin + rgndHead.addr_start + rgndHead.length;
        }
        else if (cmpChar(sgxdData, "SEQD", 4)) {
            if (isDebug) fprintf(stdout, "\nSEQD found at position: 0x%08X\n", sgxdData - sgxdBegin);
            hasSEQD = true;

            seqdHead.chunktype = getBeInt(sgxdData, 0x04);
            seqdHead.length = getLeInt(sgxdData, 0x04);

            seqdHead.addr_start = sgxdData - sgxdBegin;
            seqdHead.reserved = getLeInt(sgxdData, 0x04);
            seqdHead.amount_pre = getLeInt(sgxdData, 0x04);

            seqdHead.addr_pre_start = sgxdData - sgxdBegin;

            if (isDebug) fprintf(stdout, "    Number of Pre-definitions: %u\n", seqdHead.amount_pre);
            if (isDebug) fprintf(stdout, "    Pre-definitions found at position: 0x%08X\n", seqdHead.addr_pre_start);

            sgxdData = sgxdBegin + seqdHead.addr_start + seqdHead.length;
        }
        else if (cmpChar(sgxdData, "WAVE", 4)) {
            if (isDebug) fprintf(stdout, "\nWAVE found at position: 0x%08X\n", sgxdData - sgxdBegin);
            hasWAVE = true;

            waveHead.chunktype = getBeInt(sgxdData, 0x04);
            waveHead.length = getLeInt(sgxdData, 0x04);

            waveHead.addr_start = sgxdData - sgxdBegin;
            waveHead.reserved = getLeInt(sgxdData, 0x04);
            waveHead.amount = getLeInt(sgxdData, 0x04);

            waveHead.addr_wave_start = sgxdData - sgxdBegin;

            if (isDebug) fprintf(stdout, "    Number of samples: %u\n", waveHead.amount);
            if (isDebug) fprintf(stdout, "    Sample definitions found at position: 0x%08X\n", waveHead.addr_start);

            sgxdData = sgxdBegin + waveHead.addr_start + waveHead.length;
        }
        else if (cmpChar(sgxdData, "WSUR", 4)) {
            sgxdData += 0x04;
            unsigned int wsur_length = getLeInt(sgxdData, 0x04);

            sgxdData = sgxdData + wsur_length;
        }
        else if (cmpChar(sgxdData, "WMKR", 4)) {
            sgxdData += 0x04;
            unsigned int wmkr_length = getLeInt(sgxdData, 0x04);

            sgxdData = sgxdData + wmkr_length;
        }
        else if (cmpChar(sgxdData, "CONF", 4)) {
            sgxdData += 0x04;
            unsigned int conf_length = getLeInt(sgxdData, 0x04);

            sgxdData = sgxdData + conf_length;
        }
        else if (cmpChar(sgxdData, "NAME", 4)) {
            if (isDebug) fprintf(stdout, "\nNAME found at position: 0x%08X\n", sgxdData - sgxdBegin);
            hasNAME = true;

            nameHead.chunktype = getBeInt(sgxdData, 0x04);
            nameHead.length = getLeInt(sgxdData, 0x04);

            nameHead.addr_start = sgxdData - sgxdBegin;
            nameHead.reserved = getLeInt(sgxdData, 0x04);
            nameHead.amount = getLeInt(sgxdData, 0x04);

            nameHead.addr_name_start = sgxdData - sgxdBegin;

            if (isDebug) fprintf(stdout, "    Number of names: %u\n", nameHead.amount);
            if (isDebug) fprintf(stdout, "    Name definitions found at position: 0x%08X\n", nameHead.addr_start);

            sgxdData = sgxdBegin + nameHead.addr_start + nameHead.length;
            break;
        }
        else sgxdData += 0x01;
    }

    if (!hasNAME || !hasRGND || !hasWAVE) {
        fprintf(stderr, "\nThis sgd file is not compatible with the current programme\n");
        if (!hasNAME) fprintf(stderr, "    NAME Missing\n");
        if (!hasRGND) fprintf(stderr, "    RGND Missing\n");
        if (!hasWAVE) fprintf(stderr, "    WAVE Missing\n");
        return 0;
    }

    sgxdData = sgxdBegin;

    //Assign data from file
    setBankName(sgxdData + sgxdHead.name_offset);
    setNAME(sgxdData + nameHead.addr_name_start);
    setRGND(sgxdData + rgndHead.addr_pre_start);
    setSEQD(sgxdData + seqdHead.addr_pre_start);
    setWAVE(sgxdData + waveHead.addr_wave_start);
    setDATA(sgxdData + sgxdHead.data_offset);

    return 1;
}


void sgd::setBankName(uchar *in) {
    while (true) {
        if (!in[0]) break;
        bankName += *in++;
    }

    if (isDebug) fprintf(stdout, "\nBank Name: %s\n", bankName.c_str());
}

void sgd::setNAME(uchar *in) {
    if (isDebug) fprintf(stdout, "\nRetrieving names from NAME\n");

    nameBank = new s_name[nameHead.amount];
    for (int n = 0; n < nameHead.amount; ++n) {
        in = sgxdData + nameHead.addr_name_start + (n * 0x8);

        nameBank[n].name_id = getLeInt(in, 0x02);
        nameBank[n].name_type = getLeInt(in, 0x02);
        nameBank[n].name_offset = getLeInt(in, 0x04);
        nameBank[n].name = "";

        if (nameBank[n].name_offset < nameHead.addr_start ||
            nameBank[n].name_offset > (nameHead.addr_start + nameHead.length)) continue;

        in = sgxdData + nameBank[n].name_offset;
        while (true) {
            if (!in[0]) break;
            nameBank[n].name += *in++;
        }
    }

    //Reorganize names
    std::sort(nameBank, nameBank + nameHead.amount);
}

void sgd::setRGND(uchar *in) {
    if (isDebug) fprintf(stdout, "\nRetrieving pre-definitions from RGND\n");

    rgndHead.amount = 0;

    rgndBank = new s_rgnd[rgndHead.amount_pre];
    for (int d = 0; d < rgndHead.amount_pre; ++d) {
        in = sgxdData + rgndHead.addr_pre_start + (0x08 * d);

        rgndBank[d].amount_tones = getLeInt(in, 0x04);
        rgndBank[d].tone_offset = getLeInt(in, 0x04);

        rgndHead.amount += rgndBank[d].amount_tones;

        if (isDebug) fprintf(stdout, "    Preset: %u\n", d);
        if (isDebug) fprintf(stdout, "        Offset to tones section: 0x%08X\n", rgndBank[d].tone_offset);
        if (isDebug) fprintf(stdout, "        Number of tones in section: %u\n", rgndBank[d].amount_tones);

        rgndBank[d].tones = new tone[rgndBank[d].amount_tones];
        for (int t = 0; t < rgndBank[d].amount_tones; ++t) {
            in = sgxdData + rgndBank[d].tone_offset + (0x38 * t);

            rgndBank[d].tones[t].priority = getLeInt(in, 0x04);
            rgndBank[d].tones[t].group_id = getLeInt(in, 0x04);
            rgndBank[d].tones[t].noise_clock = getLeInt(in, 0x04);
            rgndBank[d].tones[t].bank = getLeInt(in, 0x01);
            rgndBank[d].tones[t].value_0 = getLeInt(in, 0x01);
            rgndBank[d].tones[t].value_1 = getLeInt(in, 0x01);
            rgndBank[d].tones[t].value_2 = getLeInt(in, 0x01);
            rgndBank[d].tones[t].value_3 = getLeInt(in, 0x04);
            rgndBank[d].tones[t].value_4 = getLeInt(in, 0x04);
            rgndBank[d].tones[t].range_low = getLeInt(in, 0x01);
            rgndBank[d].tones[t].range_high = getLeInt(in, 0x01);
            rgndBank[d].tones[t].reserved0 = getLeInt(in, 0x02);
            rgndBank[d].tones[t].root_key = getLeInt(in, 0x01);
            rgndBank[d].tones[t].fine_tune = getLeInt(in, 0x01);
            rgndBank[d].tones[t].volume_0 = getLeInt(in, 0x02);
            //rgndBank[d].tones[t].mod_0 = getLeInt(in, 0x01);
            rgndBank[d].tones[t].mod_1 = getLeInt(in, 0x02);
            //rgndBank[d].tones[t].mod_2 = getLeInt(in, 0x01);
            rgndBank[d].tones[t].pan = getLeInt(in, 0x02);
            rgndBank[d].tones[t].volume_1 = getLeInt(in, 0x02);
            rgndBank[d].tones[t].reserved1 = getLeInt(in, 0x02);
            rgndBank[d].tones[t].port_travel = getLeInt(in, 0x04);
            rgndBank[d].tones[t].mod_3 = getLeInt(in, 0x01);
            rgndBank[d].tones[t].mod_4 = getLeInt(in, 0x01);
            rgndBank[d].tones[t].mod_5 = getLeInt(in, 0x01);
            rgndBank[d].tones[t].mod_6 = getLeInt(in, 0x01);
            rgndBank[d].tones[t].final_val = getLeInt(in, 0x04);
            rgndBank[d].tones[t].sample_id = getLeInt(in, 0x04);

            //Catch root greater than 127
            if (rgndBank[d].tones[t].root_key > 0x7F) {
                rgndBank[d].tones[t].semi_tune = rgndBank[d].tones[t].root_key % 0x7F;
                rgndBank[d].tones[t].root_key = 0x7F;
            }
            else { rgndBank[d].tones[t].semi_tune = 0x00; }

            if (isDebug) fprintf(stdout, "            Priority: %u\n", rgndBank[d].tones[t].priority);
            if (isDebug) fprintf(stdout, "            Lowest range: %u\n", rgndBank[d].tones[t].range_low);
            if (isDebug) fprintf(stdout, "            Highest range: %u\n", rgndBank[d].tones[t].range_high);
            if (isDebug) fprintf(stdout, "            Root key: %u\n", rgndBank[d].tones[t].root_key);
            if (isDebug) fprintf(stdout, "            Semi tune: %d\n", rgndBank[d].tones[t].semi_tune);
            if (isDebug) fprintf(stdout, "            Fine tune: %d\n", rgndBank[d].tones[t].fine_tune);
            //if (isDebug) fprintf(stdout, "            Modifier 1: %d\n", rgndBank[d].tones[t].mod_0);
            if (isDebug) fprintf(stdout, "            Modifier 2: %d\n", rgndBank[d].tones[t].mod_1);
            //if (isDebug) fprintf(stdout, "            Modifier 3: %d\n", rgndBank[d].tones[t].mod_2);
            if (isDebug) fprintf(stdout, "            Pan: %i\n", rgndBank[d].tones[t].pan);
            if (isDebug) fprintf(stdout, "            Sample ID: %d\n\n", rgndBank[d].tones[t].sample_id);
        }
    }
}

void sgd::setSEQD(uchar *in) {
    if (isDebug) fprintf(stdout, "\nRetrieving sequences from SEQD\n");

    seqdHead.amount = 0;

    //Get pre-def offsets first
    uint32_t pre_def_offs[seqdHead.amount_pre] = {};
    for (int pre = 0; pre < seqdHead.amount_pre; ++pre) {
        pre_def_offs[pre] = getLeInt(in, 0x04);
    }

    seqdBank = new s_seqd[seqdHead.amount_pre];
    for (int s = 0; s < seqdHead.amount_pre; ++s) {
        if (!pre_def_offs[s]) { seqdBank[s].seqs = 0; continue; }

        in = sgxdData + pre_def_offs[s];
        in += 0x04;

        seqdBank[s].amount = getLeInt(in, 0x04);

        //Get offsets first
        uint32_t seq_offs[seqdBank[s].amount] = {};
        for (int off = 0; off < seqdBank[s].amount; ++off) {
            seq_offs[off] = getLeInt(in, 0x04);
        }

        seqdBank[s].seqs = new seq[seqdBank[s].amount];
        for (int m = 0; m < seqdBank[s].amount; ++m) {
            if (!seq_offs[m]) { seqdBank[s].seqs[m].seq_data = 0; continue; }

            in = sgxdData + seq_offs[m];
            in += 0x04;

            uint32_t nam_off = getLeInt(in, 0x04);
            if (!nam_off) { seqdBank[s].seqs[m].seq_data = 0; continue; }

            seqdBank[s].seqs[m].seq_format = getLeInt(in, 0x01);
            seqdBank[s].seqs[m].seq_data = in;

            in = sgxdData + nam_off;

            seqdBank[s].seqs[m].name = "";
            while (true) {
                if (!in[0]) break;
                seqdBank[s].seqs[m].name += *in++;
            }

            seqdHead.amount += 1;

            if (isDebug) fprintf(stdout, "    Name of sequence: %s\n", seqdBank[s].seqs[m].name.c_str());
            if (isDebug) fprintf(stdout, "        Format of sequence: %u\n", seqdBank[s].seqs[m].seq_format);
            if (isDebug) fprintf(stdout, "        Group ID %u, Song ID %u\n", s, m);
        }
    }
}

void sgd::setWAVE(uchar *in) {
    if (isDebug) fprintf(stdout, "\nRetrieving Sample definitions from WAVE\n");

    waveBank = new s_wave[waveHead.amount];
    for (int w = 0; w < waveHead.amount; ++w) {
        in = sgxdData + waveHead.addr_wave_start + (0x38 * w);

        if (isDebug) fprintf(stdout, "    Sample %u definitions\n", w);

        waveBank[w].voice_id = getLeInt(in, 0x04);
        waveBank[w].name_offset = getLeInt(in, 0x04);
        waveBank[w].codec = getLeInt(in, 0x01);
        waveBank[w].channels = getLeInt(in, 0x01);
        waveBank[w].reserved1 = getLeInt(in, 0x02);
        waveBank[w].sample_rate = getLeInt(in, 0x04);
        waveBank[w].info_type = getLeInt(in, 0x04);
        waveBank[w].info_value = getLeInt(in, 0x04);
        waveBank[w].volume_0 = getLeInt(in, 0x02);
        waveBank[w].volume_1 = getLeInt(in, 0x02);
        waveBank[w].reserved3 = getLeInt(in, 0x04);
        waveBank[w].sample_size = getLeInt(in, 0x04);
        waveBank[w].loop_start = getLeInt(in, 0x04);
        waveBank[w].loop_end = getLeInt(in, 0x04);
        waveBank[w].stream_size = getLeInt(in, 0x04);
        waveBank[w].stream_offset = getLeInt(in, 0x04);
        waveBank[w].stream_size_full = getLeInt(in, 0x04);

        //Fix stream size
        if (waveBank[w].stream_size > sgxdSize) {
            waveBank[w].stream_size = sgxdSize;
        }

        //Set pointer to stream data
        waveBank[w].data = sgxdData + sgxdHead.data_offset + waveBank[w].stream_offset;

        //Determine loop type
        waveBank[w].loop_type += ((waveBank[w].loop_start < 0) ? 0 : 1);
        waveBank[w].loop_type += ((waveBank[w].loop_end < 0) ? 0 : 1);

        //Fix negative loop points
        if (waveBank[w].loop_start < 0) waveBank[w].loop_start = 0;
        if (waveBank[w].loop_end < 0) waveBank[w].loop_end = waveBank[w].sample_size;

        //Name sample
        waveBank[w].name = "samp_" + std::to_string(w);


        if (isDebug) fprintf(stdout, "        Name offset: 0x%08X\n", waveBank[w].name_offset);
        if (isDebug) fprintf(stdout, "        Name: %s\n", waveBank[w].name.c_str());
        if (isDebug) fprintf(stdout, "        Codec: %u\n", waveBank[w].codec);
        if (isDebug) fprintf(stdout, "        Channels: %u\n", waveBank[w].channels);
        if (isDebug) fprintf(stdout, "        Sample rate: %u\n", waveBank[w].sample_rate);
        if (isDebug) fprintf(stdout, "        Info type: 0x%08X\n", waveBank[w].info_type);
        if (isDebug) fprintf(stdout, "        Info value: 0x%08X\n", waveBank[w].info_value);
        if (isDebug) fprintf(stdout, "        Short samples per channel: 0x%08X\n", waveBank[w].sample_size);
        if (isDebug) fprintf(stdout, "        Short sample loop start: 0x%08X\n", waveBank[w].loop_start);
        if (isDebug) fprintf(stdout, "        Short sample loop end: 0x%08X\n", waveBank[w].loop_end);
        if (isDebug) fprintf(stdout, "        Compressed sample size: 0x%08X\n", waveBank[w].stream_size);
        if (isDebug) fprintf(stdout, "        Compressed sample offset: 0x%08X\n", waveBank[w].stream_offset);
        if (isDebug) fprintf(stdout, "        Compressed sample size + padding: 0x%08X\n", waveBank[w].stream_size_full);
    }
}

void sgd::setDATA(uchar *in) {
    if (isDebug) fprintf(stdout, "\nRetrieving Samples\n");

    for (int w = 0; w < waveHead.amount; ++w) {
        if (!waveBank[w].stream_size   ||
             waveBank[w].channels != 1 ||
            !waveBank[w].data          ||
             waveBank[w].data >= (sgxdData + sgxdSize)) continue;

        if (isDebug) fprintf(stdout, "Sample %u Codec: ", w);
        switch (waveBank[w].codec) {
            case 0x01:      //HEADERLESS Big Endian PCM16
                if (isDebug) fprintf(stdout, "Big Endian PCM16\n");
                continue;

            case 0x02:      //Ogg Vorbis
                if (isDebug) fprintf(stdout, "Ogg Vorbis\n");
                oggVorbDecode(waveBank[w].data, waveBank[w].stream_size, waveBank[w].channels,
                              waveBank[w].sample_rate, waveBank[w].pcmle, waveBank[w].sample_size);
                continue;

            case 0x03:      //HEADERLESS Sony ADPCM
                if (isDebug) fprintf(stdout, "Sony ADPCM\n");

                if (!waveBank[w].sample_size) {
                    //Calculate approximate sample size using stream size just in case
                    waveBank[w].sample_size = 28 * (waveBank[w].stream_size / 16.00);
                }

                adpcmDecode(waveBank[w].data, waveBank[w].stream_size, waveBank[w].loop_type,
                            waveBank[w].pcmle, waveBank[w].sample_size);
                continue;

            case 0x04:      //Sony Atrac3Plus
                if (isDebug) fprintf(stdout, "Sony Atrac3Plus\n");
                continue;

            case 0x05:      //HEADERLESS Sony Short ADPCM
                if (isDebug) fprintf(stdout, "Sony IMA-ADPCM\n");
                continue;

            case 0x06:      //Dolby AC-3
                if (isDebug) fprintf(stdout, "Dolby AC-3\n");
                continue;

            default:        //Unknown
                if (isDebug) fprintf(stdout, "Unknown '%u'\n", waveBank[w].codec);
                continue;
        }
    }
}


std::string sgd::getBankName() { return bankName; }
std::string sgd::getSampleName(int samp) { return waveBank[samp].name; }

int sgd::getAmountAudio() { return waveHead.amount; }
int sgd::getAmountPrograms() { return rgndHead.amount_pre; }
int sgd::getAmountTones() { return rgndHead.amount; }
int sgd::getAmountSequence() { return seqdHead.amount; }

int sgd::getSampleCodec(int samp) { return waveBank[samp].codec; }
int sgd::getSampleChannels(int samp) { return waveBank[samp].channels; }
int sgd::getSampleSamplerate(int samp) { return waveBank[samp].sample_rate; }
int sgd::getSampleSize(int samp) { return waveBank[samp].sample_size; }
uint8_t sgd::getSampleLpType(int samp) { return waveBank[samp].loop_type; }
uint32_t sgd::getSampleLpStrt(int samp) { return waveBank[samp].loop_start; }
uint32_t sgd::getSampleLpTrm(int samp) { return waveBank[samp].loop_end; }

int sgd::getAmountProgramTones(int prgm) { return rgndBank[prgm].amount_tones; }
uint8_t sgd::getToneBank(int prgm, int tone) { return rgndBank[prgm].tones[tone].bank; }
//uint8_t sgd::getTonePreset(int prgm, int tone) { return rgndBank[prgm].tones[tone].preset; }
uint8_t sgd::getToneID(int prgm, int tone) { return rgndBank[prgm].tones[tone].value_0; }
uint8_t sgd::getToneLow(int prgm, int tone) { return rgndBank[prgm].tones[tone].range_low; }
uint8_t sgd::getToneHigh(int prgm, int tone) { return rgndBank[prgm].tones[tone].range_high; }
uint8_t sgd::getToneRoot(int prgm, int tone) { return rgndBank[prgm].tones[tone].root_key; }
int8_t sgd::getToneTuneSemi(int prgm, int tone) { return rgndBank[prgm].tones[tone].semi_tune; }
int8_t sgd::getToneTuneFine(int prgm, int tone) { return rgndBank[prgm].tones[tone].fine_tune; }
int16_t sgd::getToneAttack(int prgm, int tone) { return -32768; }
int16_t sgd::getToneDecay(int prgm, int tone) { return -32768; }
int16_t sgd::getToneSustain(int prgm, int tone) { return 0; }
int16_t sgd::getToneRelease(int prgm, int tone) { return rgndBank[prgm].tones[tone].mod_1; }
int16_t sgd::getTonePan(int prgm, int tone) { return ((int)rgndBank[prgm].tones[tone].pan /* * 125 */); }
uint16_t sgd::getTonePortTime(int prgm, int tone) { return rgndBank[prgm].tones[tone].port_travel; }
int32_t sgd::getToneSampleID(int prgm, int tone) { return rgndBank[prgm].tones[tone].sample_id; }

int sgd::getSample(int samp, int16_t *out) {
    int ret = 0;

    if (samp >= waveHead.amount || samp < 0 || waveBank[samp].channels != 1) {
        out = 0;
    }
    else {
        int16_t *in_beg = waveBank[samp].pcmle,
                *in_end = waveBank[samp].pcmle + waveBank[samp].sample_size;
        std::copy(in_beg, in_end, out);
        ret = 1;
    }

    return ret;
}

void sgd::writeSequences() {
    const char *MThd = "MThd",
               *MThd_SIZE = "\x00\x00\x00\x06",
               *MThd_FRMT = "\x00\x01",
               *MThd_TRKS = "\x00\x01",
               *MThd_DIVI = "\x9E\xC4", //Division 1, 30 FPS, 196 Ticks per frame (?)
               *MTrk = "MTrk";

    for (int b = 0; b < seqdHead.amount_pre; ++b) {
        if (!seqdBank[b].seqs) continue;

        for (int g = 0; g < seqdBank[b].amount; ++g) {
            seq &t_seq = seqdBank[b].seqs[g];

            if (!t_seq.seq_data || !t_seq.seq_format) continue;
            uchar *dat = t_seq.seq_data;

            std::string full_name = pathName + t_seq.name;
            if (seqdBank[b].amount > 1) full_name += getFData(" (GID %d SID %d)", b, g);

            std::ofstream mid(full_name + ".mid", std::ios::binary);
            try {
                mid.seekp(0x00);
                mid.write(MThd, 4);
                mid.write(MThd_SIZE, 4);
                mid.write(MThd_FRMT, 2);
                mid.write(MThd_TRKS, 2);
                mid.write(MThd_DIVI, 2);
                mid.write(MTrk, 4);
                mid.write(getFData("0000"), 4);

                uint32_t init_pos = mid.tellp();

                if (seqdBank[b].amount > 1) mid.write(getFData("%c%c%c%c%c%c", 0x00, 0xFF, 0x00, 0x02, g >> 8, g), 6);
                mid.write(getFData("%c%c%c%c%s", 0x00, 0xFF, 0x03, t_seq.name.size(), t_seq.name.c_str()), 4 + t_seq.name.size());

                while (true) {
                    if ((t_seq.seq_format == 1 && cmpChar(&dat[0], "\xFF\x2F", 2)) ||
                        (t_seq.seq_format == 0 && cmpChar(&dat[0], "\xFF\x00", 2))) {
                        dat += 2;

                        mid.write(getFData("%c%c%c", 0xFF, 0x2F, 0x00), 3);
                        break;
                    }
                    else if ((dat[0] & 0xF0) == 0xB0) {
                        //fprintf(stdout, "    CC\n");
                        if (cmpChar(&dat[1], "\x63\x14", 2)) {
                            //fprintf(stdout, "    Loop start\n");
                            mid.write(getFData("%c%c%c%s", 0xFF, 0x06, 9, "loopStart"), 12);
                            mid.write(getFData("%c%c%c%c", 0x00, dat[0], 0x74, 0x00), 4);
                        }
                        else if (cmpChar(&dat[1], "\x63\x1E", 2)) {
                            //fprintf(stdout, "    Loop end\n");
                            mid.write(getFData("%c%c%c%s", 0xFF, 0x06, 7, "loopEnd"), 10);
                            mid.write(getFData("%c%c%c%c", 0x00, dat[0], 0x75, 0x7F), 4);
                        }
                        else if (dat[1] == 0x76 || dat[1] == 0x77) {
                            //fprintf(stdout, "    Faulty loops\n");
                            mid.write(getFData("%c%c%c", dat[0], 0x03, dat[2]), 3);
                        }
                        else mid.write(getFData("%c%c%c", dat[0], dat[1], dat[2]), 3);

                        dat += 3;
                    }
                    else mid.put(*dat++);
                }
                uint32_t term_pos = mid.tellp();
                term_pos -= init_pos;

                mid.seekp(init_pos - 4);
                mid.write(getFData("%c%c%c%c", term_pos >> 24, term_pos >> 16, term_pos >> 8, term_pos), 4);

                fprintf(stdout, "    %s successfully extracted\n", t_seq.name.c_str());
            }
            catch (const std::fstream::failure & e) {
                fprintf(stderr, "    Unable to extract \"%s\"\n", t_seq.name.c_str());
                fprintf(stderr, "        %s\n", e.what());
            }
            mid.close();
        }
    }
}
