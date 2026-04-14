#include "SignalStrength.h"

SignalStrength signalstrength;

void SignalStrength::set_signal_strength(float strength)
{
	signal_strength = 20 * log10f(strength) + signal_strength_offset;
}

void SignalStrength::set_signal_strength_offset(float offset)
{
	signal_strength_offset = offset;
}

float SignalStrength::get_signal_strength()
{
	return signal_strength;
}

void SignalStrength::calculateSignalStrength(const SampleVector &samples_in)
{
	double y2 = 0.0;
	for (auto &con : samples_in)
	{
		y2 += con * con;
	}
	// smooth energy estimate using single-pole low-pass filter
	y2 = y2 / samples_in.size();
	signal_strength = 20 * log10f(y2) + signal_strength_offset;
}

void SignalStrength::calculateSignalStrength(const IQSampleVector &samples_in)
{
	float y2 = 0.0;
	
	for (auto con : samples_in)
	{
		y2 += std::norm(con);
	}
	y2 = y2 / samples_in.size();
	signal_strength = 20 * log10(y2) + signal_strength_offset;
}