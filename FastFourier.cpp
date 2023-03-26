#include "FastFourier.h"

FastFourier::FastFourier(int nbins, float rs)
	: numberOffBins(nbins), resampleRate(rs)
{
	float As{60.0f};
	
	fftBins.resize(numberOffBins);
	hammingWindow.clear();
	for (int i = 0; i < numberOffBins; i++)
	{
		hammingWindow.insert(hammingWindow.end(), liquid_windowf(LIQUID_WINDOW_HAMMING, i, numberOffBins, 0));
	}
	resampleHandle = msresamp_crcf_create(resampleRate, As);
}

FastFourier::~FastFourier()
{
	msresamp_crcf_destroy(resampleHandle);
}

void FastFourier::Process(const IQSampleVector &input)
{
	unsigned int num_written;

	if (input.size() < numberOffBins)
	{
		inputData.insert(inputData.end(), input.begin(), input.end());
		if (inputData.size() < numberOffBins)
			return;
	}
	else
		inputData.insert(inputData.end(), input.begin(), input.end());
	msresamp_crcf_execute(resampleHandle, (std::complex<float> *)inputData.data(), inputData.size(), (std::complex<float> *)inputData.data(), &num_written);
	inputData.resize(num_written);

	for (int i = 0; i < numberOffBins; i++)
	{
		inputData[i].real(inputData[i].real() * hammingWindow[i]);
		if (invert)
			inputData[i].imag(inputData[i].imag() * -1.0);
		inputData[i].imag(inputData[i].imag() * hammingWindow[i]);
	}
	fftplan plan = fft_create_plan(numberOffBins, inputData.data(), fftBins.data(), type, flags);
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
		std::complex<float> f;
	
		f = std::conj(fftBins.at(i)) * fftBins.at(i);
		outputArray[i] = f.real();
	}
	return outputArray;
}
