#pragma once
#include <cassert>
#include <cmath>
#include <complex>
#include <cstdio>
#include <liquid/liquid.h>
#include "DataBuffer.h"
#include "SdrberryTypeDefs.h"

class BandPassFilter
{
  public:
	BandPassFilter();
	BandPassFilter(float SampleRate, float Center, float cutOff);
	void executeBandpassFilter(const IQSampleVector &filter_in, IQSampleVector &filter_out);
	~BandPassFilter();
	void SetBandPassFilter(float SampleRate, float Center, float cutOff);

  private:
	iirfilt_crcf bandPassHandle{nullptr};

	const int filterOrder = 6;

	const liquid_iirdes_filtertype butterwurthType{LIQUID_IIRDES_BUTTER};
	const liquid_iirdes_bandtype bandFilterType{LIQUID_IIRDES_BANDPASS};
	const liquid_iirdes_format sosFormat{LIQUID_IIRDES_SOS};

	float cutOffFrequency;
	float centerFrequency;
	float passBandRipple;
	float StopBandAttenuation;
};

