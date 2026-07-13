#pragma once
#include "DataBuffer.h"
#include "SdrberryTypeDefs.h"
#include "AudioInput.h"

class IQGenerator
{
  public:
	IQGenerator(float ifrate, AudioInput *audio_input);
	IQSampleVector generateIQVectors(int vectors, float phaseShiftDegrees, int frequency);
	IQSampleVector generateTwoToneIQVectors(int vectors, float phaseShiftDegrees, int frequency1, int frequency2);

  private:
	int block_length;
	float ifrate;
	float delta_phase1;
	float delta_phase2;
};

