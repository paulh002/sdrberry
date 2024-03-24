#pragma once
#include "SdrberryTypeDefs.h"
#include <atomic>
#include <cmath>
#include <complex>
#include <fftw3.h>
#include <liquid/liquid.h>
#include <vector>
#include <mutex>

class FastFourier
{
  public:
	FastFourier(int nbins, float rs, float downMixFrequency);
	~FastFourier();
	void Process(const IQSampleVector &input);
	int Size();
	void SetBins(int bins);
	std::vector<float> GetSquaredBins();
	std::vector<float> GetLineatSquaredBins();
	void SetInvert(bool inv);
	void SetParameters(int nbins, float resampleRate = 0.0f, float downMixFrequency = 0.0f);

  private:
	const int type = LIQUID_FFT_FORWARD;
	int flags = 0; // FFT flags (typically ignored)
	int numberOffBins;
	std::vector<std::complex<float>> fftBins;
	std::vector<float> hammingWindow;
	std::vector<std::complex<float>> inputData;
	msresamp_crcf resampleHandle{nullptr};
	bool invert{false};
	float resampleRate{};
	nco_crcf ncoMixerHandle{nullptr};
	std::mutex mutexSingleEntry;
};
