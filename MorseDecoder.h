#pragma once
#include <string>
#include <cstdio>
#include <cassert>
#include <cmath>
#include <complex>
#include <liquid.h>
#include <vector>
#include <time.h>
#include <sys/time.h>
#include <chrono>
#include "Audiodefs.h"
#include "DataBuffer.h"

#define HIGH 1
#define LOW 0
#define num_chars 14

class MorseDecoder
{
  public:
	MorseDecoder(float ifrate);
	~MorseDecoder();
	void decode(const IQSampleVector &samples_in);
	void read(std::string &message);

  private:
	float magnitudelimit;
	float magnitudelimit_low{0.000244140625};
	iirfilt_crcf q;

	int realstate = LOW;
	int realstatebefore = LOW;
	int filteredstate = LOW;
	int filteredstatebefore = LOW;
	int stop = LOW;

	// Noise Blanker time which shall be computed so this is initial
	int nbtime{6000}; /// micro seconds noise blanker
	int wpm{0};

	std::chrono::high_resolution_clock::time_point starttimehigh{};
	std::chrono::microseconds highduration;
	std::chrono::microseconds lasthighduration;
	std::chrono::microseconds hightimesavg;
	std::chrono::high_resolution_clock::time_point startttimelow{};
	std::chrono::microseconds lowduration;
	std::chrono::high_resolution_clock::time_point laststarttime{};

	std::string DisplayLine;
	char CodeBuffer[num_chars];

	void CodeToChar();
	void AddCharacter(char newchar);
};

