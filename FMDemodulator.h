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
#include "AgcProcessor.h"

class FMDemodulator : public Demodulator
{
public:
	static bool create_demodulator(double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output);
	static void destroy_demodulator();
	static std::string getName() { return "FMDemodulator";}
	static void setLowPassAudioFilterCutOffFrequency(int bandwidth);

	FMDemodulator(double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output);
	~FMDemodulator();
	void	process(IQSampleVector&	samples_in, SampleVector& audio) ;
	void	operator()();
	atomic<bool> stop_flag{false};		
	
private:
	freqdem	demodFM {nullptr};
	AgcProcessor AgcProc;
	int threshold = -100;
	int squelch_mode = 0;
};