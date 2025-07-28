#pragma once
#include "Demodulator.h"

class EchoAudio : public Demodulator
{
  public:
	static bool create_modulator(AudioOutput *audio_ouput, AudioInput *audio_input);
	static void destroy_modulator();

	EchoAudio(AudioOutput *audio_ouput, AudioInput *audio_input);
	~EchoAudio();

	void operator()();

	atomic<bool> stop_flag{false};
	std::thread echo_thread;
};

