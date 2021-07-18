#pragma once

#include <liquid.h>

//using namespace edsp;


class FMDemodulator
{
	int Samplerate;
	int	FrequencyDeviation;
	int FilterTimeConstant;
	int	AudioSampleRate = 44100;
	int	Stereo;

	void	init();
	void	run(std::complex<float> *buf);

private:
	freqdem fdem;
	liquid_iirdes_filtertype ftype  = LIQUID_IIRDES_ELLIP;
	liquid_iirdes_bandtype   btype  = LIQUID_IIRDES_LOWPASS;
	liquid_iirdes_format     format = LIQUID_IIRDES_SOS;
	iirfilt_crcf			 filter;
	unsigned int order =   4;         // filter order
	float        fc    =   0.384f;      // cutoff frequency
	float        f0    =   0.0f;      // center frequency
	float        Ap    =   1.0f;      // pass-band ripple
	float        As    =  40.0f;      // stop-band attenuation
	unsigned int n     =  1024;         // number of samples
	dec_rate 
};

