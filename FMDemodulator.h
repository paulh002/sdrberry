#pragma once

#include <liquid.h>

//using namespace edsp;


class FMDemodulator
{
public:
	void	init(float iInSampleRate, float iOutSampleRate);
	void	run(std::complex<float> *buf);
	~FMDemodulator();

private:
	liquid_iirdes_filtertype ftype  = LIQUID_IIRDES_ELLIP;
	liquid_iirdes_bandtype   btype  = LIQUID_IIRDES_LOWPASS;
	liquid_iirdes_format     format = LIQUID_IIRDES_SOS;
	iirfilt_crcf			 filter = NULL, audio_filter =NULL;
	msresamp_crcf 			 resampler = NULL;
	freqdem					 fdem = NULL;
	unsigned int order =   4;         // filter order
	float        fc    =   0.384f;      // cutoff frequency
	float        f0    =   0.0f;      // center frequency
	float        Ap    =   1.0f;      // pass-band ripple
	float        As    =  40.0f;      // stop-band attenuation
	float			InSampleRate;
	float			OutSampleRate;
};

extern FMDemodulator	fm_demod;
void create_rx_fm_thread(float iInSampleRate, float iOutSampleRate);