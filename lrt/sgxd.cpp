#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>
#include "sgxd_forms.hpp"
#include "sgxd_const.hpp"
#include "sgxd_types.hpp"
#include "sgxd_func.hpp"
#include "directory.hpp"


///Unpacks SGXD info from SGXD file(s)
void unpackSgxd(const char *file0, const char *file1) {
    if (!file0 || !file0[0]) return;

    std::vector<unsigned char> tmp;

    if (true) {
        unsigned char *data = 0;
        unsigned size = 0;

        if (!getFileData(file0, data, size)) {
            fprintf(stderr, "Unable to open %s\n", file0);
            sgd_inf = {}; return;
        }
        tmp.insert(tmp.end(), data, data + size);
    }

    if (file1 && file0[0]) {
        unsigned char *data = 0;
        unsigned size = 0;

        if (!getFileData(file1, data, size)) {
            fprintf(stderr, "Unable to open %s\n", file1);
            sgd_inf = {}; return;
        }
        tmp.insert(tmp.end(), data, data + size);
    }

    unpackSgxd(tmp.data(), tmp.size());
}

///Unpacks SGXD info from SGXD data
void unpackSgxd(unsigned char *in, const unsigned length) {
    sgd_inf = {};
    if (!in || length < 16) return;

    sgd_beg = in;
    const unsigned char *in_end = sgd_beg + length;
    unsigned n_add, s_add, s_siz; bool s_flg;

    auto get_fcc = [&in, &in_end]() -> unsigned {
        unsigned out = 0;
        for (int t = 0; t < 4; ++t) {
            if (in >= in_end) break;
            out = (out << 8) | *(in++);
        }
        return out;
    };
    auto get_int = [&in, &in_end]() -> unsigned {
        unsigned out = 0;
        for (int i = 0; i < 4; ++i) {
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

    //Check if SGXD
    if (get_fcc() != FOURCC_SGXD) {
        fprintf(stderr, "This is not an SGXD file\n");
        sgd_beg = 0; return;
    }

    //Set name address, stream address, stream size
    n_add = get_int();
    s_add = get_int();
    s_siz = get_int();
    s_flg = s_siz & 0x80000000;
    s_siz = s_siz & 0x7FFFFFFF;

    if (s_add > length) s_add = length;
    if (s_add + s_siz > length) s_siz = length - s_add;

    //Set file name
    if (sgd_beg + n_add >= sgd_beg + s_add) {
        fprintf(stderr, "File name located outside header chunk\n");
        sgd_beg = 0; return;
    }
    else sgd_inf.file = get_str(sgd_beg, n_add);

    //Set stream chunk
    sgd_dat_beg = sgd_beg + s_add;
    sgd_dat_end = sgd_dat_beg + s_siz;

    if (sgd_debug) {
        fprintf(stderr, "    Stream Name: %s\n", sgd_inf.file.c_str());
        fprintf(stderr, "    Stream Address: 0x%08X\n", s_add);
        fprintf(stderr, "    Stream Size: %d\n", s_siz);
        fprintf(stderr, "    Stream Flag: %s\n", s_flg ? "TRUE" : "FALSE");
    }

    //Get misc chunks
    while (in < in_end) {
        unsigned t_fc, t_sz;

        t_fc = get_fcc();
        t_sz = get_int();
        if (in + t_sz > in_end) break;

        switch(t_fc) {
#ifdef UNPACKBUSS_IMPLEMENTATION
            case SGXD_BUSS:
                unpackBuss(in, t_sz);
                break;
#endif
#ifdef UNPACKRGND_IMPLEMENTATION
            case SGXD_RGND:
                unpackRgnd(in, t_sz);
                break;
#endif
#ifdef UNPACKSEQD_IMPLEMENTATION
            case SGXD_SEQD:
                unpackSeqd(in, t_sz);
                break;
#endif
#ifdef UNPACKWAVE_IMPLEMENTATION
            case SGXD_WAVE:
                unpackWave(in, t_sz);
                break;
#endif
#ifdef UNPACKWSUR_IMPLEMENTATION
            case SGXD_WSUR:
                unpackWsur(in, t_sz);
                break;
#endif
#ifdef UNPACKWMKR_IMPLEMENTATION
            case SGXD_WMKR:
            case SGXD_WMRK:
                unpackWmkr(in, t_sz);
                break;
#endif
#ifdef UNPACKCONF_IMPLEMENTATION
            case SGXD_CONF:
                unpackConf(in, t_sz);
                break;
#endif
#ifdef UNPACKTUNE_IMPLEMENTATION
            case SGXD_TUNE:
                unpackTune(in, t_sz);
                break;
#endif
#ifdef UNPACKADSR_IMPLEMENTATION
            case SGXD_ADSR:
            case SGXD_ASDR:
                unpackAdsr(in, t_sz);
                break;
#endif
#ifdef UNPACKNAME_IMPLEMENTATION
            case SGXD_NAME:
                unpackName(in, t_sz);
                break;
#endif
            default:
                break;
        }

        in += t_sz;
    }

    sgd_beg = 0; sgd_dat_beg = 0; sgd_dat_end = 0;
}


///Extracts misc data from SGXD info
void extractSgxd(const char *folder) {
    if (sgd_inf.empty() || !folder || !folder[0]) return;

    std::string out = "", tmp;
    out += folder;
    out += "@" + sgd_inf.file;

    fprintf(stdout, "Extract SGXD contents to %s/\n", out.c_str());
    if (!createFolder((out + "/").c_str())) {
        fprintf(stderr, "Unable to create %s/\n", out.c_str());
        return;
    }

#ifdef UNPACKBUSS_IMPLEMENTATION
    while (!sgd_inf.buss.empty()) {
        tmp = "/buss";
        fprintf(stdout, "    Extract BUSS contents to %s/\n", tmp.c_str());
        if (!createFolder((out + tmp + "/").c_str())) {
            fprintf(stderr, "    Unable to create %s/\n", tmp.c_str());
            break;
        }

        for (int b = 0; b < sgd_inf.buss.buss.size(); ++b) {
            const auto &bus = sgd_inf.buss.buss[b];

            std::string tmp1, tmp2, tmp3 = "", tmp4;
            tmp1 = "/" + ((!bus.name.empty()) ? bus.name : "setup_" + std::to_string(b));
            fprintf(stdout, "        Extract SETUP contents to %s/\n", tmp1.c_str());
            if (!createFolder((out + tmp + tmp1 + "/").c_str())) {
                fprintf(stderr, "        Unable to create %s/\n", tmp1.c_str());
                continue;
            }

            fprintf(stdout, "            Extract UNIT contents to /units.txt\n");
            for (const auto &u : bus.unit) {
                tmp3 += ((!u.name.empty()) ? u.name : "(none)") + "\n";
            }
            if (tmp3.empty()) tmp3 = "(none)";

            if (createFile((out + tmp + tmp1 + "/units.txt").c_str(), tmp3.data(), tmp3.size())) {
                fprintf(stdout, "                Extracted /units.txt\n");
            }
            else fprintf(stderr, "                Unable to extract /units.txt\n");

            fprintf(stdout, "            Extract EFFECT contents to %s/\n", tmp1.c_str());
            std::vector<std::string> mods {""};
            for (int e = 0; e < bus.effect.size(); ++e) {
                const auto &efx = bus.effect[e];

                auto id = std::find(mods.begin(), mods.end(), efx.module);
                if (id == mods.end()) { mods.push_back(efx.module); id = mods.end() - 1; }

                tmp2 = "";
                if (id != mods.begin()) {
                    tmp2 = "/" + efx.module;
                    if (!createFolder((out + tmp + tmp1 + tmp2 + "/").c_str())) {
                        fprintf(stderr, "                Unable to create %s/\n", tmp2.c_str());
                        continue;
                    }
                }

                tmp3 = (!efx.name.empty()) ? efx.name : "effect_" + std::to_string(e);
                tmp3 = "/" + tmp3 + ".txt";
                tmp4 = (!efx.preset.empty()) ? efx.preset : "(none)";

                if (createFile((out + tmp + tmp1 + tmp2 + tmp3).c_str(), tmp4.data(), tmp4.size())) {
                    fprintf(stdout, "                Extracted %s\n", (tmp2 + tmp3).c_str());
                }
                else fprintf(stderr, "                Unable to extract %s\n", (tmp2 + tmp3).c_str());
            }
        }

        if (sgd_text) {
            auto bus = extractBuss();
            if (createFile((out + "/buss.txt").c_str(), bus.data(), bus.size())) {
                fprintf(stdout, "        Extracted /buss.txt\n");
            }
        }

        break;
    }
#endif

#ifdef UNPACKRGND_IMPLEMENTATION
    while (!sgd_inf.rgnd.empty()) {
        tmp = "/rgnd";
        fprintf(stdout, "    Extract RGND contents to %s/\n", tmp.c_str());

        if (!createFolder((out + tmp + "/").c_str())) {
            fprintf(stderr, "    Unable to create %s/\n", tmp.c_str());
            break;
        }

        std::string nam = "/" + sgd_inf.file + ".sf2";
        const auto sf2 = rgndToSfbk();
        if (!sf2.empty() && createFile((out + tmp + nam).c_str(), (unsigned char*)sf2.data(), sf2.size())) {
            fprintf(stdout, "        Extracted %s\n", nam.c_str());
        }
        else fprintf(stderr, "        Unable to extract %s\n", nam.c_str());

        if (sgd_text) {
            const auto rgn = extractRgnd();
            if (createFile((out + "/rgnd.txt").c_str(), rgn.data(), rgn.size())) {
                fprintf(stdout, "        Extracted /rgnd.txt\n");
            }
        }

        break;
    }
#endif

#ifdef UNPACKSEQD_IMPLEMENTATION
    while (!sgd_inf.seqd.empty()) {
        tmp = "/seqd";
        fprintf(stdout, "    Extract SEQD contents to %s/\n", tmp.c_str());

        if (!createFolder((out + tmp + "/").c_str())) {
            fprintf(stderr, "    Unable to create %s/\n", tmp.c_str());
            break;
        }

        for (int g = 0; g < sgd_inf.seqd.seqd.size(); ++g) {
            if (sgd_inf.seqd.seqd[g].seq.empty()) continue;
            for (int s = 0; s < sgd_inf.seqd.seqd[g].seq.size(); ++s) {
                if (sgd_inf.seqd.seqd[g].seq[s].empty()) continue;

                const auto &sq = sgd_inf.seqd.seqd[g].seq[s];
                const auto seq = seqdToMidi(g, s);
                std::string nam, ext;

                if (!sq.name.empty()) nam = sq.name;
                else {
                    nam.resize(snprintf(nullptr, 0, "seq_%03d_%03d", g, s));
                    snprintf(nam.data(), nam.size() + 1, "seq_%03d_%03d", g, s);
                }
                ext = (sq.fmt > SEQD_RAWMIDI) ? ".unk" : ".mid";
                nam = "/" + nam + ext;

                if (!seq.empty() && createFile((out + tmp + nam).c_str(), (unsigned char*)seq.data(), seq.size())) {
                    fprintf(stdout, "        Extracted %s\n", nam.c_str());
                }
                else fprintf(stderr, "        Unable to extract %s\n", nam.c_str());
                
                if (!sgd_req.empty() && createFile((out + tmp + nam + ".req").c_str(), sgd_req.c_str(), sgd_req.size())) {
                    fprintf(stdout, "        Extracted %s.req\n", nam.c_str());
                }
            }
        }

        if (sgd_text) {
            const auto seq = extractSeqd();
            if (createFile((out + "/seqd.txt").c_str(), seq.data(), seq.size())) {
                fprintf(stdout, "        Extracted /seqd.txt\n");
            }
        }

        break;
    }
#endif

#ifdef UNPACKWAVE_IMPLEMENTATION
    while (!sgd_inf.wave.empty()) {
        tmp = "/wave";
        fprintf(stdout, "    Extract WAVE contents to %s/\n", tmp.c_str());

        if (!createFolder((out + tmp + "/").c_str())) {
            fprintf(stderr, "    Unable to create %s/\n", tmp.c_str());
            break;
        }

        for (int w = 0; w < sgd_inf.wave.wave.size(); ++w) {
            const auto wav = waveToWave(w);
            std::string nam;

            if (!sgd_inf.wave.wave[w].name.empty()) nam = sgd_inf.wave.wave[w].name;
            else {
                nam.resize(snprintf(nullptr, 0, "smpl_%03d", w));
                snprintf(nam.data(), nam.size() + 1, "smpl_%03d", w);
            }
            nam = "/" + nam + ".wav";

            if (!wav.empty() && createFile((out + tmp + nam).c_str(), (unsigned char*)wav.data(), wav.size())) {
                fprintf(stdout, "        Extracted %s\n", nam.c_str());
            }
            else fprintf(stderr, "        Unable to extract %s\n", nam.c_str());
        }

        if (sgd_text) {
            const auto wav = extractWave();
            if (createFile((out + "/wave.txt").c_str(), wav.data(), wav.size())) {
                fprintf(stdout, "        Extracted /wave.txt\n");
            }
        }

        break;
    }
#endif

#ifdef UNPACKWSUR_IMPLEMENTATION
    if (!sgd_inf.wsur.empty() && sgd_text) {
        fprintf(stdout, "    Extract WSUR contents to %s/\n", out.c_str());
        const auto sur = extractWsur();
        if (createFile((out + "/wsur.txt").c_str(), sur.data(), sur.size())) {
            fprintf(stdout, "        Extracted /wsur.txt\n");
        }
    }
#endif

#ifdef UNPACKWMKR_IMPLEMENTATION
    if (!sgd_inf.wmkr.empty() && sgd_text) {
        fprintf(stdout, "    Extract WMKR contents to %s/\n", out.c_str());
        const auto mkr = extractWmkr();
        if (createFile((out + "/wmkr.txt").c_str(), mkr.data(), mkr.size())) {
            fprintf(stdout, "        Extracted /wmkr.txt\n");
        }
    }
#endif

#ifdef UNPACKCONF_IMPLEMENTATION
    while (!sgd_inf.conf.empty()) {
        tmp = "/conf";
        fprintf(stdout, "    Extract CONF contents to %s/\n", tmp.c_str());

        if (!createFolder((out + tmp + "/").c_str())) {
            fprintf(stderr, "    Unable to create %s/\n", tmp.c_str());
            break;
        }

        for (int c = 0; c < sgd_inf.conf.conf.size(); ++c) {
            const auto &cnf = sgd_inf.conf.conf[c];
            std::string nam;

            if (!cnf.name.empty()) nam = cnf.name;
            else {
                nam.resize(snprintf(nullptr, 0, "conf_%03d", c));
                snprintf(nam.data(), nam.size() + 1, "conf_%03d", c);
            }
            nam = "/" + nam + ".txt";

            if (!cnf.text.empty() && createFile((out + tmp + nam).c_str(), cnf.text.data(), cnf.text.size())) {
                fprintf(stdout, "        Extracted %s\n", nam.c_str());
            }
            else fprintf(stderr, "        Unable to extract %s\n", nam.c_str());
        }

        if (sgd_text) {
            const auto cnf = extractConf();
            if (createFile((out + "/conf.txt").c_str(), cnf.data(), cnf.size())) {
                fprintf(stdout, "        Extracted /conf.txt\n");
            }
        }

        break;
    }
#endif

#ifdef UNPACKTUNE_IMPLEMENTATION
    if (!sgd_inf.tune.empty() && sgd_text) {
        fprintf(stdout, "    Extract TUNE contents to %s/\n", out.c_str());
        const auto tun = extractTune();
        if (createFile((out + "/tune.txt").c_str(), tun.data(), tun.size())) {
            fprintf(stdout, "        Extracted /tune.txt\n");
        }
    }
#endif

#ifdef UNPACKADSR_IMPLEMENTATION
    if (!sgd_inf.adsr.empty() && sgd_text) {
        fprintf(stdout, "    Extract ADSR contents to %s/\n", out.c_str());
        const auto env = extractAdsr();
        if (createFile((out + "/adsr.txt").c_str(), env.data(), env.size())) {
            fprintf(stdout, "        Extracted /adsr.txt\n");
        }
    }
#endif

#ifdef UNPACKNAME_IMPLEMENTATION
    if (!sgd_inf.name.empty() && sgd_text) {
        fprintf(stdout, "    Extract NAME contents to %s/\n", out.c_str());
        const auto nam = extractName();
        if (createFile((out + "/name.txt").c_str(), nam.data(), nam.size())) {
            fprintf(stdout, "        Extracted /name.txt\n");
        }
    }
#endif

    fprintf(stdout, "End extraction\n");
}
