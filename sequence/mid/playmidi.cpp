#include <cstdint>
#include <iostream>
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

using std::cerr;
using std::cout;
using std::endl;
using std::string;



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
	//Load the SoundFont from a file
	if (isDebug) cout << "Loading SF2 from file" << endl;
	g_TinySoundFont = tsf_load_filename(file.c_str());
	if (!g_TinySoundFont) {
		cerr << "Could not load " << file << endl;
		return 0;
	}

	return 1;
}

int playmidi::setBank(char* data, int len) {
	//Load the SoundFont from memory
	if (isDebug) cout << "Loading SF2 from memory" << endl;
	g_TinySoundFont = tsf_load_memory(data, len);
	if (!g_TinySoundFont) {
		cerr << "Could not load from memory" << endl;
		return 0;
	}

	return 1;
}


int playmidi::setSequence(std::string file) {
	this->sequence_file = file;

	//Load midi from file
	if (isDebug) cout << "Loading MID from file" << endl;
	g_MidiMessage = tml_load_filename(file.c_str());
	if (!g_MidiMessage) {
		cerr << "Could not load " << file << endl;
		return 0;
	}

	return 1;
}

int playmidi::setSequence(char* data, int len) {
	//Load the SoundFont from memory
	if (isDebug) cout << "Loading MID from memory" << endl;
	g_MidiMessage = tml_load_memory(data, len);
	if (!g_MidiMessage) {
		cerr << "Could not load from memory" << endl;
		return 0;
	}

	return 1;
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
					tsf_channel_midi_control(g_TinySoundFont, g_MidiMessage->channel, g_MidiMessage->control, g_MidiMessage->control_value);
					break;
			}
		}

		//Render the block of audio samples in float format
		tsf_render_float(g_TinySoundFont, (float*)stream, SampleBlock, 0);
	}
}


int playmidi::setAudioOutput() {
	//Define desired audio output format
	if (isDebug) cout << "Defining audio output format" << endl;
	outAudioSpec.freq = 44100;
	outAudioSpec.format = AUDIO_F32;
	outAudioSpec.channels = 2;
	outAudioSpec.samples = 4096;
	outAudioSpec.callback = AudioCallback;

	//Initialize audio system
	if (isDebug) cout << "Initializing audio system" << endl;
	if (SDL_AudioInit(TSF_NULL) < 0) {
		cerr << "Could not initialize audio hardware or driver" << endl;
		return 0;
	}

	return 1;
}


int playmidi::playSequence() {
	g_Msec = 0x0;
	tsf_reset(g_TinySoundFont);

	//Set audio output format
	if (isDebug) cout << "Setting audio output format" << endl;
	if (!setAudioOutput()) return 0;

    //Set SoundFont rendering output mode
    if (isDebug) cout << "Setting SF2 rendering output mode" << endl;
	tsf_set_output(g_TinySoundFont, TSF_STEREO_INTERLEAVED, outAudioSpec.freq, 0.0f);

	//Request desired audio output format
	if (SDL_OpenAudio(&outAudioSpec, TSF_NULL) < 0) {
		cerr << "Could not open the audio hardware or the desired audio output format" << endl;
		return 0;
	}


	//Start audio playback
	SDL_PauseAudio(0);

	cout << "Playing sequence "
		 << sequence_file.substr(sequence_file.find_last_of("\\/") + 1)
		 << " . . ." << endl;
	while (g_MidiMessage != NULL) SDL_Delay(100);

	tsf_close(g_TinySoundFont);
	tml_free(g_MidiMessage);

	return 1;
}
