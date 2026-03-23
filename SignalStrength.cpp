#include "SignalStrength.h"
#include "EnergyCalculator.h"

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