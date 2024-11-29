#include <cstdio>
#include <string>
#include "tsf/minisdl_audio.h"
#define TSF_IMPLEMENTATION
#define TML_IMPLEMENTATION
#include "tsf/tsf.h"
#include "tsf/tml.h"
#include "playmidi_types.hpp"
#include "playmidi_func.hpp"

static tsf *g_TinySoundFont = NULL;                         // Pointer to Soundfont
static tml_message *g_MidiMessage = NULL;                   // Pointer to Midi playback state
static double g_Msec = 0.0;                                 // Pointer to Total playback time

///Callback function called by audio thread
static void AudioCallback(void *data, unsigned char *stream, int len) {
	//Number of samples to process
	int SampleBlock,
		SampleCount = (len / (2 * sizeof(short))); //2 output channels

	for (SampleBlock = TSF_RENDER_EFFECTSAMPLEBLOCK; SampleCount; SampleCount -= SampleBlock, stream += (SampleBlock * (2 * sizeof(short)))) {
		//Process MIDI playback, then process TSF_RENDER_EFFECTSAMPLEBLOCK samples at once
		if (SampleBlock > SampleCount) SampleBlock = SampleCount;

		//Loop through all MIDI messages until current playback time
		for (g_Msec += SampleBlock * (1000.0 / 44100.0); g_MidiMessage && g_Msec >= g_MidiMessage->time; g_MidiMessage = g_MidiMessage->next) {
			switch (g_MidiMessage->type) {
				case TML_NOTE_OFF:				// Stop a note
					if (playmidi_debug) fprintf(stderr, "    Stop Note\n");
                    tsf_channel_note_off(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->key);
					break;
                case TML_NOTE_ON:				// Play a note
					if (playmidi_debug) fprintf(stderr, "    Start Note\n");
                    tsf_channel_note_on(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->key, g_MidiMessage->velocity / 127.0f);
					break;
                case TML_CONTROL_CHANGE:		// MIDI controller messages
					if (playmidi_debug) fprintf(stderr, "    Set controller\n");
                    tsf_channel_midi_control(
                        g_TinySoundFont, g_MidiMessage->channel,
                        g_MidiMessage->control, g_MidiMessage->control_value
                    );
					break;
                case TML_PROGRAM_CHANGE:		// Channel program (preset) change
					if (playmidi_debug) fprintf(stderr, "    Set preset\n");
                    tsf_channel_set_presetnumber(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->program, 0);
					break;
				case TML_PITCH_BEND:            // Pitch wheel modification
					if (playmidi_debug) fprintf(stderr, "    Set pitch wheel\n");
                    tsf_channel_set_pitchwheel(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->pitch_bend);
					break;
                case TML_EOT:                   // End of track message
                    if (playmidi_debug) fprintf(stderr, "    End of track\n");
                    tsf_note_off_all(g_TinySoundFont);
                    break;
                default:
					break;
			}
		}

		//Render the block of audio samples in short format
		tsf_render_short(g_TinySoundFont, (short*)stream, SampleBlock, 0);
	}
}


///Play sequences
int playSequence() {
	SDL_AudioSpec out;
    
    auto set_output = [&out]() -> int {
        if (playmidi_debug) fprintf(stderr, "Define audio output format\n");

        //Define desired audio output format
        out.freq = 44100;
        out.format = AUDIO_S16;
        out.channels = 2;
        out.samples = 4096;
        out.callback = AudioCallback;

        if (playmidi_debug) fprintf(stderr, "Initialize audio system\n");

        //Initialize audio system
        if (SDL_AudioInit(TSF_NULL) < 0) {
            if (playmidi_debug) fprintf(stderr, "Could not initialize audio hardware or driver\n");
            return 0;
        }

        return 1;
    };
    
    //Set audio output format
    if (playmidi_debug) fprintf(stderr, "Set audio output format\n");
	if (!set_output()) {
        fprintf(stderr, "Could not open set audio output format\n");
        return 0;
    }
    
    //Set SoundFont
    if (playmidi_debug) fprintf(stderr, "Set SF2\n");
    g_TinySoundFont = tsf_load_filename(a_tml.sf2.c_str());
    if (!g_TinySoundFont) {
        fprintf(stderr, "Could not set SF2\n");
        return 0;
    }

    //Set SoundFont rendering output mode
    if (playmidi_debug) fprintf(stderr, "Set SF2 rendering output mode\n");
    tsf_set_output(g_TinySoundFont, TSF_STEREO_INTERLEAVED, out.freq, 0.0f);

	//Request desired audio output format
    if (playmidi_debug) fprintf(stderr, "Open audio hardware and output format\n");
	if (SDL_OpenAudio(&out, TSF_NULL) < 0) {
		fprintf(stderr, "Could not open the audio hardware or the desired audio output format\n");
		return 0;
	}

	//Start audio playback
    if (playmidi_debug) fprintf(stderr, "Play MIDI files\n");
    for (int m = 0; m < a_tml.mid.size(); ++m) {
        tml_message *tmp = NULL;
        std::string nam;
        
        tmp = tml_load_filename(a_tml.mid[m].c_str());
        nam = a_tml.mid[m].substr(a_tml.mid[m].find_last_of("\\/") + 1);
        if (!tmp) {
            fprintf(stderr, "Could not open %s\n", nam.c_str());
            continue;
        }
        
        fprintf(stdout, "Playing %s\n", nam.c_str());
        SDL_PauseAudio(0);
        g_Msec = 0.0;
        g_MidiMessage = tmp;
        while (g_MidiMessage != NULL) SDL_Delay(100);
        
        //tml_free(tmp);
    }
    
    //Clean up
    //tsf_close(g_TinySoundFont);
    a_tml = {};

	return 1;
}
