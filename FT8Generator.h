#pragma once
#include <string>
#include "common.h"
#include "wave.h"
#include "debug.h"
#include "pack.h"
#include "encode.h"
#include "constants.h"
#include "SdrberryTypeDefs.h"
#include "DataBuffer.h"
#include "Settings.h"
#include "AudioInput.h"

#define LOG_LEVEL LOG_INFO

#define FT8_SYMBOL_BT 2.0f ///< symbol smoothing filter bandwidth factor (BT)
#define FT4_SYMBOL_BT 1.0f ///< symbol smoothing filter bandwidth factor (BT)

#define GFSK_CONST_K 5.336446f ///< == pi * sqrt(2 / log(2))

enum encoding
{
	FT8, FT4
};

class FT8Generator
{
  public:
	FT8Generator(AudioInput *input) { audioinput = input; }
	vector<float> generate(int frequency, encoding code,std::string message);

  private:
	SampleVector audioSamples;
	AudioInput *audioinput;
	void gfsk_pulse(int n_spsym, float symbol_bt, float *pulse);
	void synth_gfsk(const uint8_t *symbols, int n_sym, float f0, float symbol_bt, float symbol_period, int signal_rate, float *signal);
};
