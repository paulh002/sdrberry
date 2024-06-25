#include "NoiseFilter.h"

NoiseFilter::NoiseFilter()
{
	
}

void NoiseFilter::Process(IQSampleVector &filter_in, IQSampleVector &filter_out)
{
	std::vector<float> power, smoothedGainPtr;
	std::vector<float> gainPtr;
	std::unique_ptr<FastFourier> fft;

	fft.reset();
	fft = std::make_unique<FastFourier>(filter_in.size(), 0.0, 0.0);
	fft->ProcessForward(filter_in);
	power = fft->SpectrumPower();

	smoothedGainPtr.resize(filter_in.size());
	gainPtr.resize(filter_in.size());
	for (int i = 0; i < filter_in.size(); i++)
	{
		gainPtr[i] = ((power[i] > Demodulator::get_threshold()) ? 1.0f : 0.0f);
	}
	for (int j = 0; j < fft->GetfftBins().size(); j++)
	{
		float num = 0.0f;
		for (int k = -16; k < 16; k++)
		{
			int num2 = j + k;
			if (num2 >= fft->GetfftBins().size())
			{
				num2 -= fft->GetfftBins().size();
			}
			if (num2 < 0)
			{
				num2 += fft->GetfftBins().size();
			}
			num += gainPtr[num2];
		}
		float num3 = num / 32.0f;
		smoothedGainPtr[j] = num3;
	}
	for (int l = 0; l < fft->GetfftBins().size(); l++)
	{
		fft->GetfftBins()[l] = fft->GetfftBins()[l] * smoothedGainPtr[l];		
	}
	fft->ProcessBackward(filter_out);
}
