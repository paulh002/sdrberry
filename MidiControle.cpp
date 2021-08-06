#include "MidiControle.h"
#include <alsa/asoundlib.h>
#include <vector>
#include "RtMidi.h"

MidiControle::MidiControle()
{
	// RtMidiIn constructor
	try {
		midiin = new RtMidiIn();
	}
	catch(RtMidiError &error) {
		error.printMessage();
		exit(EXIT_FAILURE);
	}
}

MidiControle::~MidiControle()
{
	if (midiin)
		delete midiin;
}

// Check inputs.
int MidiControle::check_midi_input()
{
	if (midiin == nullptr)
		return -1;
	unsigned int nPorts = midiin->getPortCount();
	std::cout << "\nThere are " << nPorts << " MIDI input sources available.\n";
	std::string portName;
	for (unsigned int i = 0; i < nPorts; i++) {
		try {
			portName = midiin->getPortName(i);
		}
		catch (RtMidiError &error) {
			error.printMessage();
		}
		std::cout << "  Input Port #" << i + 1 << ": " << portName << '\n';
	}
	return nPorts;
}

void MidiControle::openport(int port)
{
	midiin->openPort(port);
	//midiin->ignoreTypes(false, false, false);
}


int MidiControle::read_midi_input()
{
	double stamp;
	int nBytes, i;
	
	stamp = midiin->getMessage(&message);
	nBytes = message.size();
	for (i = 0; i < nBytes; i++)
		std::cout << "Byte " << i << " = " << (int)message[i] << ", ";
	if (nBytes > 0)
		std::cout << "stamp = " << stamp << std::endl;
	return 0;
}
