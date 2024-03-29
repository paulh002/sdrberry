#pragma once
#include <cmath>
#include <complex>
#include "SdrberryTypeDefs.h"

class EnvelopeFollower
{
  public:
	EnvelopeFollower();

	void Setup(double attackMs, double releaseMs, double sampleRate);
	void Process(const IQSample &src);
	float getEnvelope() { return envelope; }
	
	float envelope;

  protected:
	float a;
	float r;
};

//----------



//----------

struct Limiter
{
  public:
	Limiter(double attackMs, double releaseMs, double sampleRate);
	void Setup(double attackMs, double releaseMs, double sampleRate);
	void Process(IQSampleVector &dest);
	float getEnvelope() { return e.getEnvelope(); }
	static const std::string getsetting() { return "Radio"; }

  private:
	EnvelopeFollower e;
};


