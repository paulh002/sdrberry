#pragma once
#include <cstdio>
#include <cassert>
#include <cmath>
#include <complex>
#include <liquid.h>
#include <vector>
#include <tuple>
#include <array>
#include "DataBuffer.h"
#include "Audiodefs.h"

#define NR_FFT_L 1024 //256
#define PI 3.14159265358979323846

class SpectralNoiseReduction
{
  public:
	SpectralNoiseReduction(float pcmrate, tuple<float, float> bandwidth);
	void Process(const SampleVector &samples_in, SampleVector &samples_out);
	void SpectralNoiseReductionInit();

  private:
	float pslp[NR_FFT_L / 2];
	float xt[NR_FFT_L / 2];
	float xtr;
	float pre_power;
	float post_power;
	float power_ratio;
	int16_t NN;
	
	float ax;   //=0.8;       // ax=exp(-tinc/tax); % noise output smoothing factor
	float ap;   //=0.9;        // ap=exp(-tinc/tap); % noise output smoothing factor
	float xih1; // = 31.6;
	
	float xih1r ;
	float pfac ;
	uint8_t NR_init_counter;
	int NR_first_time_2;

	float NR_alpha = 0.95;

	const float tinc = 0.00533333; // frame time 5.3333ms
	const float tax = 0.0239;	  // noise output smoothing time constant = -tinc/ln(0.8)
	const float tap = 0.05062;	 // speech prob smoothing time constant = -tinc/ln(0.9) tinc = frame time (5.33ms)
	const float psthr = 0.99;	  // threshold for smoothed speech probability [0.99]
	const float pnsaf = 0.01;	  // noise probability safety value [0.01]
	const float asnr = 20;		   // active SNR in dB
	const float psini = 0.5;	   // initial speech probability [0.5]
	const float pspri = 0.5;	   // prior speech probability [0.5]
	float DF;
	float SampleRate;
	tuple<float, float> bandwidth;

	array<float, NR_FFT_L / 2> NR_last_sample_buffer_R;
	array<float, NR_FFT_L / 2> NR_last_sample_buffer_L;
	array<array<float, 2>, NR_FFT_L / 2> NR_Gts;
	array<float, NR_FFT_L / 2> NR_G;
	array<array<float, 2>, NR_FFT_L / 2> NR_Nest;
	array<float, NR_FFT_L / 2> NR_Hk_old;
	array<complex<float>, NR_FFT_L> NR_FFT_buffer;
	array<complex<float>, NR_FFT_L> NR_FFT_buffer1;
	array<float, NR_FFT_L> NR_last_iFFT_result;
	array<array<float, 3>, NR_FFT_L /2> NR_X;
	array<float, NR_FFT_L / 2> NR_SNR_prio;
	array<float, NR_FFT_L / 2> NR_SNR_post;
	array<float, NR_FFT_L / 2> NR_long_tone_gain;
};