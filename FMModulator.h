#pragma once
#include "Demodulator.h"

class FMModulator : public Demodulator
{
  public:
	FMModulator(int mode, int duplex, double ifrate, audioTone tone, std::string filename,DataBuffer<IQSample> *source_buffer, AudioInput *audio_input);
	~FMModulator();

	void process(std::span<Sample>);
	void operator()();
	static void destroy_modulator();
	static bool create_modulator(int mode, int duplex, double ifrate, audioTone tone, std::string filename, DataBuffer<IQSample> *source_buffer, AudioInput *audio_input);
	static void setLowPassAudioFilterCutOffFrequency(int bandwidth);
	std::atomic<bool> stop_flag{false};
	std::thread fmmod_thread;

  private:
	freqmod modFM{nullptr};
	bool duplex;
	bool audio_file_mode;
	std::string play_prerecorded_file;
};
