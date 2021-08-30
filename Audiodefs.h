#pragma once
#include <complex>
#include <vector>
#include <mutex>

typedef std::complex<float> IQSample;
typedef std::vector<IQSample> IQSampleVector;

typedef std::complex<int16_t> IQSample16;
typedef std::vector<IQSample16> IQSampleVector16;

typedef double Sample;
typedef std::vector<Sample> SampleVector;

static void to_upper(std::string& str) {
	for (auto& i : str) i = toupper(i);
};