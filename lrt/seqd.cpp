#include <algorithm>
#include <bitset>
#include <cmath>
#include <cstdio>
#include <ctime>
#include <map>
#include <utility>
#include <vector>
#include "sgxd_const.hpp"
#include "sgxd_types.hpp"
#include "sgxd_func.hpp"
#include "midi/midi_const.hpp"
#include "midi/midi_types.hpp"
#include "midi/midi_func.hpp"


///Unpacks variable sequence definitions from SEQD data
void unpackSeqd(unsigned char *in, const unsigned length) {
    if (sgd_debug) fprintf(stderr, "    Unpack SEQD\n");

    sgd_inf.seqd = {};
    if (!sgd_beg || !in || length < 8) return;

    const unsigned char *in_end = in + length;
    unsigned t_sz;
    auto &out = sgd_inf.seqd;
    
    struct seqd_vals {
        int v[3] {};
        int& operator[](const int &i) { return v[i]; }
        const int& operator[](const int &i) const { return v[i]; }
    };

    auto get_int = [&in, &in_end](const unsigned length) -> unsigned {
        unsigned out = 0;
        for (int i = 0; i < length; ++i) {
            if (in >= in_end) break;
            out |= (unsigned)*(in++) << (8 * i);
        }
        return out;
    };
    auto get_str = [](const unsigned char *in, const unsigned adr) -> std::string {
        std::string out;
        if (!adr);
        else { out = (const char*)(in + adr); if (!out[0]) out.clear(); }
        return out;
    };
    auto get_fstr = []<typename... T>(const char *in, T&&... args) -> std::string {
        std::string out; int s0 = snprintf(nullptr, 0, in, args...) + 1;
        out.resize(s0 - 1); snprintf(out.data(), s0, in, args...);
        return out;
    };
    auto get_bcd = [](auto &in, int siz, const bool is_neg, const unsigned def = 0) -> int {
            if (!in[0]) return def;
            
            char tmp[16] {};
            for (int z = 0; siz-- > 0 && *in < 0xA0; z += 2) {
                snprintf(tmp + z, 3, "%02X", *in++);
            }
            
            return strtol(tmp, nullptr, 10) * ((!is_neg) ? 1 : -1);
        };
    auto get_val = [&get_bcd](auto&& self, auto &in, const unsigned def = 0) -> int {
        int typ, out;
        if (*in >= 0xA0) return def;

        typ = *in++;
        if (typ >> 4 == SEQD_RANDOM) {
            int b0, b1, b2, b3;
            b0 = self(self, in);    // Minimum
            b1 = self(self, in);    // Maximum
            b2 = self(self, in);    // Increment
            b3 = (!b2 ? 0 : abs(b1 - b0) / abs(b2)) + 1;
            int b4[b3] {};
            for (auto &b : b4) { b = b0; b0 += b2; }
            out = b4[rand() % b3];
        }
        else if (typ >> 4 < SEQD_SKIP) {
            out = get_bcd(in, typ & 0x0F, typ >> 4 == SEQD_NEGATIVE, def);
        }
        else {
            auto end = in + (typ & 0x0F);
            while (in < end) self(self, in);
            out = def;
        }
        return out;
    };
    auto set_bnk = [&sgd_inf](const int &prs, const int &nte, int &bnk) -> void {
        bnk = -1;
        for (const auto &rgn : sgd_inf.rgnd.rgnd) {
            if (&rgn - sgd_inf.rgnd.rgnd.data() != prs) continue;
            for (const auto &ton : rgn) {
                if (bnk < 0 && nte >= ton.notelow && nte <= ton.notehigh) {
                    bnk = ton.bnkid;
                    break;
                }
            }
            break;
        }
    };
    auto chk_seq = [&sgd_inf](const int &grp, const int &seq) -> bool {
        return (grp >= 0 && grp < sgd_inf.seqd.seqd.size()) &&
               (seq >= 0 && seq < sgd_inf.seqd.seqd[grp].seq.size()) &&
               !sgd_inf.seqd.seqd[grp].seq[seq].empty();
    };
    auto get_mid = [](const std::vector<mesginfo> &in) -> std::vector<unsigned char> {
        std::vector<unsigned char> out;
        const mesginfo *prv = 0;
        for (const auto &m : in) {
            auto dat = m.getAll(prv);
            prv = &m;
            out.insert(out.end(), dat.begin(), dat.end());
            if (m.getStat() == META_END_OF_SEQUENCE) break;
        }
        return std::move(out);
    };
    
    
    if (sgd_debug) fprintf(stderr, "    Read SEQD Header\n");
    out.flag = get_int(4);
    out.seqd.resize(get_int(4));
    signed seq0offs[out.seqd.size()] {};

    if (sgd_debug) fprintf(stderr, "        Global flag: 0x%08X\n", out.flag);

    if (sgd_debug) fprintf(stderr, "    Read SEQD Setup\n");
    for (auto &f : seq0offs) f = get_int(4);

    if (sgd_debug) fprintf(stderr, "    Read SEQD Group\n");
    for (int g = 0; g < out.seqd.size(); ++g) {
        if (!seq0offs[g]) continue;
        in = (unsigned char*)sgd_beg + seq0offs[g];
        out.seqd[g].flag = get_int(4);
        out.seqd[g].seq.resize(get_int(4));
        signed seq1offs[out.seqd[g].seq.size()] {};
        for (auto &f : seq1offs) f = get_int(4);

        if (sgd_debug) {
            fprintf(stderr, "        Current SEQD Group: %d\n", g);
            fprintf(stderr, "            Group flag: 0x%08X\n", out.seqd[g].flag);
        }

        for (int f = 0; f < out.seqd[g].seq.size(); ++f) {
            if (!seq1offs[f]) continue;
            in = (unsigned char*)sgd_beg + seq1offs[f];
            out.seqd[g].seq[f].flag = get_int(4);
            out.seqd[g].seq[f].name = get_str(sgd_beg, get_int(4));
            out.seqd[g].seq[f].fmt = get_int(2);
            out.seqd[g].seq[f].div = get_int(2);
            out.seqd[g].seq[f].volleft = get_int(2);
            out.seqd[g].seq[f].volright = get_int(2);
            t_sz = get_int(4);
            if (in + t_sz <= in_end) out.seqd[g].seq[f].data.assign(in, in + t_sz);

            if (sgd_debug) {
                fprintf(stderr, "            Current SEQD: %d\n", f);
                fprintf(stderr, "                Local flag: 0x%08X\n", out.seqd[g].seq[f].flag);
                fprintf(stderr, "                Name: %s\n", out.seqd[g].seq[f].name.c_str());
                fprintf(stderr, "                Format: %d\n", out.seqd[g].seq[f].fmt);
                fprintf(stderr, "                Division: 0x%04X\n", out.seqd[g].seq[f].div);
                fprintf(stderr, "                Left volume: %d\n", out.seqd[g].seq[f].volleft);
                fprintf(stderr, "                Right volume: %d\n", out.seqd[g].seq[f].volright);
                fprintf(stderr, "                Sequence size: %d\n", out.seqd[g].seq[f].data.size());
            }
        }
    }
    
    if (sgd_debug) fprintf(stderr, "    Convert SEQD (First Pass)\n");
    srand(time(0));
    for (auto &grp : out.seqd) {
        if (grp.seq.empty()) continue;
        
        if (sgd_debug) fprintf(stderr, "        Current SEQD Group: %d\n", &grp - out.seqd.data());
        for (auto &seq : grp.seq) {
            if (seq.empty()) continue;
            if (seq.fmt != SEQD_REQUEST && seq.fmt != SEQD_RAWMIDI) continue;
            
            std::vector<mesginfo> tmp;
            mid_inf = {};
            t_sz = 0;
            
            if (sgd_debug) fprintf(stderr, "            Current SEQD: %d\n", &seq - grp.seq.data());
            if (seq.fmt == SEQD_REQUEST) {
                //https://github.com/Nenkai/010GameTemplates/blob/main/Sony/SGXD.bt
                
                std::map<int, seqd_vals> ids;
                
                seq.asmdata = "abstime_000000:\n";
                
                //Convert PSX-style requests into midi and asm
                if (sgd_debug) fprintf(stderr, "                Convert PSX-style requests\n");
                for (auto ptr = seq.data.begin(); ptr < seq.data.end();) {
                    while (*ptr < 0xA0) ptr += 1;

                    switch((char)(*ptr++)) {
                        //Auditory instructions
                        case SEQD_SUB_START: {
                            if (sgd_debug) fprintf(stderr, "                    Set song start\n");
                            int id, prg, nte, vol;
                            id = get_val(get_val, ptr);
                            prg = get_val(get_val, ptr);
                            nte = get_val(get_val, ptr);
                            vol = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        ID: %d\n", id);
                                fprintf(stderr, "                        Group: %d\n", prg);
                                fprintf(stderr, "                        Sequence: %d\n", nte);
                                fprintf(stderr, "                        Volume: %d\n", vol);
                            }
                            
                            if (chk_seq(prg, nte)) {
                                tmp.emplace_back(
                                    t_sz,
                                    META_SEQUENCER_EXCLUSIVE,
                                    (unsigned char[]){SEQD_SUB_START, prg, nte}
                                );
                                ids[id] = {prg, nte};
                            }
                            continue;
                        }
                        case SEQD_SUB_STOP: {
                            if (sgd_debug) fprintf(stderr, "                    Set song stop\n");
                            int id;
                            id = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        ID: %d\n", id);
                            }
                            
                            if (ids.find(id) != ids.end()) {
                                tmp.emplace_back(
                                    t_sz,
                                    META_SEQUENCER_EXCLUSIVE,
                                    (unsigned char[]){SEQD_SUB_STOP, ids[id][0], ids[id][1]}
                                );
                            }
                            continue;
                        }
                        case SEQD_SUB_STOPREL: {
                            if (sgd_debug) fprintf(stderr, "                    Set song fade\n");
                            int id;
                            id = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        ID: %d\n", id);
                            }
                            
                            if (ids.find(id) != ids.end()) {
                                tmp.emplace_back(
                                    t_sz,
                                    META_SEQUENCER_EXCLUSIVE,
                                    (unsigned char[]){SEQD_SUB_STOPREL, ids[id][0], ids[id][1]}
                                );
                            }
                            continue;
                        }
                        case SEQD_SUB_GETSTAT: {
                            if (sgd_debug) fprintf(stderr, "                    Set song status\n");
                            int id;
                            id = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        ID: %d\n", id);
                            }
                            
                            if (ids.find(id) != ids.end()) {
                                tmp.emplace_back(
                                    t_sz,
                                    META_SEQUENCER_EXCLUSIVE,
                                    (unsigned char[]){SEQD_SUB_GETSTAT, ids[id][0], ids[id][1]}
                                );
                            }
                            continue;
                        }
                        case SEQD_CONTROL: {
                            if (sgd_debug) fprintf(stderr, "                    Set controller\n");
                            int id, typ, val, tim, unk, glb;
                            id = get_val(get_val, ptr);
                            typ = get_val(get_val, ptr);
                            val = get_val(get_val, ptr);
                            tim = get_val(get_val, ptr);
                            unk = get_val(get_val, ptr);
                            glb = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        ID: %d\n", id);
                                fprintf(stderr, "                        Type: %d\n", typ);
                                fprintf(stderr, "                        Value: %d\n", val);
                                fprintf(stderr, "                        Time: %d\n", tim);
                                fprintf(stderr, "                        Unknown: %d\n", unk);
                                fprintf(stderr, "                        Is global: %d\n", glb);
                            }
                            //Dunno the controller types, come back later
                            continue;
                        }
                        case SEQD_ADSR: {
                            if (sgd_debug) fprintf(stderr, "                    Set envelope id\n");
                            int id, typ, glb;
                            id = get_val(get_val, ptr);
                            typ = get_val(get_val, ptr);
                            glb = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        ID: %d\n", id);
                                fprintf(stderr, "                        Type: %d\n", typ);
                                fprintf(stderr, "                        Is global: %d\n", glb);
                            }
                            //Dunno the ADSR indices, come back later
                            continue;
                        }
                        case SEQD_BEND: {
                            if (sgd_debug) fprintf(stderr, "                    Set bend\n");
                            int unk0, unk1, unk2, glb;
                            unk0 = get_val(get_val, ptr);
                            unk1 = get_val(get_val, ptr, 12);
                            unk2 = get_val(get_val, ptr, 12);
                            glb = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        Unknown 1: %d\n", unk0);
                                fprintf(stderr, "                        Unknown 2: %d\n", unk1);
                                fprintf(stderr, "                        Unknown 3: %d\n", unk2);
                                fprintf(stderr, "                        Is global: %d\n", glb);
                            }
                            //Dunno how bend works, come back later
                            continue;
                        }
                        case SEQD_ADSR_DIRECT: {
                            if (sgd_debug) fprintf(stderr, "                    Set envelope\n");
                            int p0, unk, p0_0, p0_1, p0_2, p0_3, p1_0, p1_1, p1_2, p1_3, glb;
                            p0 = get_val(get_val, ptr);
                            unk = get_val(get_val, ptr);
                            p0_0 = get_val(get_val, ptr);
                            p0_1 = std::min(127, get_val(get_val, ptr));
                            p0_2 = std::min(15, get_val(get_val, ptr));
                            p0_3 = std::min(15, get_val(get_val, ptr));
                            p1_0 = get_val(get_val, ptr);
                            p1_1 = get_val(get_val, ptr);
                            p1_2 = get_val(get_val, ptr);
                            p1_3 = get_val(get_val, ptr);
                            glb = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        Type: %d\n", p0);
                                fprintf(stderr, "                        Value 1: 0x%02X02X02X02X\n", p0_0, p0_1, p0_2, p0_3);
                                fprintf(stderr, "                        Value 2: 0x%02X02X02X02X\n", p1_0, p1_1, p1_2, p1_3);
                                fprintf(stderr, "                        Is global: %d\n", glb);
                            }
                            //Too lazy, come back later
                            continue;
                        }
                        case SEQD_START: {
                            if (sgd_debug) fprintf(stderr, "                    Set note start\n");
                            int id, prg, nte, vol, bnk;
                            id = get_val(get_val, ptr);
                            prg = get_val(get_val, ptr);
                            nte = get_val(get_val, ptr);
                            vol = get_val(get_val, ptr, 4096);
                            if (sgd_debug) {
                                fprintf(stderr, "                        ID: %d\n", id);
                                fprintf(stderr, "                        Region: %d\n", prg);
                                fprintf(stderr, "                        Note: %d\n", nte);
                                fprintf(stderr, "                        Volume: %d\n", vol);
                            }
                            
                            set_bnk(prg, nte, bnk);
                            if (bnk >= 0) {
                                if (sgd_debug) fprintf(stderr, "                        Bank: %d\n", bnk);
                                tmp.emplace_back(
                                    t_sz,
                                    META_SEQUENCER_EXCLUSIVE,
                                    (unsigned char[]){SEQD_START, bnk, prg, nte, (vol * 127) / 4096}
                                );
                                ids[id] = {bnk, prg, nte};
                            }
                            continue;
                        }
                        case SEQD_STOP: {
                            if (sgd_debug) fprintf(stderr, "                    Set note stop\n");
                            int id, glb;
                            id = get_val(get_val, ptr);
                            glb = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        ID: %d\n", id);
                                fprintf(stderr, "                        Is global: %d\n", glb);
                            }
                            
                            if (!glb && ids.find(id) == ids.end()) continue;
                            if (glb) tmp.emplace_back(
                                t_sz, META_SEQUENCER_EXCLUSIVE,
                                (unsigned char[]){SEQD_STOP}
                            );
                            else tmp.emplace_back(
                                t_sz, META_SEQUENCER_EXCLUSIVE,
                                (unsigned char[]){SEQD_STOP, ids[id][0], ids[id][1], ids[id][2], 0}
                            );
                            continue;
                        }
                        case SEQD_STOPREL: {
                            if (sgd_debug) fprintf(stderr, "                    Set note fade\n");
                            int id, glb;
                            id = get_val(get_val, ptr);
                            glb = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        ID: %d\n", id);
                                fprintf(stderr, "                        Is global: %d\n", glb);
                            }
                            
                            if (!glb && ids.find(id) == ids.end()) continue;
                            if (glb) tmp.emplace_back(
                                t_sz, META_SEQUENCER_EXCLUSIVE,
                                (unsigned char[]){SEQD_STOPREL}
                            );
                            else tmp.emplace_back(
                                t_sz, META_SEQUENCER_EXCLUSIVE,
                                (unsigned char[]){SEQD_STOPREL, ids[id][0], ids[id][1], ids[id][2], 127}
                            );
                            continue;
                        }
                        case SEQD_GETPORTSTAT: {
                            if (sgd_debug) fprintf(stderr, "                    Set note status\n");
                            int id, glb;
                            id = get_val(get_val, ptr);
                            glb = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        ID: %d\n", id);
                                fprintf(stderr, "                        Is global: %d\n", glb);
                            }
                            
                            if (!glb && ids.find(id) == ids.end()) continue;
                            if (glb) tmp.emplace_back(
                                t_sz, META_SEQUENCER_EXCLUSIVE,
                                (unsigned char[]){SEQD_GETPORTSTAT}
                            );
                            else tmp.emplace_back(
                                t_sz, META_SEQUENCER_EXCLUSIVE,
                                (unsigned char[]){SEQD_GETPORTSTAT, ids[id][0], ids[id][1], ids[id][2]}
                            );
                            continue;
                        }
                        case SEQD_STARTSMPL: {
                            if (sgd_debug) fprintf(stderr, "                    Set sample start\n");
                            int id, sid, vol, pri, grp, gmd, gnm;
                            id = get_val(get_val, ptr);
                            sid = get_val(get_val, ptr);
                            vol = get_val(get_val, ptr, 4096);
                            pri = get_val(get_val, ptr, 128);
                            grp = get_val(get_val, ptr);
                            gmd = get_val(get_val, ptr);
                            gnm = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        ID: %d\n", id);
                                fprintf(stderr, "                        Sample ID: %d\n", sid);
                                fprintf(stderr, "                        Volume: %d\n", vol);
                                fprintf(stderr, "                        Priority: %d\n", pri);
                                fprintf(stderr, "                        Group ID 1: %d\n", grp);
                                fprintf(stderr, "                        Group mode: %d\n", gmd);
                                fprintf(stderr, "                        Group ID 2: %d\n", gnm);
                            }
                            //Need example, come back later
                            continue;
                        }
                        case SEQD_STARTNOISE: {
                            if (sgd_debug) fprintf(stderr, "                    Set noise start\n");
                            int id, nid, vol, pri, grp, gmd, gnm;
                            id = get_val(get_val, ptr);
                            nid = get_val(get_val, ptr);
                            vol = get_val(get_val, ptr, 4096);
                            pri = get_val(get_val, ptr, 128);
                            grp = get_val(get_val, ptr);
                            gmd = get_val(get_val, ptr);
                            gnm = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        ID: %d\n", id);
                                fprintf(stderr, "                        Noise ID: %d\n", nid);
                                fprintf(stderr, "                        Volume: %d\n", vol);
                                fprintf(stderr, "                        Priority: %d\n", pri);
                                fprintf(stderr, "                        Group ID 1: %d\n", grp);
                                fprintf(stderr, "                        Group mode: %d\n", gmd);
                                fprintf(stderr, "                        Group ID 2: %d\n", gnm);
                            }
                            //Need example, come back later
                            continue;
                        }
                        //MIPS R4000 instructions (guesstimate)
                        case SEQD_SYSREG_INIT: {
                            if (sgd_debug) fprintf(stderr, "                    Set register\n");
                            int typ, vl0, vl1;
                            typ = get_val(get_val, ptr);
                            vl0 = get_val(get_val, ptr);
                            vl1 = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        Register: %d\n", typ);
                                fprintf(stderr, "                        Value 1: %d\n", vl0);
                                fprintf(stderr, "                        Value 2: %d\n", vl1);
                            }
                            
                            seq.asmdata += get_fstr("    li $s%d, %d\n", typ, vl0);
                            seq.asmdata += get_fstr("    addi $s%d, $s%d, %d\n", typ, typ, vl1);
                            continue;
                        }
                        case SEQD_SYSREG_ADD: {
                            if (sgd_debug) fprintf(stderr, "                    Set register ADD\n");
                            int typ, vl0, vl1;
                            typ = get_val(get_val, ptr);
                            vl0 = get_val(get_val, ptr);
                            vl1 = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        Register: %d\n", typ);
                                fprintf(stderr, "                        Value 1: %d\n", vl0);
                                fprintf(stderr, "                        Value 2: %d\n", vl1);
                            }
                            
                            seq.asmdata += get_fstr("    addi $s%d, $0, %d\n", typ, vl0);
                            seq.asmdata += get_fstr("    addi $s%d, $s%d, %d\n", typ, typ, vl1);
                            continue;
                        }
                        case SEQD_SYSREG_MINUS: {
                            if (sgd_debug) fprintf(stderr, "                    Set register SUBTRACT\n");
                            int typ, vl0, vl1;
                            typ = get_val(get_val, ptr);
                            vl0 = get_val(get_val, ptr);
                            vl1 = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        Register: %d\n", typ);
                                fprintf(stderr, "                        Value 1: %d\n", vl0);
                                fprintf(stderr, "                        Value 2: %d\n", vl1);
                            }
                            
                            seq.asmdata += get_fstr("    addi $s%d, $0, %d\n", typ, vl0);
                            seq.asmdata += get_fstr("    addi $s%d, $s%d, %d\n", typ, typ, -1 * vl1);
                            continue;
                        }
                        case SEQD_SYSREG_MULT: {
                            if (sgd_debug) fprintf(stderr, "                    Set register MULTIPLY\n");
                            int typ, vl0, vl1;
                            typ = get_val(get_val, ptr);
                            vl0 = get_val(get_val, ptr);
                            vl1 = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        Register: %d\n", typ);
                                fprintf(stderr, "                        Value 1: %d\n", vl0);
                                fprintf(stderr, "                        Value 2: %d\n", vl1);
                            }
                            
                            seq.asmdata += get_fstr("    addi $s%d, $0, %d\n", typ, vl0);
                            seq.asmdata += get_fstr("    li $t4, %d\n", vl1);
                            seq.asmdata += get_fstr("    mult $s%d, $t4\n", typ);
                            seq.asmdata += get_fstr("    mfhi $v1\n");
                            seq.asmdata += get_fstr("    mflo $s%d\n", typ);
                            continue;
                        }
                        case SEQD_SYSREG_DIVI: {
                            if (sgd_debug) fprintf(stderr, "                    Set register DIVISION\n");
                            int typ, vl0, vl1;
                            typ = get_val(get_val, ptr);
                            vl0 = get_val(get_val, ptr);
                            vl1 = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        Register: %d\n", typ);
                                fprintf(stderr, "                        Value 1: %d\n", vl0);
                                fprintf(stderr, "                        Value 2: %d\n", vl1);
                            }
                            
                            seq.asmdata += get_fstr("    addi $s%d, $0, %d\n", typ, vl0);
                            seq.asmdata += get_fstr("    li $t4, %d\n", vl1);
                            seq.asmdata += get_fstr("    div $s%d, $t4\n", typ);
                            seq.asmdata += get_fstr("    mfhi $v1\n");
                            seq.asmdata += get_fstr("    mflo $s%d\n", typ);
                            continue;
                        }
                        case SEQD_SYSREG_MODU: {
                            if (sgd_debug) fprintf(stderr, "                    Set register MODULUS\n");
                            int typ, vl0, vl1;
                            typ = get_val(get_val, ptr);
                            vl0 = get_val(get_val, ptr);
                            vl1 = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        Register: %d\n", typ);
                                fprintf(stderr, "                        Value 1: %d\n", vl0);
                                fprintf(stderr, "                        Value 2: %d\n", vl1);
                            }
                            
                            seq.asmdata += get_fstr("    addi $s%d, $0, %d\n", typ, vl0);
                            seq.asmdata += get_fstr("    li $t4, %d\n", vl1);
                            seq.asmdata += get_fstr("    div $s%d, $t4\n", typ);
                            seq.asmdata += get_fstr("    mfhi $s%d\n", typ);
                            seq.asmdata += get_fstr("    mflo $v0\n");
                            continue;
                        }
                        case SEQD_SYSREG_AND: {
                            if (sgd_debug) fprintf(stderr, "                    Set register AND\n");
                            int typ, vl0, vl1;
                            typ = get_val(get_val, ptr);
                            vl0 = get_val(get_val, ptr);
                            vl1 = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        Register: %d\n", typ);
                                fprintf(stderr, "                        Value 1: %d\n", vl0);
                                fprintf(stderr, "                        Value 2: %d\n", vl1);
                            }
                            
                            seq.asmdata += get_fstr("    addi $s%d, $0, %d\n", typ, vl0);
                            seq.asmdata += get_fstr("    andi $s%d, $s%d, %d\n", typ, typ, vl1);
                            continue;
                        }
                        case SEQD_SYSREG_OR: {
                            if (sgd_debug) fprintf(stderr, "                    Set register OR\n");
                            int typ, vl0, vl1;
                            typ = get_val(get_val, ptr);
                            vl0 = get_val(get_val, ptr);
                            vl1 = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        Register: %d\n", typ);
                                fprintf(stderr, "                        Value 1: %d\n", vl0);
                                fprintf(stderr, "                        Value 2: %d\n", vl1);
                            }
                            
                            seq.asmdata += get_fstr("    addi $s%d, $0, %d\n", typ, vl0);
                            seq.asmdata += get_fstr("    ori $s%d, $s%d, %d\n", typ, typ, vl1);
                            continue;
                        }
                        case SEQD_SYSREG_XOR: {
                            if (sgd_debug) fprintf(stderr, "                    Set register XOR\n");
                            int typ, vl0, vl1;
                            typ = get_val(get_val, ptr);
                            vl0 = get_val(get_val, ptr);
                            vl1 = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        Register: %d\n", typ);
                                fprintf(stderr, "                        Value 1: %d\n", vl0);
                                fprintf(stderr, "                        Value 2: %d\n", vl1);
                            }
                            
                            seq.asmdata += get_fstr("    addi $s%d, $0, %d\n", typ, vl0);
                            seq.asmdata += get_fstr("    xori $s%d, $s%d, %d\n", typ, typ, vl1);
                            continue;
                        }
                        //Misc instructions
                        case SEQD_WAIT: {
                            if (sgd_debug) fprintf(stderr, "                    Set WAIT\n");
                            int tim;
                            tim = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        Time: %d\n", tim);
                            }
                            
                            seq.asmdata += get_fstr("abstime_%06d:\n", (t_sz += tim));
                            continue;
                        }
                        case SEQD_JUMP: {
                            if (sgd_debug) fprintf(stderr, "                    Set JUMP\n");
                            int tim;
                            tim = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        Time: %d\n", tim);
                            }
                            
                            seq.asmdata += get_fstr("    j abstime_%06d\n", tim);
                            continue;
                        }
                        case SEQD_LOOPBEG: {
                            if (sgd_debug) fprintf(stderr, "                    Set LOOP START\n");
                            int cnt;
                            cnt = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        Count: %d\n", cnt);
                            }
                            //Too lazy, come back later
                            continue;
                        }
                        case SEQD_LOOPEND: {
                            if (sgd_debug) fprintf(stderr, "                    Set LOOP STOP\n");
                            //Too lazy, come back later
                            continue;
                        }
                        case SEQD_JUMPNEQ: {
                            if (sgd_debug) fprintf(stderr, "                    Set JUMP IF NOT EQUAL\n");
                            int vl0, vl1, tim;
                            vl0 = get_val(get_val, ptr);
                            vl1 = get_val(get_val, ptr);
                            tim = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        Value 1: %d\n", vl0);
                                fprintf(stderr, "                        Value 2: %d\n", vl1);
                                fprintf(stderr, "                        Time: %d\n", tim);
                            }
                            
                            seq.asmdata += get_fstr("    li $t3, %d\n", vl0);
                            seq.asmdata += get_fstr("    li $t4, %d\n", vl1);
                            seq.asmdata += get_fstr("    bne $t3, $t4, abstime_%06d\n", tim);
                            seq.asmdata += get_fstr("    nop\n");
                            continue;
                        }
                        case SEQD_JUMP3: {
                            if (sgd_debug) fprintf(stderr, "                    Set JUMP?\n");
                            int vl0, vl1, tim;
                            vl0 = get_val(get_val, ptr);
                            vl1 = get_val(get_val, ptr);
                            tim = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        Value 1: %d\n", vl0);
                                fprintf(stderr, "                        Value 2: %d\n", vl1);
                                fprintf(stderr, "                        Time: %d\n", tim);
                            }
                            
                            //Assumption based off of surrounding instructions
                            seq.asmdata += get_fstr("    li $t3, %d\n", vl0);
                            seq.asmdata += get_fstr("    li $t4, %d\n", vl1);
                            seq.asmdata += get_fstr("    beq $t3, $t4, abstime_%06d\n", tim);
                            seq.asmdata += get_fstr("    nop\n");
                            continue;
                        }
                        case SEQD_JUMPGEQ: {
                            if (sgd_debug) fprintf(stderr, "                    Set JUMP IF GREATER OR EQUAL\n");
                            int vl0, vl1, tim;
                            vl0 = get_val(get_val, ptr);
                            vl1 = get_val(get_val, ptr);
                            tim = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        Value 1: %d\n", vl0);
                                fprintf(stderr, "                        Value 2: %d\n", vl1);
                                fprintf(stderr, "                        Time: %d\n", tim);
                            }
                            
                            seq.asmdata += get_fstr("    li $t2, %d\n", vl0);
                            seq.asmdata += get_fstr("    li $t3, %d\n", vl1);
                            seq.asmdata += get_fstr("    slt $t4, $t2, $t3\n");
                            seq.asmdata += get_fstr("    beq $t4, $0, abstime_%06d\n", tim);
                            seq.asmdata += get_fstr("    nop\n");
                            continue;
                        }
                        case SEQD_JUMPLES: {
                            if (sgd_debug) fprintf(stderr, "                    Set JUMP IF LESSER\n");
                            int vl0, vl1, tim;
                            vl0 = get_val(get_val, ptr);
                            vl1 = get_val(get_val, ptr);
                            tim = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        Value 1: %d\n", vl0);
                                fprintf(stderr, "                        Value 2: %d\n", vl1);
                                fprintf(stderr, "                        Time: %d\n", tim);
                            }
                            
                            seq.asmdata += get_fstr("    li $t2, %d\n", vl0);
                            seq.asmdata += get_fstr("    li $t3, %d\n", vl1);
                            seq.asmdata += get_fstr("    slt $t4, $t2, $t3\n");
                            seq.asmdata += get_fstr("    bne $t4, $0, abstime_%06d\n", tim);
                            seq.asmdata += get_fstr("    nop\n");
                            continue;
                        }
                        case SEQD_CALLMKR: {
                            if (sgd_debug) fprintf(stderr, "                    Set marker\n");
                            int mkr;
                            mkr = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        Marker: %d\n", mkr);
                            }
                            
                            tmp.emplace_back(
                                t_sz,
                                META_SEQUENCER_EXCLUSIVE,
                                (unsigned char[]){SEQD_CALLMKR, mkr}
                            );
                            continue;
                        }
                        case SEQD_LOOPBREAK: {
                            if (sgd_debug) fprintf(stderr, "                    Set LOOP BREAK\n");
                            int vl0, vl1;
                            vl0 = get_val(get_val, ptr);
                            vl1 = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        Value 1: %d\n", vl0);
                                fprintf(stderr, "                        Value 2: %d\n", vl1);
                            }
                            //Too lazy, come back later
                            continue;
                        }
                        case SEQD_PRINT: {
                            if (sgd_debug) fprintf(stderr, "                    Set PRINT\n");
                            int val;
                            val = get_val(get_val, ptr);
                            if (sgd_debug) {
                                fprintf(stderr, "                        Value: %d\n", val);
                            }
                            
                            seq.asmdata += get_fstr("    li $a0, %d\n", val);
                            seq.asmdata += get_fstr("    li $v0, 11\n");
                            seq.asmdata += get_fstr("    syscall\n");
                            continue;
                        }
                        case SEQD_EOR: {
                            if (sgd_debug) fprintf(stderr, "                    Set end of sequence\n");
                            tmp.emplace_back(t_sz, META_END_OF_SEQUENCE);
                            break;
                        }
                        default: {
                            if (sgd_debug) fprintf(stderr, "                    Request 0x%02X\n", *(ptr - 1));
                            continue;
                        }
                    }
                    break;
                }
            }
            if (seq.fmt == SEQD_RAWMIDI) {
                unpackMesg((unsigned char*)seq.data.data(), seq.data.size());
                tmp.swap(mid_inf.msg[0]);
                
                //Replace PSX-style controllers with more common ones
                if (sgd_debug) fprintf(stderr, "                Replace PSX-style MIDI controls\n");
                for (auto &md : tmp) {
                    const auto &st = md.getStat();
                    if (st == STAT_CONTROLLER) {
                        const auto &dt = md.getData();
                        if (dt[0] == SEQD_CC_PSX_LOOP) {
                            if (dt[1] == SEQD_CC_PSX_LOOPSTART) {
                                if (sgd_debug) fprintf(stderr, "                Set loop start controller\n");
                                md = {
                                    md.getTime(), st,
                                    (unsigned char[]){CC_XML_LOOPSTART, CC_XML_LOOPINFINITE}
                                };
                            }
                            if (dt[1] == SEQD_CC_PSX_LOOPEND) {
                                if (sgd_debug) fprintf(stderr, "                Set loop stop controller\n");
                                md = {
                                    md.getTime(), st,
                                    (unsigned char[]){CC_XML_LOOPEND, CC_XML_LOOPRESERVED}
                                };
                            }
                        }

                        //Replaces psx event with sequencer specific event
                        if (dt[0] == SEQD_CC_SONGEVENT || dt[0] == SEQD_CC_UNKNOWN1) {
                            if (sgd_debug) fprintf(stderr, "                Set unknown controller 0x%02X\n", dt[0]);
                            md = {
                                md.getTime(),
                                META_SEQUENCER_EXCLUSIVE,
                                (unsigned char[]){dt[0], md.getChan(), dt[1]}
                            };
                        }
                    }
                }
            }
            seq.data = get_mid(tmp);
            
            //Clear empty asm if applicable
            if (!seq.asmdata.empty()) {
                std::string tmp0 = seq.asmdata, tmp1;
                bool is_diff = false;
                auto pos = std::string::npos;
                while ((pos = tmp0.find("\n")) != std::string::npos) {
                    tmp1 = tmp0.substr(0, pos); tmp0.erase(0, pos + 1);
                    if (tmp1.find("abstime_") == std::string::npos) { is_diff = true; break; }
                }
                if (!is_diff) seq.asmdata.clear();
            }
        }
    }
    
    if (sgd_debug) fprintf(stderr, "    Convert SEQD (Second Pass)\n");
    for (auto &grp : out.seqd) {
        if (grp.seq.empty()) continue;
        
        if (sgd_debug) fprintf(stderr, "        Current SEQD Group: %d\n", &grp - out.seqd.data());
        for (auto &seq : grp.seq) {
            if (seq.empty()) continue;
            if (seq.fmt != SEQD_REQUEST) continue;
            if (seq.data.empty()) continue;
            
            std::vector<mesginfo> tmp;
            mid_inf = {};
            
            if (sgd_debug) fprintf(stderr, "            Current SEQD: %d\n", &seq - grp.seq.data());
            unpackMesg((unsigned char*)seq.data.data(), seq.data.size());
            tmp.swap(mid_inf.msg[0]);
            
            //Replace sub sections
            for (int t = 0, ofs = 0; t < tmp.size(); ++t) {
                tmp[t].setTime(tmp[t].getTime() + ofs);
                
                const auto tm = tmp[t].getTime();
                const auto st = tmp[t].getStat();
                const auto dt = tmp[t].getData();
                
                if (st != META_SEQUENCER_EXCLUSIVE) continue;
                if (dt.size() != 3) continue;
                if (
                    (char)dt[0] != SEQD_SUB_START &&
                    (char)dt[0] != SEQD_SUB_STOP &&
                    (char)dt[0] != SEQD_SUB_STOPREL
                ) continue;
                t_sz = tm;
                
                if (sgd_debug) fprintf(stderr, "                Assign sub-sequence %d from group %d\n", dt[2], dt[1]);
                const auto &sub = sgd_inf.seqd.seqd[dt[1]].seq[dt[2]]; mid_inf = {};
                unpackMesg((unsigned char*)sub.data.data(), sub.data.size());
                for (auto &s : mid_inf.msg[0]) {
                    if (s.getStat() == META_END_OF_SEQUENCE) s.setStat(STAT_NONE);
                    else if (
                        s.getStat() == META_SEQUENCER_EXCLUSIVE &&
                        s.getData().size() > 1 && (
                            (char)dt[0] == SEQD_SUB_STOP ||
                            (char)dt[0] == SEQD_SUB_STOPREL
                        )
                    ) {
                        const auto tdt = s.getData();
                        if ((char)tdt[0] == SEQD_START) {
                            s = {
                                s.getTime(),
                                s.getStat(),
                                (unsigned char[]){
                                    ((char)dt[0] == SEQD_SUB_STOP) ? SEQD_STOP : SEQD_STOPREL,
                                    tdt[1], tdt[2], tdt[3],
                                    ((char)dt[0] == SEQD_SUB_STOP) ? 0 : 127
                                }
                            };
                        }
                        else if ((char)tdt[0] == SEQD_STOP || (char)tdt[0] == SEQD_STOPREL) {
                            s.setStat(STAT_NONE);
                        }
                    }
                    s.setTime(t_sz + s.getTime());
                }
                ofs += mid_inf.msg[0].back().getTime();
                std::erase_if(
                    mid_inf.msg[0],
                    [](const mesginfo &m) { return m.getStat() == STAT_NONE; }
                );
                
                tmp[t] = mid_inf.msg[0][0];
                tmp.insert(tmp.begin() + t + 1, mid_inf.msg[0].begin() + 1, mid_inf.msg[0].end());
            }
            
            //Assign end of track event and sort
            t_sz = (*std::max_element(tmp.begin(), tmp.end())).getTime() + 8000;
            if (tmp.back().getStat() != META_END_OF_SEQUENCE) tmp.emplace_back(t_sz, META_END_OF_SEQUENCE);
            else tmp.back().setTime(t_sz);
            std::sort(tmp.begin(), tmp.end());
            seq.data = get_mid(tmp);
        }
    }
}

