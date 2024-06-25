#pragma once
#include <cassert>
#include <cmath>
#include <complex>
#include <cstdio>
#include <liquid/liquid.h>
#include <thread>
#include <vector>
#include <atomic>
#include "SdrberryTypeDefs.h"
#include "DataBuffer.h"
#include "FastFourier.h"
#include "Demodulator.h"

class NoiseFilter
{
  private:
	std::unique_ptr<FastFourier> fft;
	
  public:
	NoiseFilter();
	void Process(IQSampleVector &filter_in, IQSampleVector &filter_out);
};

