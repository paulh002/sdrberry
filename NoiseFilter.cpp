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
	power = fft->SpectrumPower();

	smoothedGainPtr.resize(size);
	gainPtr.resize(size);
	for (int i = 0; i < size; i++)
	{
		gainPtr[i] = ((power[i] > Demodulator::get_threshold()) ? 1.0f : 0.0f);
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
