#include "IQGenerator.h"
#include <iostream>

IQGenerator::IQGenerator(float _ifrate, AudioInput *audio_input)
	: ifrate(_ifrate)
{
	block_length = audio_input->getbufferFrames();
	delta_phase1 = 2.0f * M_PI * 1500.0f / _ifrate;
	delta_phase2 = 2.0f * M_PI * 750.0f / _ifrate;

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

IQSampleVector IQGenerator::generateTwoToneIQVectors(int vectors, float phaseShiftDegrees, int frequency1, int frequency2)
{
	IQSampleVector iqsamples;

	static float phase1 = 0.0f;
	static float phase2 = 0.0f;
	
	
	for (int i = 0; i < block_length; ++i)
	{
		float I_sample = (0.5f * cosf(phase1) + 0.5f * cosf(phase2)) / 10.0f;
		float Q_sample = (0.5f * sinf(phase1) + 0.5f * sinf(phase2)) / 10.0f;
		
		iqsamples.push_back(IQSample(I_sample, Q_sample)) ;
			
		phase1 += delta_phase1;
		phase2 += delta_phase2;
		if (phase1 >= 2.0f * M_PI) phase1 -= 2.0f * M_PI;
		if (phase2 >= 2.0f * M_PI) phase2 -= 2.0f * M_PI;
	}
	return iqsamples;
}