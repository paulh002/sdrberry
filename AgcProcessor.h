#pragma once
#include <cstdio>
#include <cassert>
#include <cmath>
#include <complex>
#include <liquid/liquid.h>
#include <vector>
#include "DataBuffer.h"
#include "SdrberryTypeDefs.h"

class AgcProcessor
{
  private:
	agc_crcf agc_object{};
	bool squelch_enabled;
	float threshold;

  public:
	AgcProcessor(float bandwidth = 1e-3f);
	~AgcProcessor();
	
	void Process(IQSampleVector &samples_in);
	float getSignalLevel();
	float getRssi();
	void SetSquelch(bool squelch);
	void SetSquelchThreshold(float _threshold);
	void Lock(bool lock);
	void print();
	bool squelch();
};

