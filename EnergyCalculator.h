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
	double getEnergyCorrelation() { return energyCorrelation; }
	double getEnergyCorrelationNorm() { return energyCorrelationNorm; }
	std::tuple<float, float, float> ResultsMoseleyIQ();

  private:
	float alpha{0.1}, aplha_mosely{0.01};
	float accuf{0};
	float energyLevel{0};
	float energyLevelI{0};
	float accuI{0};
	float energyLevelQ{0};
	float accuQ{0};
	float phase{90};
	double accuCorrelation{0.0};
	double accuCorrelationNorm{0.0};
	double energyCorrelation{0.0};
	double energyCorrelationNorm{0.0};
	float teta1{0.0}, teta1_old{0.0}, teta2{0.0}, teta2_old{0.0}, teta3{0.0}, teta3_old{0.0};
	float M_c1{0.0};
	float M_c2{0.0};
	float phase_IQ{0.0};

	void calculateMoseleyIQ();
};

