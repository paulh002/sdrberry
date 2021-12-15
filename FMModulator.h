#pragma once
#include "Demodulator.h"

class FMModulator :
    public Demodulator
{
public:
	FMModulator(int mode, double ifrate, int pcmrate, int tone, DataBuffer<IQSample16> *source_buffer, AudioInput *audio_input);
	~FMModulator();
	
	void	process(const IQSampleVector& samples_in, SampleVector& samples);
	void	operator()();
	void	tone(bool tone);
	double get_if_level() const
	{
		return m_if_level;
	}
	static void destroy_modulator();
	static bool create_modulator(int mode, double ifrate, int pcmrate, int tone, DataBuffer<IQSample16> *source_buffer, AudioInput *audio_input);
	atomic<bool>			stop_flag {false};	
	std::thread				fmmod_thread;

private:
	bool					m_tone;
	freqmod					modFM {nullptr};
};

