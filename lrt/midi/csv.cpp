#include <string>
#include <format>
#include <vector>
#include "midi_const.hpp"
#include "midi_types.hpp"
#include "midi_func.hpp"


///Packs variable messages from MIDI chunk to MIDICSV-style string
std::string packCsv() {
    std::string csv = "";
    auto set_fstr = [&csv]<typename... T>(std::format_string<T...> in, T&&... args) -> void {
        csv += std::format(in, std::forward<T>(args)...);
    };

    set_fstr("0, 0, HEADER, {0:d}, {1:d}, {2:d}\n", mid_inf.fmt, mid_inf.trk, mid_inf.div);
    for (const auto &msg : mid_inf.msg) {
        unsigned id = 1 + (&msg - mid_inf.msg.data());

        set_fstr("{0:d}, 0, START_TRACK\n", id);
        for (const auto &m : msg) {
            std::string txt = "";
            const auto &tm = m.getTime();
            const auto &st = m.getStat();
            const auto &ch = m.getChan();
            const auto &dt = m.getData();

            switch(st) {
                case META_SEQUENCE_ID:          txt = "SEQUENCE_NUMBER"; break;
                case META_TEXT:                 txt = "TEXT_T"; break;
                case META_COPYRIGHT:            txt = "COPYRIGHT_T"; break;
                case META_TRACK_NAME:           txt = "TITLE_T"; break;
                case META_INSTRUMENT_NAME:      txt = "INSTRUMENT_NAME_T"; break;
                case META_LYRICS:               txt = "LYRIC_T"; break;
                case META_MARKER:               txt = "MARKER_T"; break;
                case META_CUE:                  txt = "CUE_POINT_T"; break;
                case META_PATCH_NAME:           txt = "INSTRUMENT_NAME_T"; break;
                case META_PORT_NAME:
                case META_MISC_TEXT_A:
                case META_MISC_TEXT_B:
                case META_MISC_TEXT_C:
                case META_MISC_TEXT_D:
                case META_MISC_TEXT_E:
                case META_MISC_TEXT_F:          txt = "TEXT_T"; break;
                case META_CHANNEL_PREFIX:       txt = "CHANNEL_PREFIX"; break;
                case META_PORT:                 txt = "MIDI_PORT"; break;
                case META_END_OF_SEQUENCE:      txt = "END_TRACK"; break;
                case META_TEMPO:                txt = "TEMPO"; break;
                case META_SMPTE:                txt = "SMPTE_OFFSET"; break;
                case META_TIME_SIGNATURE:       txt = "TIME_SIGNATURE"; break;
                case META_KEY_SIGNATURE:        txt = "KEY_SIGNATURE"; break;
                case META_SEQUENCER_EXCLUSIVE:  txt = "SEQUENCER_SPECIFIC"; break;
                case META_NONE:
                case STAT_NONE:                 continue;
                case STAT_NOTE_OFF:             txt = "NOTE_OFF_C"; break;
                case STAT_NOTE_ON:              txt = "NOTE_ON_C"; break;
                case STAT_KEY_PRESSURE:         txt = "POLY_AFTERTOUCH_C"; break;
                case STAT_CONTROLLER:           txt = "CONTROL_C"; break;
                case STAT_PROGRAMME_CHANGE:     txt = "PROGRAM_C"; break;
                case STAT_CHANNEL_PRESSURE:     txt = "CHANNEL_AFTERTOUCH_C"; break;
                case STAT_PITCH_WHEEL:          txt = "PITCH_BEND_C"; break;
                case STAT_SYSTEM_EXCLUSIVE_STOP:txt = "_PACKET";
                case STAT_SYSTEM_EXCLUSIVE:     txt = "SYSTEM_EXCLUSIVE" + txt + ", "; break;
                default:                        txt = "UNKNOWN_META_EVENT"; break;
            }
            
            set_fstr("{0:d}, {1:d}, {2:s}, ", id, tm, txt);
            if (txt == "UNKNOWN_META_EVENT")
                set_fstr("{0:d}, ", (st < 0) ? -st : st & 0xFF);
            if (txt.find("UNKNOWN") != std::string::npos)
                set_fstr("{0:d}, ", dt.size());
            else if (txt.find("EXCLUSIVE") != std::string::npos)
                set_fstr("{0:d}, ", dt.size());
            else if (txt.find("SPECIFIC") != std::string::npos)
                set_fstr("{0:d}, ", dt.size());
            if (txt.ends_with("_C"))
                set_fstr("{0:d}, ", ch);
            if (txt == "END_TRACK")
                csv.replace(csv.rfind(", "), 2, "\n");
            else if (txt == "SEQUENCE_NUMBER")
                set_fstr("{0:d}\n", (unsigned short)dt[0] << 8 | dt[1]);
            else if (txt == "PITCH_BEND_C")
                set_fstr("{0:d}\n", (unsigned short)(dt[1] & 0x7F) << 7 | dt[0] & 0x7F);
            else if (txt == "TEMPO")
                set_fstr("{0:d}\n", (unsigned)dt[0] << 16 | (unsigned)dt[1] << 8 | dt[2] << 0);
            else if (txt == "KEY_SIGNATURE")
                set_fstr("{0:d}, \"{1:s}\"\n", (char)dt[0], !dt[1] ? "major" : "minor");
            else if (txt.ends_with("_T"))
                set_fstr("\"{0:s}\"\n", std::string(dt.begin(), dt.end()));
            else
                for (const auto &d : dt) set_fstr("{0:d}{1:s}", d, (&d < &dt.back()) ? ", " : "\n");
        }
    }
    csv += "0, 0, END_OF_FILE\n";

    return csv;
}
