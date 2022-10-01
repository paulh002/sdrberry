#pragma once
#include <cstdio>
#include <cassert>
#include <cmath>
#include <complex>
#include <liquid.h>
#include <vector>
#include <tuple>
#include <array>
#include <list>
#include "DataBuffer.h"
#include "SdrberryTypeDefs.h"


class LMSNoisereducer
{
  public:
	LMSNoisereducer();
	~LMSNoisereducer();
	void Process(const IQSampleVector &samples_in, IQSampleVector &samples_out);
	
  private:
	eqlms_cccf q;
	unsigned int n = 32; // number of training symbols
	unsigned int p = 10; // equalizer order
	float mu = 0.500f;   // LMS learning rate
	bool first;
	std::list<std::complex<float>> samples_training;
};

class LMSNoiseReduction
{
  public:
	LMSNoiseReduction(int);
	~LMSNoiseReduction();
	void Process(const IQSampleVector &samples_in, IQSampleVector &samples_out);

  private:
	uint16_t calc_taps = 96;
	float mu_calc;
};

#define ANR_DLINE_SIZE 1024

class Xanr
{
  public:
	Xanr();
	~Xanr();
	void Process(const SampleVector &samples_in, SampleVector &samples_out);

  private:
	float ANR_d[ANR_DLINE_SIZE];
	int ANR_buff_size = ANR_DLINE_SIZE/2;
	int ANR_in_idx = 0;
	int ANR_dline_size = ANR_DLINE_SIZE;
	int ANR_mask = ANR_dline_size - 1;
	int ANR_delay = 16;
	int ANR_taps = 64;
	uint8_t ANR_notch = 0;
	
	float ANR_den_mult = 6.25e-10;
	float ANR_gamma = 0.1;
	float ANR_lidx = 120.0;
	float ANR_lidx_min = 120.0;
	float ANR_lidx_max = 200.0;
	float ANR_lincr = 1.0;
	float ANR_ldecr = 3.0;
	float ANR_ngamma = 0.001;
	float ANR_two_mu = 0.0001;
	float ANR_w[ANR_DLINE_SIZE];
};