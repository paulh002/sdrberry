#pragma once
#include <cmath>
#include <string>
#include "RtAudio.h"
#include "SdrberryTypeDefs.h"
#include "DataBuffer.h"

const float DEFAULT_THRESHOLD = 0.f;
const float DEFAULT_RATIO = 1.f;
const float DEFAULT_ATTACK = 0.020f;  // seconds
const float DEFAULT_RELEASE = 0.500f; // seconds

#define dB(x) 20.0 * ((x) > 0.00001 ? log10(x) : -5.0)
#define dB2mag(x) pow(10.0, (x) / 20.0)

class PeakLevelDetector
{
  public:
	PeakLevelDetector(float sampleRate);
	~PeakLevelDetector();

	float tick(float inputSample);
	void setDetector(float sampleRate);

  private:
	float fs, inputAbs, peakOutput;
	float b0Attack, b0Release, b0, a1;
	float releaseTime = 0.100f; // seconds
};

class GainDynamics
{
  public:
	GainDynamics(float sampleRate, float attackTime, float releaseTime);
	~GainDynamics();

	float tick(float inputSample);
	void setDetector(float sampleRate);
	void setAttack(float attackTime);
	void setRelease(float releaseTime);

  private:
	float fs, outputGain;
	double b0Attack, b0Release, b0;
	float attackTime, releaseTime; // in seconds
};

//thresshold  - 20, 0, 0.01
// setRange 1, 10, 0.01;

class AudioProcessor
{
  public:
	AudioProcessor();
	void processBlock(SampleVector &buffer);
	void prepareToPlay(double sampleRate);
	void setThresholdDB(float threshold) { thresholdDb = threshold; }
	void setRatio(float Ratio) { ratio = Ratio; }
	void setAtack(float Attack) { attackTime = Attack; }
	void setRelease(float Release) { releaseTime = Release; }
	float getGain() { return gainDb; }
	float getPeak() { return peakSumDb; }
	float getRatio() { return ratio; }
	float getRelease() { return releaseTime; }
	float getAtack() { return attackTime; }
	float getThreshold() { return thresholdDb; }
	
		
  private:
	float ratio;
	float attackTime;
	float releaseTime;
	float fs;
	float gain{1.f};
	float peakOut;
	float peakSum;
	float peakSumDb;
	float gainDb;
	float thresholdDb;
	
	std::unique_ptr<PeakLevelDetector> LevelDetector;
	std::unique_ptr<GainDynamics> gainDymanics;
};