///Packs specified sequence into MIDI data
std::vector<unsigned char> seqdToMidi(const int &grp, const int &seq) {
    if (sgd_debug) fprintf(stderr, "    Extract sequence\n");

    if (
        sgd_inf.seqd.empty() ||
        grp < 0 || grp >= sgd_inf.seqd.seqd.size() ||
        seq < 0 || seq >= sgd_inf.seqd.seqd[grp].seq.size()
    ) return {};

    const auto &sq = sgd_inf.seqd.seqd[grp].seq[seq];
    midiinfo out;
    
    auto get_fstr = []<typename... T>(const char *in, T&&... args) -> std::string {
        std::string out; int s0 = snprintf(nullptr, 0, in, args...) + 1;
        out.resize(s0 - 1); snprintf(out.data(), s0, in, args...);
        return out;
    };

    if (sq.fmt != SEQD_REQUEST && sq.fmt != SEQD_RAWMIDI) {
        if (sgd_debug) fprintf(stderr, "        Unknown sequence type %d\n", sq.fmt);
        return sq.data;
    }
    if (sq.data.size() < 6) return {};

    if (sgd_debug) fprintf(stderr, "        Set MIDI header\n");
    out = {MIDI_SINGLE_TRACK, 1, sq.div};

    if (sgd_debug) fprintf(stderr, "        Set MIDI sequences\n");
    mid_inf = {};
    unpackMesg((unsigned char*)sq.data.data(), sq.data.size());
    
    if (sq.fmt == SEQD_REQUEST) {
        struct seqd_vals {
            int v[2] {};
            int& operator[](const int &i) { return v[i]; }
            const int& operator[](const int &i) const { return v[i]; }
        };
        std::map<int, seqd_vals> seq_chns;
        
        out.msg.emplace_back();
        //out.msg[0].emplace_back(0, META_TIME_SIGNATURE, (unsigned char[]){1, 5, 24, 8});
        //out.msg[0].emplace_back(0, META_TEMPO, (unsigned char[]){0x03, 0x97, 0x1E});
        //out.msg[0].emplace_back(0, META_SMPTE, (unsigned char[]){0x60, 0x00, 0x00, 0x00, 0x00, 0x00});
        for (const auto &m : mid_inf.msg[0]) {
            const auto tm = m.getTime();
            const auto st = m.getStat();
            const auto dt = m.getData();
            
            if (
                st != META_SEQUENCER_EXCLUSIVE || dt.empty() || (
                    (char)dt[0] != SEQD_START &&
                    (char)dt[0] != SEQD_STOP &&
                    (char)dt[0] != SEQD_STOPREL
                )
            ) { out.msg[0].emplace_back(m); continue; }
            
            if (dt.size() == 1) {
                int cc;
                cc = ((char)dt[0] == SEQD_STOPREL) ? CC_RESET_NOTES : CC_RESET_SOUND;
                out.msg[0].emplace_back(tm, STAT_CONTROLLER, (unsigned char[]){cc, 0});
            }
            else {
                short stt, bnk, prs, nte, vel;
                stt = ((char)dt[0] != SEQD_START) ? STAT_NOTE_OFF : STAT_NOTE_ON;
                bnk = dt[1];
                prs = dt[2];
                nte = dt[3];
                vel = dt[4];
                
                if ((char)dt[0] == SEQD_START) {
                    //out.msg[0].emplace_back(tm, META_MARKER, get_fstr("Bank %d, Preset %d, Note %d", bnk, prs, nte).c_str());
                }
                if (seq_chns.find(prs) == seq_chns.end() || seq_chns[prs][0] != bnk) {
                    seq_chns[prs] = {bnk, -1};
                    out.msg[0].emplace_back(tm, STAT_CONTROLLER | prs, (unsigned char[]){CC_BANK_SELECT_C, bnk});
                }
                if (seq_chns[prs][1] != prs) {
                    seq_chns[prs][1] = prs;
                    out.msg[0].emplace_back(tm, STAT_PROGRAMME_CHANGE | prs, (unsigned char[]){prs});
                }
                out.msg[0].emplace_back(tm, stt | prs, (unsigned char[]){nte, vel});
            }
        }
    }
    if (sq.fmt == SEQD_RAWMIDI) out.msg.swap(mid_inf.msg);
    
    if (!sq.name.empty()) {
        if (sgd_debug) fprintf(stderr, "        Set MIDI title\n");
        out.msg[0].insert(out.msg[0].begin(), {0, META_TRACK_NAME, sq.name.c_str()});
    }
    mid_inf = std::move(out);

    return std::move(packMidi());
}

