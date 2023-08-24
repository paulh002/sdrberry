#pragma once
#include <vector>
#include <thread>
#include "DataBuffer.h"
#include "SdrberryTypeDefs.h"
#include "AudioOutput.h"
#include "AudioInput.h"

struct ModulatorParameters
{
	int mode;
	double ifrate;
	audioTone tone;
	bool even;
	int timeslotTensofSec;
	std::vector<float> signal;
};

class DigitalTransmission
{
  public:
	DigitalTransmission(DataBuffer<IQSample> *source_buffer_tx, DataBuffer<IQSample> *source_buffer_rx, AudioInput *audio_input);
	void operator()();
	std::thread DTthread;

	static void WaitForTimeSlot();
	static void StartDigitalTransmission(ModulatorParameters param);
	static bool CancelDigitalTransmission();

  private:
	DataBuffer<IQSample> *Source_buffer_rx;
};