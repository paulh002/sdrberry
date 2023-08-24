#include "IQGenerator.h"
#include <iostream>

IQGenerator::IQGenerator(double _ifrate, AudioInput *audio_input)
	: ifrate(_ifrate)
{
	block_length = ifrate / audio_input->get_samplerate() * audio_input->getbufferFrames();
}

IQSampleVector IQGenerator::generateIQVectors(int vectors, float phaseShiftDegrees, int frequency)
{
	IQSampleVector iqsamples;

	float ShiftDegrees = 360.0f / vectors; // space between vectors
	float ShiftRadians = ShiftDegrees * M_PI / 180.0f; // start point first vector
	float phaseShiftRadians = phaseShiftDegrees * M_PI / 180.0f; // start point first vector
	float I, Q;
	//vectors = 1;
	
	for (int i = 0; i < block_length; ++i)
	{
		I = Q = 0.0f;
		float time = (static_cast<float>(i) / ifrate);
		for (int ii = 0; ii < vectors; ++ii)
		{
			I += cos(2 * M_PI * frequency * time + ShiftRadians  + phaseShiftRadians * ii);
			Q += sin(2 * M_PI * frequency * time + ShiftRadians  + phaseShiftRadians * ii);
		}
		iqsamples.push_back(IQSample(I / (float)vectors, Q / (float)vectors));
	}
	return iqsamples;
}