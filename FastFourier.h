#pragma once
#include "SdrberryTypeDefs.h"
#include <atomic>
#include <cmath>
#include <complex>
#include <fftw3.h>
#include <liquid/liquid.h>
#include <vector>

class FastFourier
{
  public:
	FastFourier(int nbins, float rs);
	~FastFourier();
	void Process(const IQSampleVector &input);
	int Size();
	void SetBins(int bins);
	std::vector<float> GetSquaredBins();
	void SetInvert(bool inv);

  private:
	const int type = LIQUID_FFT_FORWARD;
	int flags = 0; // FFT flags (typically ignored)
	int numberOffBins;
	std::vector<std::complex<float>> fftBins;
	std::vector<float> hammingWindow;
	std::vector<std::complex<float>> inputData;
	msresamp_crcf resampleHandle;
	bool invert{false};
	float resampleRate{};
};
