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

struct	modulator_struct
{
	double					ifrate;
	double					tuner_offset;
	liquid_ampmodem_type	mode;
	int						suppressed_carrier;
	int						pcmrate;
	double					bandwidth_pcm;
	unsigned int			downsample;
	DataBuffer<IQSample>	*source_buffer;
	AudioInput				*audio_input;
};

class AMmodulator
{
public:
	void	init(modulator_struct * ptr);
	void	set_filter(double if_rate, int band_width);
	void	process(const SampleVector& samples, double if_rate, DataBuffer<IQSample> *source_buffer);
	double get_if_level() const
	{
		return m_if_level;
	}
	
private:
	modulator_struct		m_modulator;
	mutex					m_mutex;  // used to lock the process for changing filters
	ampmodem				m_mod {0};
	iirfilt_crcf			m_lowpass {0};
	int						m_order {6};
	bool					m_bresample {false};
	long					m_resample;
	msresamp_crcf 			m_q {0};
	DataBuffer<IQSample>    *m_source_buffer = NULL;	
	double                  m_audio_mean, m_audio_rms, m_audio_level, m_if_level;
};


void start_dsb_tx(int mode, double ifrate, int pcmrate, DataBuffer<IQSample> *source_buffer, AudioInput *audio_input);