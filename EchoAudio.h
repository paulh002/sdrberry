#pragma once
#include "Demodulator.h"

class EchoAudio : public Demodulator
{
  public:
	static bool create_modulator(int pcmrate, AudioOutput *audio_ouput, AudioInput *audio_input);
	static void destroy_modulator();

	EchoAudio(int pcmrate, AudioOutput *audio_ouput, AudioInput *audio_input);
	~EchoAudio();

	void operator()() override;
	void process(const IQSampleVector &samples_in, SampleVector &audio) override;

	atomic<bool> stop_flag{false};
	std::thread echo_thread;
};

