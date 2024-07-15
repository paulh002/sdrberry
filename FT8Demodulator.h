#pragma once
#include <cstdio>
#include <cassert>
#include <cmath>
#include <complex>
#include <liquid/liquid.h>
#include <vector>
#include "DataBuffer.h"
#include "AudioOutput.h"
#include "Demodulator.h"
#include "FT8Processor.h"


class FT8Demodulator : public Demodulator
{
  public:
	static bool create_demodulator(double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output, int mode);
	static void destroy_demodulator();
	static std::string getName() { return "FT8Demodulator"; }
	static void setLowPassAudioFilterCutOffFrequency(int bandwidth);

	FT8Demodulator(double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output, int mode);
	~FT8Demodulator();
	void process(const IQSampleVector &samples_in, SampleVector &audio) ;
	void operator()() override;

	atomic<bool> stop_flag{false};
	std::thread amdemod_thread;

  private:
	ampmodem m_demod{nullptr};
	float m_bandwidth;
	float ft8_rate{12000.0};
	std::shared_ptr<FT8Processor> ft8processor;
	int wsjtxmode;
};