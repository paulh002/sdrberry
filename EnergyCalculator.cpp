#include "EnergyCalculator.h"

void EnergyCalculator::calculateEnergyLevel(const IQSampleVector &samples_in)
{
	float y2 = 0.0;
	for (auto &con : samples_in)
	{
		y2 += std::real(con * std::conj(con));
	}
	// smooth energy estimate using single-pole low-pass filter
	y2 = y2 / samples_in.size();
	accuf = (1.0 - alpha) * accuf + alpha * y2;
	energyLevel = accuf;
}

void EnergyCalculator::calculateEnergyLevel(const SampleVector &samples_in)
{
	float y2 = 0.0;
	for (auto &con : samples_in)
	{
		y2 += con * con;
	}
	// smooth energy estimate using single-pole low-pass filter
	y2 = y2 / samples_in.size();
	accuf = (1.0 - alpha) * accuf + alpha * y2;
	energyLevel = accuf;
}