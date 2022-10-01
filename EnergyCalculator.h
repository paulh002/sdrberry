#pragma once
#include "SdrberryTypeDefs.h"

class EnergyCalculator
{
  public:
	void calculateEnergyLevel(const IQSampleVector &samples_in);
	void calculateEnergyLevel(const SampleVector &samples_in);
	
	float getEnergyLevel() { return energyLevel; }

  private:
	float alpha{0.1};
	float accuf{0};
	float energyLevel{};
};

