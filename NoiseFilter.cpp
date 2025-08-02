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
#include "Demodulator.h"
#include "FastFourier.h"
#include "NoiseFilter.h"

NoiseFilter::NoiseFilter()
{
	fft.reset();
	fft = std::make_unique<FastFourier>(0, 0.0, 0.0);
}

void NoiseFilter::Process(IQSampleVector &filter_in, IQSampleVector &filter_out)
{
	std::vector<float> power, smoothedGainPtr;
	std::vector<float> gainPtr;
	int size = filter_in.size();

	fft->ProcessForward(filter_in);
	fft->SpectrumPower(power);

	smoothedGainPtr.resize(size);
	gainPtr.resize(size);
	float thresshold = Demodulator::get_threshold();
	for (int i = 0; i < size; i++)
	{
		gainPtr[i] = ((power[i] > thresshold) ? 1.0f : 0.0f);
	}
	for (int j = 0; j < size; j++)
	{
		float num = 0.0f;
		for (int k = -16; k < 16; k++)
		{
			int num2 = j + k;
			if (num2 >= size)
			{
				num2 -= size;
			}
			if (num2 < 0)
			{
				num2 += size;
			}
			num += gainPtr[num2];
		}
		float num3 = num / 32.0f;
		smoothedGainPtr[j] = num3;
	}
	fft->ProcessfftBins(smoothedGainPtr);
	fft->ProcessBackward(filter_out);
}