///Extracts variable sequence definitions into string
std::string extractSeqd() {
    if (sgd_debug) fprintf(stderr, "    Extract SEQD info\n");

    std::string out;
    auto set_fstr = [&out]<typename... T>(const char *in, T&&... args) -> void {
        int s0 = snprintf(nullptr, 0, in, args...) + 1, s1 = out.size();
        out.resize(s1 + s0 - 1); snprintf(out.data() + s1, s0, in, args...);
    };

    set_fstr("Global Flags: %s\n", std::bitset<32>(sgd_inf.seqd.flag).to_string().c_str());
    set_fstr("Sequences:\n");
    for (const auto &g : sgd_inf.seqd.seqd) {
        if (g.empty()) continue;
        set_fstr("    Sequence Group: %d\n", &g - sgd_inf.seqd.seqd.data());
        set_fstr("        Group Flags: %s\n", std::bitset<32>(g.flag).to_string().c_str());
        for (const auto &s : g.seq) {
            if (s.empty()) continue;
            set_fstr("        Sequence: %d\n", &s - g.seq.data());
            set_fstr("            Sequence Flags: %s\n", std::bitset<32>(s.flag).to_string().c_str());
            set_fstr("            Name: %s\n", (!s.name.empty()) ? s.name.c_str() : "(none)");
            set_fstr("            Format: %s\n", (s.fmt == SEQD_REQUEST) ? "REQUEST" :
                                                 (s.fmt == SEQD_RAWMIDI) ? "MIDI" : "UNKNOWN");
            set_fstr("            Division: ");
            if (s.div < 0) {
                set_fstr(
                    "%d SMPTE Frames per Second, %d Ticks per SMPTE Frame\n",
                    (s.div >> 8) & 0x7F,
                    (s.div >> 0) & 0xFF
                );
            }
            else set_fstr("%d Pulses per Quarternote\n", s.div & 0x7FFF);
            set_fstr("            Left Volume: %d\n", s.volleft);
            set_fstr("            Right Volume: %d\n", s.volright);
        }
    }

    return out;
}
