#pragma once
#include <vector>
#include <alsa/asoundlib.h>
#include "RtMidi.h"

class MidiControle
{
public:
	int check_midi_input();
	MidiControle();
	~MidiControle();
	int read_midi_input();
	void openport(int port);
	
private:
	RtMidiIn  *midiin = 0;
	std::vector<unsigned char> message;
};

