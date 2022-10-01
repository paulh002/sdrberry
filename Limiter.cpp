#include "Limiter.h"

EnvelopeFollower::EnvelopeFollower()
{
	envelope = 0;
}

void EnvelopeFollower::Setup(double attackMs, double releaseMs, double sampleRate)
{
	a = pow(0.01, 1.0 / (attackMs * sampleRate * 0.001));
	r = pow(0.01, 1.0 / (releaseMs * sampleRate * 0.001));
}

void EnvelopeFollower::Process(const IQSample &src)
{
	double v = sqrt(std::real(src * std::conj(src)));
	if (v > envelope)
		envelope = a * (envelope - v) + v;
	else
		envelope = r * (envelope - v) + v;
}

//----------
Limiter::Limiter(double attackMs, double releaseMs, double sampleRate)
{
	e.Setup(attackMs, releaseMs, sampleRate);
}

void Limiter::Setup(double attackMs, double releaseMs, double sampleRate)
{
	e.Setup(attackMs, releaseMs, sampleRate);
}

void Limiter::Process(IQSampleVector &dest)
{
	for (auto &con : dest)
	{
		e.Process(con);
		if (e.envelope > 1)
			con = con / e.envelope;
	}
}