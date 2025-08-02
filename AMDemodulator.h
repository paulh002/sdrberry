#pragma once
#include <cstdio>
#include <cassert>
#include <cmath>
#include <complex>
#include <liquid/liquid.h>
#include <vector>
#include "DataBuffer.h"
#include "AudioOutput.h"
#include "AudioInput.h"
#include "Demodulator.h"
#include "MorseDecoder.h"
#include "LMSNoisereducer.h"
#include "SpectralNoiseReduction.h"
#include "FastFourier.h"
#include "NoiseFilter.h"

class AMDemodulator : public Demodulator
{
public:
	static bool		create_demodulator(int mode, double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output);
	static void		destroy_demodulator();
	static std::string getName() { return "AMDemodulator";}
	static void		setLowPassAudioFilterCutOffFrequency(int ifilter);

	AMDemodulator(int mode, double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output);
	~AMDemodulator();
	void	process(IQSampleVector&	samples_in, SampleVector& audio);
	void	operator()();
	
	atomic<bool>		stop_flag {false};
	std::thread			amdemod_thread;
	
	
  private:
	ampmodem		demodulatorHandle {nullptr};
	unique_ptr<MorseDecoder> pMDecoder;
	unique_ptr<LMSNoisereducer> pLMS;
	unique_ptr<Xanr> pXanr;
	unique_ptr<SpectralNoiseReduction> pNoisesp;
	unique_ptr<NoiseFilter> nf;
	int receiverMode;
};

