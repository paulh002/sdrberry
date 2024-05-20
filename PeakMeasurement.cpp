#include "PeakMeasurement.h"
#include <cmath>
#include <algorithm>
#include <cstdio>

void PeakMeasurement::uploadData(std::vector<float> data)
{
	IndexToPeaks.clear();
	PeakFinder::findPeaks(data, IndexToPeaks, false);
	peakValues.clear();
	for (auto index : IndexToPeaks)
	{
		peakValues.push_back(data.at(index - 1));
	}
	SearchTwoPeaksSupperssion();
	if (peakValuesDb.size() == 2)
	{
		suppression = peakValuesDb.at(1) - peakValuesDb.at(0);
	}
	if (peakValuesDb.size() > 2)
		suppression = peakValuesDb.at(peakValuesDb.size() - 1) - peakValuesDb.at(peakValuesDb.size() - 2);

	if (peakValuesDb.size() == 1)
		suppression = 100.0;
	//PrintPeakes();
}

void PeakMeasurement::SearchTwoPeaksSupperssion()
{
	peakValuesDb.clear();
	std::sort(peakValues.begin(), peakValues.end());
	for (auto value : peakValues)
	{
		peakValuesDb.push_back(20.0 * log10(value));
	}
}

void PeakMeasurement::PrintPeakes()
{
	printf("\r");
	//for (auto value : peakValuesDb)
	//{
	//	printf("%2.2f db ", value);
	//}
	printf(" suppression %f db      ", suppression.load());
	fflush(stdout);
}

float PeakMeasurement::GetSuppression()
{
	return suppression;
}
