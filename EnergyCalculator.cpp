#include "EnergyCalculator.h"
#include <cmath>
#include <tuple>

float sign(float x)
{
	if (x < 0)
		return -1.0f;
	else if (x > 0)
		return 1.0f;
	else
		return 0.0f;
}

// Moseley, N.A. & C.H. Slump (2006): A low-complexity feed-forward I/Q imbalance compensation algorithm.
// in 17th Annual Workshop on Circuits, Nov. 2006, pp. 158–164.
// http://doc.utwente.nl/66726/1/moseley.pdf
// Implementation from DD4WH/Teensy-ConvolutionSDR
	
void EnergyCalculator::calculateEnergyLevel(const IQSampleVector &samples_in)
{
	float y2{ 0 }, I2{ 0 }, Q2{ 0 }, imag{ 0 }, real{ 0 };
	double Max{0.0}, Correlation{0.0}, CorrelationNorm{0.0};
	teta1 = 0.0;
	teta2 = 0.0;
	teta3 = 0.0;

	for (auto con : samples_in)
	{
		y2 += std::real(con * std::conj(con));

		I2 += con.real() * con.real();
		Q2 += con.imag() * con.imag();
		Correlation += con.real() * con.imag();

		teta1 += sign(con.imag()) * con.real(); // eq (34)
		teta2 += sign(con.imag()) * con.imag(); // eq (35)
		teta3 += sign(con.real()) * con.real(); // eq (36)
	}

	teta1 = (1.0 - (double)aplha_mosely) * teta1_old + (double)aplha_mosely * teta1 * -1.0; // eq (34) and first order lowpass
	teta2 = (1.0 - (double)aplha_mosely) * teta2_old + (double)aplha_mosely * teta2 * 1.0; // eq (35) and first order lowpass
	teta3 = (1.0 - (double)aplha_mosely) * teta3_old + (double)aplha_mosely * teta3 * 1.0; // eq (36) and first order lowpass
	calculateMoseleyIQ();
	
	// Correlation is the integral of real * imag of samples
	// Normalized devided by square root of energy I2 * Q2
	CorrelationNorm = Correlation / sqrt(I2 * Q2);
	//printf("Phase %f I %f Q %f size %d\n", Angle, real, imag, samples_in.size());

	y2 = y2 / samples_in.size();
	I2 = I2 / samples_in.size();
	Q2 = Q2 / samples_in.size();

	// smooth energy estimate using single-pole low-pass filter
	accuf = (1.0 - alpha) * accuf + alpha * y2;
	energyLevel = accuf;

	// IQ
	accuI = (1.0 - alpha) * accuI + alpha * I2;
	energyLevelI = accuI;

	accuQ = (1.0 - alpha) * accuQ + alpha * Q2;
	energyLevelQ = accuQ;

	accuCorrelation = (1.0 - (double)alpha) * accuCorrelation + (double)alpha * Correlation;
	energyCorrelation = accuCorrelation;

	accuCorrelationNorm = (1.0 - (double)alpha) * accuCorrelationNorm + (double)alpha * CorrelationNorm;
	energyCorrelationNorm = accuCorrelationNorm;
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

void EnergyCalculator::calculateMoseleyIQ()
{

	if (teta2 != 0.0) // prevent divide-by-zero
	{
		M_c1 = teta1 / teta2; // eq (30)
	}
	else
	{
		M_c1 = 0.0f;
	}

	float moseley_help = (teta2 * teta2);
	if (moseley_help > 0.0f) // prevent divide-by-zero
	{
		moseley_help = (teta3 * teta3 - teta1 * teta1) / moseley_help; // eq (31)
	}
	if (moseley_help > 0.0f) // prevent sqrtf of negative value
	{
		M_c2 = sqrtf(moseley_help); // eq (31)
	}
	else
	{
		M_c2 = 1.0f;
	}
	if (teta3 != 0.0f)
	{
		phase_IQ = asinf(teta1 / teta3) * 180 / M_PI;
	}
	
	teta1_old = teta1;
	teta2_old = teta2;
	teta3_old = teta3;
}

std::tuple<float, float, float> EnergyCalculator::ResultsMoseleyIQ()
{
	std::tuple<float, float, float> retval(phase_IQ, M_c1, M_c2);

	return retval;
}