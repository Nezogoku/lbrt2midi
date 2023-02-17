#include <cstdint>
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


static tsf* g_TinySoundFont;		// Pointer to soundfont
static tml_message* g_MidiMessage;	// Midi playback state
static double g_Msec;				// Total playback time


playmidi::playmidi() {
	playmidi(false);
}

playmidi::playmidi(bool isDebug) {
	this->isDebug = isDebug;
	this->hasSF2 = false;
	g_MidiMessage = NULL;
}


int playmidi::setBank(std::string file) {
	bool success = true;

	//Load the SoundFont from a file
	if (isDebug) fprintf(stdout, "\nLoading soundbank from file\n");
	g_TinySoundFont = tsf_load_filename(file.c_str());
	if (!g_TinySoundFont) {
		fprintf(stderr, "Could not load %s\n", file.c_str());
		success = false;
	}

	return success;
}

int playmidi::setBank(char* data, int len) {
	bool success = true;

	//Load the SoundFont from memory
	if (isDebug) fprintf(stdout, "\nLoading soundbank from memory\n");
	g_TinySoundFont = tsf_load_memory(data, len);
	if (!g_TinySoundFont) {
		fprintf(stderr, "Could not load soundbank from memory\n");
		success = false;
	}

	return success;
}


int playmidi::setSequence(std::string file) {
	bool success = true;

	this->sequence_file = file;

	//Load midi from file
	if (isDebug) fprintf(stdout, "\nLoading sequence from file\n");
	g_MidiMessage = tml_load_filename(sequence_file.c_str());
	if (!g_MidiMessage) {
		fprintf(stderr, "Could not load %s\n", sequence_file.c_str());
		success = false;
	}

	return success;
}

int playmidi::setSequence(char* data, int len) {
	bool success = true;

	//Load midi from memory
	if (isDebug) fprintf(stdout, "\nLoading sequence from memory\n");
	g_MidiMessage = tml_load_memory(data, len);
	if (!g_MidiMessage) {
		fprintf(stderr, "Could not load sequence from memory\n");
		success = false;
	}

	return success;
}


///Callback function called by audio thread
void AudioCallback(void* data, uint8_t* stream, int len) {
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
				case TML_PITCH_BEND:			//pitch wheel modification
					tsf_channel_set_pitchwheel(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->pitch_bend);
					break;
				case TML_CONTROL_CHANGE:		//MIDI controller messages
					if (g_MidiMessage->control == TML_PAN_MSB) tsf_channel_set_pan(g_TinySoundFont, g_MidiMessage->channel, float(0x64 - g_MidiMessage->control_value));
					else tsf_channel_midi_control(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->control, g_MidiMessage->control_value);
					break;
			}
		}

		//Render the block of audio samples in float format
		tsf_render_float(g_TinySoundFont, (float*)stream, SampleBlock, 0);
	}
}


int playmidi::setAudioOutput() {
	bool success = true;

	if (isDebug) fprintf(stdout, "\nDefine audio output format\n");

	//Define desired audio output format
	outAudioSpec.freq = 44100;
	outAudioSpec.format = AUDIO_F32;
	outAudioSpec.channels = 2;
	outAudioSpec.samples = 4096;
	outAudioSpec.callback = AudioCallback;

	if (isDebug) fprintf(stdout, "\nInitialize audio system\n");

	//Initialize audio system
	if (SDL_AudioInit(TSF_NULL) < 0) {
		if (isDebug) fprintf(stderr, "Could not initialize audio hardware or driver\n");
		success = false;
	}

	return success;
}


int playmidi::playSequence() {
	g_Msec = 0x00;
	tsf_reset(g_TinySoundFont);

	if (isDebug) fprintf(stdout, "\nSet audio output format\n");

	//Set audio output format
	if (!setAudioOutput()) return 0;

    if (isDebug) fprintf(stdout, "\nSet SF2 rendering output mode\n");

    //Set SoundFont rendering output mode
    tsf_set_output(g_TinySoundFont, TSF_STEREO_INTERLEAVED, outAudioSpec.freq, 0.0f);

	//Request desired audio output format
	if (SDL_OpenAudio(&outAudioSpec, TSF_NULL) < 0) {
		fprintf(stdout, "\nCould not open the audio hardware or the desired audio output format\n");
		return 0;
	}


	//Start audio playback
	SDL_PauseAudio(0);

	fprintf(stdout, "Playing sequence %s . . .\n",
			sequence_file.substr(sequence_file.find_last_of("\\/") + 1).c_str());
	while (g_MidiMessage != NULL) SDL_Delay(50);

	tsf_close(g_TinySoundFont);
	tml_free(g_MidiMessage);

	return 1;
}
