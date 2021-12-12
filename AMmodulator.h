#pragma once

#include <cstdio>
#include <cassert>
#include <cmath>
#include <complex>
#include <liquid.h>
#include <vector>
#include "DataBuffer.h"
#include "AudioOutput.h"
#include "AudioInput.h"
#include "sdrberry.h"


class AMmodulator
{
public:
	AMmodulator(int mode, double ifrate, int pcmrate, int tone, DataBuffer<IQSample16> *source_buffer, AudioInput *audio_input);
	~AMmodulator();
	void	tone(bool tone);
	void	set_filter(float samplerate, float band_width);
	void	process(const SampleVector& samples, double if_rate, DataBuffer<IQSample16> *source_buffer);
	void	operator()();
	double get_if_level() const
	{
		return m_if_level;
	}
	void	createBandpass(int txfilter);
	void	tune_offset(long offset);
	
	static void destroy_modulator();
	static bool create_modulator(int mode, double ifrate, int pcmrate, int tone, DataBuffer<IQSample16> *source_buffer, AudioInput *audio_input);
	atomic<bool>			stop_txmod_flag {false};	
	std::thread				ammod_thread;
	
private:
	mutex					m_mutex;  // used to lock the process for changing filters
	ampmodem				m_mod {0};
	iirfilt_crcf			m_lowpass {0};
	int						m_order {6};
	bool					m_bresample {false};
	long					m_resample;
	msresamp_crcf 			m_q {0};
	float					m_r;
	DataBuffer<IQSample16>    *m_source_buffer = NULL;	
	double                  m_audio_mean, m_audio_rms, m_audio_level, m_if_level;
	nco_crcf				m_upssb {nullptr}; 
	nco_crcf				m_upnco {nullptr}; 
	iirfilt_crcf			m_au_lp_q { nullptr };
	iirfilt_crcf			m_tx_lp_q { nullptr };
	long					m_offset {0};
	bool					m_tone;
	AudioInput				*m_audio_input;
	double					m_ifrate;
	int						m_pcmrate;
};
