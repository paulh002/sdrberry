#pragma once
#include <atomic>

class SignalStrength
{
  private:
	std::atomic<double> signal_strength{0.0};
	double signal_strength_offset{0.0};

  public:
	void set_signal_strength(double strength);
	void set_signal_strength_offset(double offset);
	double get_signal_strength();
};

extern SignalStrength signalstrength;