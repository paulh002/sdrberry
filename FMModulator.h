#pragma once
#include "Demodulator.h"

class FMModulator : public Demodulator
{
  public:
	FMModulator(int mode, double ifrate, audioTone tone, DataBuffer<IQSample> *source_buffer, AudioInput *audio_input);
	~FMModulator();

	void process(const IQSampleVector &samples_in, SampleVector &samples);
	void operator()();
	static void destroy_modulator();
	static bool create_modulator(int mode, double ifrate, audioTone tone, DataBuffer<IQSample> *source_buffer, AudioInput *audio_input);
	atomic<bool> stop_flag{false};
	std::thread fmmod_thread;

  private:
	freqmod modFM{nullptr};
};
