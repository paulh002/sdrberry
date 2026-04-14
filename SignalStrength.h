#pragma once
#include <atomic>
#include "SdrberryTypeDefs.h"

class SignalStrength
{
  private:
	std::atomic<float> signal_strength{0.0};
	float signal_strength_offset{0.0};
	
  public:
	void set_signal_strength(float strength);
	void set_signal_strength_offset(float offset);
	float get_signal_strength();
	void calculateSignalStrength(const SampleVector &samples_in);
	void calculateSignalStrength(const IQSampleVector &samples_in);
};

extern SignalStrength signalstrength;