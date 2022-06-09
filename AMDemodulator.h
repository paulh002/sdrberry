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
#include "MorseDecoder.h"
#include "LMSNoisereducer.h"
#include "SpectralNoiseReduction.h"

class AMDemodulator : public Demodulator
{
public:
	static bool		create_demodulator(int mode, double ifrate, int pcmrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output);
	static void		destroy_demodulator();
	static std::string getName() { return "AMDemodulator";}
	
	AMDemodulator(int mode, double ifrate, int pcmrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output);
	~AMDemodulator();
	void	process(const IQSampleVector&	samples_in, SampleVector& audio) override;
	void	operator()() override;
	
	atomic<bool>		stop_flag {false};
	std::thread			amdemod_thread;
	
private:
	ampmodem		m_demod {nullptr};
	float			m_bandwidth;
	float			sample_ratio;
	Agc_class		agc;
	int				m_iagc = 0;
	unique_ptr<MorseDecoder> pMDecoder;
	unique_ptr<LMSNoisereducer> pLMS;
	unique_ptr<Xanr> pXanr;
	unique_ptr<SpectralNoiseReduction> pNoisesp;
};

void select_filter(int ifilter);