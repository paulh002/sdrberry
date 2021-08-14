#pragma once
#include <complex>
#include <vector>
#include <mutex>

typedef std::complex<float> IQSample;
typedef std::vector<IQSample> IQSampleVector;

typedef double Sample;
typedef std::vector<Sample> SampleVector;

static void to_upper(std::string& str) {
	for (auto& i : str) i = toupper(i);
};