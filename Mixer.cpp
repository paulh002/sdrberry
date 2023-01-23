#include "Mixer.h"

void Mixer::executeMix(const IQSampleVector &mix_in,
										   IQSampleVector &mix_out)
{
	for (auto &col : mix_in)
	{
		std::complex<float> v;

		v = NumericOscilator();
		v = v * col;
		mix_out.insert(mix_out.end(), v);
	}
}

void Mixer::setup(float SampleRate, float MixerFrequency){
	Mixer::SampleRate = SampleRate;
	Mixer::MixerFrequency = MixerFrequency;

	Steps = 0;
}

std::complex<float> Mixer::NumericOscilator()
{
	std::complex<float> v;

	double angle = Steps * MixerFrequency * (2 * M_PI / SampleRate);
	if (++Steps >= SampleRate)
		Steps = 0;
	v.real(cos(angle));
	v.imag(sin(angle));
	return v;
}