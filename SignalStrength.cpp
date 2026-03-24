#include "SignalStrength.h"

SignalStrength signalstrength;

void SignalStrength::set_signal_strength(double strength)
{
	signal_strength = 20 * log10(strength) + signal_strength_offset;
}

void SignalStrength::set_signal_strength_offset(double offset)
{
	signal_strength_offset = offset;
}

double SignalStrength::get_signal_strength()
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
	accuf = (1.0 - alpha) * accuf + alpha * y2;
	signal_strength = 20 * log10(accuf) + signal_strength_offset;
}

void SignalStrength::calculateSignalStrength(const IQSampleVector &samples_in)
{
	double y2 = 0.0;
	
	for (auto con : samples_in)
	{
		y2 += std::real(con * std::conj(con));
	}
	y2 = y2 / samples_in.size();
	accuf = (1.0 - alpha) * accuf + alpha * y2;
	signal_strength = 20 * log10(accuf) + signal_strength_offset;
}