#include "FastFourier.h"

FastFourier::FastFourier(int nbins, float rs, float downMixFrequency)
	: numberOffBins(nbins), resampleRate(rs)
{
	float As{60.0f};
	
	fftBins.resize(numberOffBins);
	hammingWindow.clear();
	for (int i = 0; i < numberOffBins; i++)
	{
		hammingWindow.insert(hammingWindow.end(), liquid_windowf(LIQUID_WINDOW_HAMMING, i, numberOffBins, 0));
	}
	if (resampleRate > 0.0)
		resampleHandle = msresamp_crcf_create(resampleRate, As);
	if (downMixFrequency > 0.0)
	{
		float rad_per_sample = 2.0f * (float)M_PI * downMixFrequency;
		if (ncoMixerHandle == nullptr)
			ncoMixerHandle = nco_crcf_create(LIQUID_NCO);
		nco_crcf_set_phase(ncoMixerHandle, 0.0f);
		nco_crcf_set_frequency(ncoMixerHandle, rad_per_sample);
	}
}

void FastFourier::SetParameters(int nbins, float resampleRate, float downMixFrequency)
{
	float As{60.0f};
	std::unique_lock<std::mutex> lock(mutexSingleEntry);
	
	fftBins.resize(numberOffBins);
	if (resampleHandle != nullptr)
		msresamp_crcf_destroy(resampleHandle);
	if (ncoMixerHandle != nullptr)
		nco_crcf_destroy(ncoMixerHandle);
	resampleHandle = nullptr;
	ncoMixerHandle = nullptr;
	hammingWindow.clear();
	for (int i = 0; i < numberOffBins; i++)
	{
		hammingWindow.insert(hammingWindow.end(), liquid_windowf(LIQUID_WINDOW_HAMMING, i, numberOffBins, 0));
	}
	
	if (resampleRate > 0.0)
		resampleHandle = msresamp_crcf_create(resampleRate, As);
	if (downMixFrequency > 0.0)
	{
		float rad_per_sample = 2.0f * (float)M_PI * downMixFrequency;
		if (ncoMixerHandle == nullptr)
			ncoMixerHandle = nco_crcf_create(LIQUID_NCO);
		nco_crcf_set_phase(ncoMixerHandle, 0.0f);
		nco_crcf_set_frequency(ncoMixerHandle, rad_per_sample);
	}
	//printf("FFT: nbins %d, reasmpleRate %f, downMixFrequency %f \n", nbins, resampleRate, downMixFrequency);
}

FastFourier::~FastFourier()
{
	if (resampleHandle)
		msresamp_crcf_destroy(resampleHandle);
	if (ncoMixerHandle != nullptr)
		nco_crcf_destroy(ncoMixerHandle);
}

void FastFourier::Process(const IQSampleVector &input)
{
	std::unique_lock<std::mutex> lock(mutexSingleEntry);
	unsigned int num_written;

	if (resampleHandle)
	{
		std::vector<std::complex<float>> data, filter_out;

		for (auto col = input.begin(); col != input.begin() + numberOffBins && col != input.end(); col++)
		{
			std::complex<float> v;

			if (ncoMixerHandle)
			{
				nco_crcf_step(ncoMixerHandle);
				nco_crcf_mix_down(ncoMixerHandle, *col, &v);
			}
			else
			{
				v = *col;
			}
			filter_out.push_back(v);
		}
		data.resize(filter_out.size());
		msresamp_crcf_execute(resampleHandle, (std::complex<float> *)filter_out.data(), filter_out.size(), (std::complex<float> *)data.data(), &num_written);
		data.resize(num_written);
		inputData.insert(inputData.end(), data.begin(), data.end());
		if (inputData.size() < numberOffBins)
			return;
	}
	else
	{
		if (input.size() < numberOffBins)
		{
			inputData.insert(inputData.end(), input.begin(), input.end());
			if (inputData.size() < numberOffBins)
				return;
		}
		else
		{
			auto end = input.begin() + numberOffBins;
			inputData.insert(inputData.end(), input.begin(), end);
		}
	}

	for (int i = 0; i < numberOffBins; i++)
	{
		inputData[i].real(inputData[i].real() * hammingWindow[i]);
		if (invert)
			inputData[i].imag(inputData[i].imag() * -1.0);
		inputData[i].imag(inputData[i].imag() * hammingWindow[i]);
	}
	int i = numberOffBins % 2; // Make sure number of bins is even
	fftplan plan = fft_create_plan(numberOffBins - i, inputData.data(), fftBins.data(), type, flags);
	fft_execute(plan);
	fft_destroy_plan(plan);
	inputData.clear();
}

int FastFourier::Size()
{
	return numberOffBins;
}

void FastFourier::SetBins(int bins){
	numberOffBins = bins;
}

void FastFourier::SetInvert(bool inv)
{
	invert = inv;
}

std::vector<float> FastFourier::GetSquaredBins()
{
	std::vector<float> outputArray(numberOffBins);
	for (int i = 0; i < numberOffBins; i ++)
	{
		outputArray[i] = std::norm(fftBins.at(i));
	}
	return outputArray;
}

std::vector<float> FastFourier::GetLineatSquaredBins()
{
	int ii = numberOffBins / 2;
	bool highrange = true;
	
	std::vector<float> outputArray(numberOffBins);

	ii = numberOffBins / 2;
	for (int i = 0; i < numberOffBins/2; i++)
	{
		outputArray[ii++] = std::norm(fftBins.at(i));
	}

	ii = 0;
	for (int i = numberOffBins / 2; i < numberOffBins ; i++)
	{
		outputArray[ii++] = std::norm(fftBins.at(i));
	}
	return outputArray;
}

void FastFourier::ProcessForward(IQSampleVector &input)
{
	fftplan plan = fft_create_plan(numberOffBins, input.data(), fftBins.data(), type, flags);
	fft_execute(plan);
	fft_destroy_plan(plan);
}

void FastFourier::ProcessBackward(IQSampleVector &output)
{
	output.clear();
	output.resize(numberOffBins);
	fftplan plan = fft_create_plan(numberOffBins, fftBins.data(), output.data(), LIQUID_FFT_BACKWARD, flags);
	fft_execute(plan);
	fft_destroy_plan(plan);
}

std::vector<float> FastFourier::SpectrumPower(float offset)
{
	std::vector<float> sp;
	
	for (int i = 0; i < numberOffBins; i++)
	{
		float num = std::norm(fftBins[i]);
		sp.push_back((float)(10.0 * log10(1E-60 + (double)num)) + offset);
	}
	return sp;
}

IQSampleVector &FastFourier::GetfftBins()
{
	return fftBins;
}