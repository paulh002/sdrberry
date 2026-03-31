#pragma once
#include "AudioOutput.h"
#include "DataBuffer.h"
#include "Demodulator.h"
#include "WSPRProcessor.h"
#include <cassert>
#include <cmath>
#include <complex>
#include <cstdio>
#include <liquid/liquid.h>
#include <vector>

class WSPRDemodulator : public Demodulator
{
  public:
	static bool create_demodulator(double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output, int mode);
	static void destroy_demodulator();
	static std::string getName() { return "WSPRDemodulator"; }
	static void setLowPassAudioFilterCutOffFrequency(int bandwidth);

	WSPRDemodulator(double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output, int mode);
	~WSPRDemodulator();
	void process(IQSampleVector &samples_in, SampleVector &audio);
	unsigned long readwavfile(char *ptr_to_infile, int ntrmin, IQSampleVector &samples_out, SampleVector &audio);
	void operator()();

	atomic<bool> stop_flag{false};
	std::thread amdemod_thread;

  private:
	ampmodem demod{nullptr};
	float bandwidth;
	float wspr_rate{12000.0};
	std::shared_ptr<WSPRProcessor> wspr_processor;
	int wsjtxmode;
};