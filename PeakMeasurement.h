#pragma once
#include <vector>
#include <atomic>
#include "PeakFinder.h"

class PeakMeasurement
{
  public:
	void uploadData(const std::vector<float>& data);
	void SearchTwoPeaksSupperssion();
	void PrintPeakes();
	float GetSuppression();
	std::vector<int> GetIndexToPeaks() { return IndexToPeaks; }

  private:
	std::vector<float> peakValues;
	std::vector<float> peakValuesDb;
	std::vector<int> IndexToPeaks;
	std::atomic<float> suppression{0.0};
};

