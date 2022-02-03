#pragma once
#include <cstdio>
#include <cassert>
#include <cmath>
#include <complex>
#include <liquid.h>
#include <vector>
#include "DataBuffer.h"
#include "AudioOutput.h"
#include "Demodulator.h"

class FT8Demodulator :
    public Demodulator
{
  public:
	static bool create_demodulator(int mode, double ifrate, int pcmrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output);
	static void destroy_demodulator();
	static std::string getName() { return "FT8Demodulator"; }

	FT8Demodulator(int mode, double ifrate, int pcmrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output);
	~FT8Demodulator();
	void process(const IQSampleVector &samples_in, SampleVector &audio) override;
	void operator()() override;

	atomic<bool> stop_flag{false};
	std::thread amdemod_thread;

  private:
	ampmodem m_demod{nullptr};
	float m_bandwidth;
	float ft8_rate{12000.0};
};

