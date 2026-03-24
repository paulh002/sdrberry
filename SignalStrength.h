#pragma once
#include <atomic>
#include "SdrberryTypeDefs.h"

class SignalStrength
{
  private:
	std::atomic<double> signal_strength{0.0};
	double signal_strength_offset{0.0};
	double accuf{0};
	double alpha{0.5};
	
  public:
	void set_signal_strength(double strength);
	void set_signal_strength_offset(double offset);
	double get_signal_strength();
	void calculateSignalStrength(const SampleVector &samples_in);
	void calculateSignalStrength(const IQSampleVector &samples_in);
};

extern SignalStrength signalstrength;