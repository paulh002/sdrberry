#pragma once
#include <math.h>
#include <complex>
#include <liquid/liquid.h>
#include "DataBuffer.h"
#include "SdrberryTypeDefs.h"

class Mixer
{
  public:
	void setup(float SampleRate,float fc);
	void executeMix(const IQSampleVector &filter_in, IQSampleVector &filter_out);

  private:
	float MixerFrequency;
	std::complex<float> NumericOscilator();

	long Steps;
	float SampleRate;
};

