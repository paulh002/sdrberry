#pragma once
#include "Demodulator.h"
#include "WavWriter.h"

class recorder : public Demodulator
{
  public:
	static bool create_recorder(AudioOutput *audio_output, AudioInput *audio_input);
	static void destroy_recorder();

	recorder(AudioOutput *audio_ouput, AudioInput *audio_input);
	~recorder();

	void operator()();

	std::atomic<bool> stop_flag{false};
	std::thread recorder_thread;
};