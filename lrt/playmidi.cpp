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
static tml_message *g_MidiMessage;      // Pointer to Midi playback state
static double g_Msec;                   // Total playback time

///Callback function called by audio thread
void AudioCallback(void *data, unsigned char *stream, int len) {
	//Number of samples to process
	int SampleBlock,
		SampleCount = (len / (2 * sizeof(float))); //2 output channels

	for (SampleBlock = TSF_RENDER_EFFECTSAMPLEBLOCK; SampleCount; SampleCount -= SampleBlock, stream += (SampleBlock * (2 * sizeof(float)))) {
		//Process the MIDI playback, then process TSF_RENDER_EFFECTSAMPLEBLOCK samples at once
		if (SampleBlock > SampleCount) SampleBlock = SampleCount;

		//Loop through all MIDI messages until current playback time
		for (g_Msec += SampleBlock * (1000.0 / 44100.0); g_MidiMessage && g_Msec >= g_MidiMessage->time; g_MidiMessage = g_MidiMessage->next) {
			switch (g_MidiMessage->type) {
				case TML_PROGRAM_CHANGE:		//channel program (preset) change
					tsf_channel_set_presetnumber(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->program, 0);
					break;
				case TML_NOTE_ON:				//play a note
					tsf_channel_note_on(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->key, g_MidiMessage->velocity / 127.0f);
					break;
				case TML_NOTE_OFF:				//stop a note
					tsf_channel_note_off(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->key);
					break;
				case TML_PITCH_BEND:            //pitch wheel modification
					tsf_channel_set_pitchwheel(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->pitch_bend);
					break;
				case TML_CONTROL_CHANGE:		//MIDI controller messages
					tsf_channel_midi_control(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->control, g_MidiMessage->control_value);
					break;
			}
		}

		//Render the block of audio samples in float format
		tsf_render_float(g_TinySoundFont, (float*)stream, SampleBlock, 0);
	}
}


playmidi::~playmidi() {
    if (this->num_seqs) setAmountSequences(0);
    if (this->bank) tsf_close(this->bank);

    g_TinySoundFont = NULL;
    g_MidiMessage = NULL;
    g_Msec = 0;
}

playmidi::playmidi() : playmidi(0, false) {}
playmidi::playmidi(int num_seq) : playmidi(num_seq, false) {}
playmidi::playmidi(int num_seq, bool isDebug) {
	this->debug = isDebug;
	this->num_seqs = 0;
    this->bank = NULL;
    this->mesg = 0;
    this->seq_names = 0;
    if (num_seq) setAmountSequences(num_seq);

    g_TinySoundFont = NULL;
    g_MidiMessage = NULL;
    g_Msec = 0;
}


bool playmidi::hasDuplicate(std::string name) {
    return std::count(this->seq_names, this->seq_names + this->num_seqs, name);
}


void playmidi::setDebug(bool isDebug) { this->debug = isDebug; }

void playmidi::setAmountSequences(int num_seq) {
    if (this->mesg) {
        for (int s = 0; s < this->num_seqs; ++s) {
            if (this->mesg[s]) tml_free(this->mesg[s]);
        }
        delete[] this->mesg; this->mesg = 0;
        delete[] this->seq_names; this->seq_names = 0;
    }

    this->num_seqs = num_seq;
    if (this->num_seqs) {
        this->seq_names = new std::string[this->num_seqs] {};
        this->mesg = new tml_message*[this->num_seqs] {};

        for (int s = 0; s < this->num_seqs; ++s) this->mesg[s] = NULL;
    }
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

int playmidi::setBank(unsigned data_size, unsigned char *data) {
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

int playmidi::setSequence(std::string midi_file, int s) {
	bool success = true;

    std::string tnam = midi_file;
    tnam = tnam.substr(tnam.find_last_of("\\/") + 1);
    tnam = tnam.substr(0, tnam.find_last_of('.'));

    if (!hasDuplicate(tnam)) {
        this->seq_names[s] = tnam;

        //Load midi from file
        if (this->debug) fprintf(stderr, "\nLoading sequence from a file\n");
        this->mesg[s] = tml_load_filename(midi_file.c_str());
        if (!this->mesg[s]) {
            fprintf(stderr, "Could not load sequence from a file\n");
            success = false;
        }
    }

	return success;
}

int playmidi::setSequence(unsigned data_size, unsigned char *data, std::string data_name, int s) {
	bool success = true;

    if (!hasDuplicate(data_name)) {
        this->seq_names[s] = data_name;

        //Load midi from file
        if (this->debug) fprintf(stderr, "\nLoading sequence from memory\n");
        this->mesg[s] = tml_load_memory(data, data_size);
        if (!this->mesg[s]) {
            fprintf(stderr, "Could not load sequence\n");
            success = false;
        }
    }

	return success;
}

int playmidi::setAudioOutput() {
	bool success = true;

	if (this->debug) fprintf(stderr, "\nDefine audio output format\n");

	//Define desired audio output format
	this->outAudioSpec.freq = 44100;
	this->outAudioSpec.format = AUDIO_F32;
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
	fprintf(stdout, "\nPlaying sequences . . .\n");

    SDL_PauseAudio(0);
    for (int s = 0; s < this->num_seqs; ++s) {
        if (!this->mesg[s]) continue;

        fprintf(stdout, "Playing %s\n", this->seq_names[s].c_str());

        g_TinySoundFont = tsf_copy(this->bank);
        g_MidiMessage = this->mesg[s];

        while (g_MidiMessage != NULL) SDL_Delay(50);

        tsf_reset(g_TinySoundFont);
        g_Msec = 0;
    }
    tsf_close(g_TinySoundFont); g_TinySoundFont = NULL;

	return 1;
}
