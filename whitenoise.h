#pragma once
#include <random>
#include <complex>
#include <vector>

class WhiteNoiseGenerator {
private:
	std::mt19937 rng; // Mersenne Twister random number generator
	std::normal_distribution<float> dist; // Gaussian distribution (mean=0, stddev=1)
	float amplitude; // Scaling factor to control volume/level

public:
	// Constructor
	// seed: Optional seed for reproducibility. Defaults to random hardware seed.
	// amp: Amplitude scaling. Keep this low (e.g., 0.1) to prevent clipping!
	WhiteNoiseGenerator(float amp = 0.1f, unsigned int seed = std::random_device { }()) 
		: rng(seed)
		, dist(0.0f, 1.0f)
		, amplitude(amp) {
	}

	// Generate a single real audio sample
	float generateSample() {
		return dist(rng) * amplitude;
	}

	// Fill an existing float buffer with white noise
	void generateBuffer(float* buffer, size_t num_samples) {
		for (size_t i = 0; i < num_samples; ++i) {
			buffer[i] = dist(rng) * amplitude;
		}
	}

	// Generate a complex (IQ) sample for SDR baseband
	// I and Q are independent Gaussian noise sources
	std::complex<float> generateComplexSample() {
		float i = dist(rng) * amplitude;
		float q = dist(rng) * amplitude;
		return std::complex<float>(i, q);
	}

	// Fill a complex (IQ) buffer
	void generateComplexBuffer(std::complex<float>* buffer, size_t num_samples) {
		for (size_t i = 0; i < num_samples; ++i) {
			buffer[i] = generateComplexSample();
		}
	}

	// Adjust amplitude on the fly
	void setAmplitude(float amp) {
		amplitude = amp;
	}
};

