#include "DataBuffer.h"
#include "Audiodefs.h"
#include "PeakLevelDetector.h"


PeakLevelDetector::PeakLevelDetector(float sampleRate)
{
	setDetector(sampleRate);
}

PeakLevelDetector::~PeakLevelDetector() {}

float PeakLevelDetector::tick(float inputSample)
{
	inputAbs = fabs(inputSample);

	if (inputAbs > peakOutput)
	{
		b0 = b0Attack;
	}
	else
	{
		b0 = b0Release;
	}

	// Simplified filter equation (out = b0 * input + a1 * lastOut)
	peakOutput += b0 * (inputAbs - peakOutput);

	return peakOutput;
}

void PeakLevelDetector::setDetector(float sampleRate)
{
	fs = sampleRate;
	peakOutput = 0.f;

	// set coefficients for leaky integrator
	b0Attack = 1.f;
	a1 = expf(-1 / (releaseTime * fs));
	b0Release = 1.f - a1;
}

// Times are in seconds (e.g. 100ms = 0.1f, 1.2s = 1.2f)
GainDynamics::GainDynamics(float sampleRate, float newAttackTime, float newReleaseTime)
{
	attackTime = newAttackTime;
	releaseTime = newReleaseTime;
	setDetector(sampleRate);
}

GainDynamics::~GainDynamics() {}

float GainDynamics::tick(float inputGain)
{
	if (inputGain < outputGain)
	{ // Isn't this suppose to be (input > lastOutput)?
		b0 = b0Attack;
	}
	else
	{
		b0 = b0Release;
	}

	// Simplified filter equation (out = b0 * input + a1 * lastOut)
	outputGain += b0 * (inputGain - outputGain);
	return outputGain;
}

void GainDynamics::setDetector(float sampleRate)
{
	fs = sampleRate;
	outputGain = 0.f;
	setAttack(attackTime);
	setRelease(releaseTime);
}

void GainDynamics::setAttack(float newAttackTime)
{
	attackTime = newAttackTime;
	b0Attack = 1. - expf(-1. / (attackTime * fs));
}

void GainDynamics::setRelease(float newReleaseTime)
{
	releaseTime = newReleaseTime;
	b0Release = 1. - expf(-1. / (releaseTime * fs));
}

AudioProcessor::AudioProcessor()
{
	thresholdDb = DEFAULT_THRESHOLD;
	ratio = DEFAULT_RATIO;
	attackTime = DEFAULT_ATTACK;
	releaseTime = DEFAULT_RELEASE;
}

void AudioProcessor::prepareToPlay(double sampleRate)
{
	// Use this method as the place to do any pre-playback
	// initialisation that you need..
	fs = sampleRate;
	gain = 1.f;

	if (LevelDetector == nullptr)
	{
		LevelDetector = make_unique<PeakLevelDetector>(sampleRate);
	}
	else
	{
		LevelDetector->setDetector(sampleRate);
	}

	if (gainDymanics == nullptr)
	{
		gainDymanics = make_unique<GainDynamics>(sampleRate, attackTime, releaseTime);
	}
	else
	{
		gainDymanics->setDetector(sampleRate);
	}
}

void AudioProcessor::processBlock(SampleVector &buffer)
{

	// This is the place where you'd normally do the guts of your plugin's
	// audio processing...
	for (int i = 0; i < buffer.size(); i++)
	{
		// Peak detector
		peakOut = LevelDetector->tick(buffer[i]);

		// Convert to db
		peakSumDb = dB(peakOut);

		// Calculate gain
		if (peakSumDb < thresholdDb)
		{
			gainDb = 0.f;
		}
		else
		{
			gainDb = -(peakSumDb - thresholdDb) * (1.f - 1.f / ratio);
		}

		// Gain dynamics (attack and release)
		gainDb = gainDymanics->tick(gainDb);

		// Convert to Linear
		gain = dB2mag(gainDb);

		// Apply gain
		buffer[i] *= gain;
	}
}