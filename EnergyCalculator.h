#pragma once
#include "SdrberryTypeDefs.h"

class EnergyCalculator
{
  public:
	void calculateEnergyLevel(const IQSampleVector &samples_in);
	void calculateEnergyLevel(const SampleVector &samples_in);
	
	float getEnergyLevel() { return energyLevel; }
	float getEnergyLevelI() { return energyLevelI; }
	float getEnergyLevelQ() { return energyLevelQ; }

  private:
	float alpha{0.1};
	float accuf{0};
	float energyLevel{};
	float energyLevelI{};
	float accuI{0};
	float energyLevelQ{};
	float accuQ{0};
};

