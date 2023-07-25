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
	if (resampleRate > 0.0)
		resampleHandle = msresamp_crcf_create(resampleRate, As);
}

FastFourier::~FastFourier()
{
	if (resampleHandle)
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
	if (resampleHandle)
	{
		msresamp_crcf_execute(resampleHandle, (std::complex<float> *)inputData.data(), inputData.size(), (std::complex<float> *)inputData.data(), &num_written);
		inputData.resize(num_written);
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