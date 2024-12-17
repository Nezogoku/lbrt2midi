#include <bitset>
#include <cstdio>
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
}

///Packs specified sequence into MIDI data
std::vector<unsigned char> seqdToMidi(const int &grp, const int &seq) {
    if (sgd_debug) fprintf(stderr, "    Extract sequence\n");

    mid_inf = {};
    sgd_req = "";
    if (
        sgd_inf.seqd.empty() ||
        grp < 0 || grp >= sgd_inf.seqd.seqd.size() ||
        seq < 0 || seq >= sgd_inf.seqd.seqd[grp].seq.size()
    ) return {};

    const auto &sq = sgd_inf.seqd.seqd[grp].seq[seq];

    if (sq.fmt != SEQD_REQUEST && sq.fmt != SEQD_RAWMIDI) {
        if (sgd_debug) fprintf(stderr, "        Unknown sequence type %d\n", sq.fmt);
        return {};
    }

    if (sgd_debug) fprintf(stderr, "        Set MIDI header\n");
    mid_inf = {MIDI_SINGLE_TRACK, 1, sq.div};

    if (sgd_debug) fprintf(stderr, "        Set MIDI sequences\n");
    if (sq.fmt == SEQD_REQUEST) {
        //Will get to this later...
        return {};

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
                int b0, b1, b2;
                b0 = self(self, in);
                b1 = self(self, in);
                b2 = self(self, in);
                out = b0 + (rand() % (b1 - b2 + 1) + b2);
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

        struct req { unsigned char typ; std::vector<int> dat; };
        std::vector<req> seq_req;

        //https://github.com/Nenkai/010GameTemplates/blob/main/Sony/SGXD.bt
        for (auto ptr = sq.data.begin(); ptr < sq.data.end();) {
            while (*ptr < 0xA0) ptr += 1;

            seq_req.emplace_back();
            auto &rq = seq_req.back();
            switch(rq.typ = *ptr++) {
                case SEQD_SUB_START:
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    break;
                case SEQD_SUB_STOP:
                case SEQD_SUB_STOPREL:
                    rq.dat.push_back(get_val(get_val, ptr));
                    break;
                case SEQD_SUB_GETSTAT:
                    rq.dat.push_back(get_val(get_val, ptr));
                    break;
                case SEQD_CONTROL:
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    break;
                case SEQD_ADSR:
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    break;
                case SEQD_BEND:
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr, 12));
                    rq.dat.push_back(get_val(get_val, ptr, 12));
                    rq.dat.push_back(get_val(get_val, ptr));
                    break;
                case SEQD_ADSR_DIRECT:
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    break;
                case SEQD_START:
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr, 4096));
                    break;
                case SEQD_STOP:
                case SEQD_STOPREL:
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    break;
                case SEQD_GETPORTSTAT:
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    break;
                case SEQD_STARTSMPL:
                case SEQD_STARTNOISE:
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr, 4096));
                    rq.dat.push_back(get_val(get_val, ptr, 128));
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    break;
                case SEQD_SYSREG_INIT:
                case SEQD_SYSREG_ADD:
                case SEQD_SYSREG_MINUS:
                case SEQD_SYSREG_MULT:
                case SEQD_SYSREG_DIVI:
                case SEQD_SYSREG_MODU:
                case SEQD_SYSREG_AND:
                case SEQD_SYSREG_OR:
                case SEQD_SYSREG_XOR:
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    break;
                case SEQD_WAIT:
                    rq.dat.push_back(get_val(get_val, ptr));
                    break;
                case SEQD_JUMP:
                    rq.dat.push_back(get_val(get_val, ptr));
                    break;
                case SEQD_LOOPBEG:
                    rq.dat.push_back(get_val(get_val, ptr));
                case SEQD_LOOPEND:
                    break;
                case SEQD_JUMPNEQ:
                case SEQD_JUMP3:
                case SEQD_JUMPGEQ:
                case SEQD_JUMPLES:
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    break;
                case SEQD_CALLMKR:
                    rq.dat.push_back(get_val(get_val, ptr));
                    break;
                case SEQD_LOOPBREAK:
                    rq.dat.push_back(get_val(get_val, ptr));
                    rq.dat.push_back(get_val(get_val, ptr));
                    break;
                case SEQD_PRINT:
                    rq.dat.push_back(get_val(get_val, ptr));
                    break;
                case SEQD_EOR:
                default:
                    break;
            }
        }
    }
    else {
        unpackMesg((unsigned char*)sq.data.data(), sq.data.size());

        //Replace PSX-style controllers with more common ones
        if (sgd_debug) fprintf(stderr, "        Replace PSX-style MIDI controls\n");
        for (auto &md : mid_inf.msg[0]) {
            const auto &st = md.getStat();
            if (st == STAT_CONTROLLER) {
                const auto &dt = md.getData();
                if (dt[0] == SEQD_CC_PSX_LOOP) {

                    if (dt[1] == SEQD_CC_PSX_LOOPSTART) {
                        md = {
                            md.getTime(), st,
                            (unsigned char[]){CC_XML_LOOPSTART, CC_XML_LOOPINFINITE}
                        };
                    }
                    if (dt[1] == SEQD_CC_PSX_LOOPEND) {
                        md = {
                            md.getTime(), st,
                            (unsigned char[]){CC_XML_LOOPEND, CC_XML_LOOPRESERVED}
                        };
                    }
                }

                //Replaces psx event with text
                if (dt[0] == SEQD_CC_SONGEVENT) {
                    md = {
                        md.getTime(), META_CUE,
                        ("SongEvent_" + std::to_string(dt[1])).c_str()
                    };
                }
            }
        }
        return packMidi();
    }

    if (!sq.name.empty()) {
        if (sgd_debug) fprintf(stderr, "        Set MIDI title\n");
        mid_inf.msg[0].insert(
            mid_inf.msg[0].begin(),
            {0, META_TRACK_NAME, sq.name.c_str()}
        );
    }

    return packMidi();
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
            set_fstr("            Sequence Size: %d\n", s.data.size());
        }
    }

    return out;
}
