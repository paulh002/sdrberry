#pragma once
#include "DataBuffer.h"
#include "SdrberryTypeDefs.h"
#include "AudioInput.h"

class IQGenerator
{
  public:
	IQGenerator(double ifrate, AudioInput *audio_input);
	IQSampleVector generateIQVectors(int vectors, float phaseShiftDegrees, int space);

  private:
	int block_length;
	double ifrate;
};

