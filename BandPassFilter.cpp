#include "BandPassFilter.h"

BandPassFilter::BandPassFilter(float SampleRate, float Center, float cutOff)
{
	cutOffFrequency = cutOff / (float)SampleRate;  // cutoff frequency
	centerFrequency = Center / (float)SampleRate; // center frequency
	passBandRipple = 1.0f;								 // pass-band ripple
	StopBandAttenuation = 40.0f;						 // stop-band attenuation

	if (bandPassHandle)
		iirfilt_crcf_destroy(bandPassHandle);
	bandPassHandle = iirfilt_crcf_create_prototype(butterwurthType, bandFilterType, sosFormat, filterOrder, cutOffFrequency, centerFrequency, passBandRipple, StopBandAttenuation);
	iirfilt_crcf_print(bandPassHandle);
}

BandPassFilter::BandPassFilter()
{
	cutOffFrequency = 0;
	centerFrequency = 0; 
	passBandRipple = 1.0f;						  // pass-band ripple
	StopBandAttenuation = 40.0f;				  // stop-band attenuation
	bandPassHandle = nullptr;
}

BandPassFilter::~BandPassFilter()
{
	if (bandPassHandle)
		iirfilt_crcf_destroy(bandPassHandle);

	bandPassHandle = nullptr;

}

void BandPassFilter::SetBandPassFilter(float SampleRate, float Center, float cutOff)
{
	cutOffFrequency = cutOff / (float)SampleRate; // cutoff frequency
	centerFrequency = Center / (float)SampleRate; // center frequency
	passBandRipple = 1.0f;						  // pass-band ripple
	StopBandAttenuation = 40.0f;				  // stop-band attenuation

	if (bandPassHandle)
		iirfilt_crcf_destroy(bandPassHandle);
	bandPassHandle = iirfilt_crcf_create_prototype(butterwurthType, bandFilterType, sosFormat, filterOrder, cutOffFrequency, centerFrequency, passBandRipple, StopBandAttenuation);
	iirfilt_crcf_print(bandPassHandle);
}

void BandPassFilter::executeBandpassFilter(const IQSampleVector &filter_in,
										   IQSampleVector &filter_out)
{
	if (bandPassHandle == nullptr)
	{
		filter_out = filter_in;
		return;
	}
	for (auto &col : filter_in)
	{
		std::complex<float> v;

		iirfilt_crcf_execute(bandPassHandle, col, &v);
		filter_out.insert(filter_out.end(), v);
	}
}