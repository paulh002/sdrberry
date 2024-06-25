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
  public:
	NoiseFilter();
	void Process(IQSampleVector &filter_in, IQSampleVector &filter_out);
};

