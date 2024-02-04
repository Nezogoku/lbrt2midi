#include <algorithm>
#include <cstdio>
#include <string>
#include "playmidi.hpp"
#include "minisdl_audio.h"
#define TSF_IMPLEMENTATION
#include "tsf.h"
#define TML_IMPLEMENTATION
#include "tml.h"

#define TSF_STATIC
#define TML_MEMCPY
#define TML_MALLOC
#define TML_REALLOC
#define TML_FREE
#define TML_STATIC

static tsf *g_TinySoundFont;            // Pointer to Soundfont
static tml_message **g_MidiMessage;     // Pointer to Midi playback state
static double g_Msec;                   // Total playback time

///Callback function called by audio thread
void AudioCallback(void *data, unsigned char *stream, int len) {
	//Number of samples to process
	int SampleBlock,
		SampleCount = (len / (2 * sizeof(short))); //2 output channels

	for (SampleBlock = TSF_RENDER_EFFECTSAMPLEBLOCK; SampleCount; SampleCount -= SampleBlock, stream += (SampleBlock * (2 * sizeof(short)))) {
		//Process MIDI playback, then process TSF_RENDER_EFFECTSAMPLEBLOCK samples at once
		if (SampleBlock > SampleCount) SampleBlock = SampleCount;

		//Loop through all MIDI messages until current playback time
		for (g_Msec += SampleBlock * (1000.0 / 44100.0); (*g_MidiMessage) && g_Msec >= (*g_MidiMessage)->time; (*g_MidiMessage) = (*g_MidiMessage)->next) {
			switch ((*g_MidiMessage)->type) {
				case TML_NOTE_OFF:				// Stop a note
					tsf_channel_note_off(g_TinySoundFont, (*g_MidiMessage)->channel, (*g_MidiMessage)->key);
					break;
                case TML_NOTE_ON:				// Play a note
					tsf_channel_note_on(g_TinySoundFont, (*g_MidiMessage)->channel, (*g_MidiMessage)->key, (*g_MidiMessage)->velocity / 127.0f);
					break;
                case TML_CONTROL_CHANGE:		// MIDI controller messages
					tsf_channel_midi_control(g_TinySoundFont, (*g_MidiMessage)->channel, (*g_MidiMessage)->control, (*g_MidiMessage)->control_value);
					break;
                case TML_PROGRAM_CHANGE:		// Channel program (preset) change
					tsf_channel_set_presetnumber(g_TinySoundFont, (*g_MidiMessage)->channel, (*g_MidiMessage)->program, 0);
					break;
				case TML_PITCH_BEND:            // Pitch wheel modification
					tsf_channel_set_pitchwheel(g_TinySoundFont, (*g_MidiMessage)->channel, (*g_MidiMessage)->pitch_bend);
					break;
				case TML_SET_TEMPO:             // Tempo change
                    break;
			}
		}

		//Render the block of audio samples in short format
		tsf_render_short(g_TinySoundFont, (short*)stream, SampleBlock, 0);
	}
}


playmidi::~playmidi() {
    if (!this->seq_name.empty()) this->seq_name.clear();
    if (this->bank) tsf_close(this->bank);

    g_TinySoundFont = NULL;
    g_MidiMessage = NULL;
    g_Msec = 0;
}

playmidi::playmidi() {
    this->debug = false;
    this->seq_name = "";
    this->bank = NULL;
    this->mesg = 0;

    g_TinySoundFont = NULL;
    g_MidiMessage = NULL;
    g_Msec = 0;
}


int playmidi::setBank(std::string bank_file) {
	bool success = true;

	//Load the SoundFont from a file
	if (this->debug) fprintf(stderr, "\nLoading soundbank from a file\n");
	this->bank = tsf_load_filename(bank_file.c_str());
	if (!this->bank) {
		fprintf(stderr, "Could not load soundbank from file\n");
		success = false;
	}

	return success;
}

int playmidi::setBank(unsigned char *data, unsigned data_size) {
	bool success = true;

	//Load the SoundFont from memory
	if (this->debug) fprintf(stderr, "\nLoading soundbank from memory\n");
	this->bank = tsf_load_memory(data, data_size);
	if (!this->bank) {
		fprintf(stderr, "Could not load soundbank from memory\n");
		success = false;
	}

	return success;
}

int playmidi::setSequence(std::string midi_file) {
	bool success = true;

    this->seq_name = midi_file.substr(midi_file.find_last_of("\\/") + 1);
    this->seq_name = this->seq_name.substr(0, this->seq_name.find_last_of('.'));
    
    //Load midi from file
    if (this->debug) fprintf(stderr, "\nLoading sequence from a file\n");
    this->mesg = tml_load_filename(midi_file.c_str());
    if (!this->mesg) {
        fprintf(stderr, "Could not load sequence from a file\n");
        success = false;
    }

	return success;
}

int playmidi::setSequence(unsigned char *data, unsigned data_size) {
	bool success = true;

    //Load midi from file
    if (this->debug) fprintf(stderr, "\nLoading sequence from memory\n");
    this->mesg = tml_load_memory(data, data_size);
    if (!this->mesg) {
        fprintf(stderr, "Could not load sequence\n");
        success = false;
    }

	return success;
}

int playmidi::setAudioOutput() {
	bool success = true;

	if (this->debug) fprintf(stderr, "\nDefine audio output format\n");

	//Define desired audio output format
	this->outAudioSpec.freq = 44100;
	this->outAudioSpec.format = AUDIO_S16;
	this->outAudioSpec.channels = 2;
	this->outAudioSpec.samples = 4096;
	this->outAudioSpec.callback = AudioCallback;

	if (this->debug) fprintf(stderr, "\nInitialize audio system\n");

	//Initialize audio system
	if (SDL_AudioInit(TSF_NULL) < 0) {
		if (this->debug) fprintf(stderr, "Could not initialize audio hardware or driver\n");
		success = false;
	}

	return success;
}


int playmidi::playSequence() {
	g_TinySoundFont = tsf_copy(this->bank);

    //Set audio output format
    if (this->debug) fprintf(stderr, "\nSet audio output format\n");
	if (!setAudioOutput()) return 0;

    //Set SoundFont rendering output mode
    if (this->debug) fprintf(stderr, "\nSet SF2 rendering output mode\n");
    if (g_TinySoundFont) {
        tsf_set_output(g_TinySoundFont, TSF_STEREO_INTERLEAVED, outAudioSpec.freq, 0.0f);
    }
    else {
        if (this->debug) fprintf(stderr, "Could not set SF2 rendering output mode\n");
        return 0;
    }

	//Request desired audio output format
	if (SDL_OpenAudio(&outAudioSpec, TSF_NULL) < 0) {
		fprintf(stderr, "\nCould not open the audio hardware or the desired audio output format\n");
		return 0;
	}

	//Start audio playback
	fprintf(stdout, "Playing %s\n", this->seq_names[s].c_str());

    SDL_PauseAudio(0);
    g_MidiMessage = this->mesg;
    while ((*g_MidiMessage) != NULL) SDL_Delay(50);
    
    tsf_reset(g_TinySoundFont);
    tsf_close(g_TinySoundFont);
    g_TinySoundFont = NULL;
    g_Msec = 0;
    

	return 1;
}
