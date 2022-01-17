#pragma once
#include "Demodulator.h"

class AMModulator :
    public Demodulator
{
public:
	AMModulator(int mode, double ifrate, int pcmrate, int tone, DataBuffer<IQSample16> *source_buffer, AudioInput *audio_input);
	~AMModulator();
	
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
	std::thread				ammod_thread;

private:
	bool					m_tone;
	ampmodem 				modAM {nullptr};
	nco_crcf				m_fft {nullptr};
	void					fft_offset(long offset);
	void					mix_up_fft(const IQSampleVector& filter_in, IQSampleVector& filter_out);
};

