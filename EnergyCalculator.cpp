#include "EnergyCalculator.h"

void EnergyCalculator::calculateEnergyLevel(const IQSampleVector &samples_in)
{
	float y2{}, I2{}, Q2{};
	for (auto &con : samples_in)
	{
		y2 += std::real(con * std::conj(con));

		I2 += con.real() * con.real();
		Q2 += con.imag() * con.imag();
	}
	// smooth energy estimate using single-pole low-pass filter
	y2 = y2 / samples_in.size();

	I2 = I2 / samples_in.size();
	Q2 = Q2 / samples_in.size();

	accuf = (1.0 - alpha) * accuf + alpha * y2;
	energyLevel = accuf;
	
	
	// IQ
	accuI = (1.0 - alpha) * accuI + alpha * I2;
	energyLevelI = accuI;

	accuQ = (1.0 - alpha) * accuQ + alpha * Q2;
	energyLevelQ = accuQ;
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