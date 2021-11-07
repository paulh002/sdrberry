#pragma once
#include <cstdio>
#include <cassert>
#include <cmath>
#include <complex>
#include <liquid.h>
#include <vector>
#include "DataBuffer.h"
#include "AudioOutput.h"
#include "Agc_class.h"

/*
type = LIQUID_AMPMODEM_DSB;
type = LIQUID_AMPMODEM_USB;
type = LIQUID_AMPMODEM_LSB;
*/
	

struct	demod_struct
{
	double					ifrate;
	double					tuner_offset;
	liquid_ampmodem_type	mode;
	int						suppressed_carrier;
	int						pcmrate;
	double					bandwidth_pcm;
	unsigned int			downsample;
	DataBuffer<IQSample>	*source_buffer;
	AudioOutput				*audio_output;
	bool					stereo;
};
	

class AMDemodulator
{
public:
	void	init(demod_struct * ptr);
	void	set_filter(double if_rate, int band_width);
	double	get_if_level()
	{
		return m_if_level;
	}
	void	calc_if_level(const IQSampleVector& samples_in);
	void	process(const IQSampleVector& samples_in, SampleVector& audio);
	~AMDemodulator();
	void	mono_to_left_right(const SampleVector& samples_mono, SampleVector& audio);
	void	adjust_gain(IQSampleVector& samples_in, float vol);
	void	tune_offset(long offset);
	void	exit_demod();
	
	ampmodem	get_am_demod()
	{
		return m_demod;
	}
	
	DataBuffer<IQSample>        *m_source_buffer = NULL;	
	double                      m_audio_mean, m_audio_rms, m_audio_level;

private:	
	bool						m_init = false;
	bool						m_bresample = false;
	ampmodem					m_demod {0};
	msresamp_crcf 				m_q {0};
	iirfilt_crcf				m_lowpass {0};
	double						m_if_level {0};
	mutex						m_mutex; // used to lock the process for changing filters
	condition_variable			m_cond;
	int							m_order {6};
	float						m_r;
	long						m_offset;
	nco_crcf					m_upnco {nullptr};
	Agc_class					agc;
	int							m_iagc = 0;
	float						alpha {0.1};
	float						accuf {0};
};


int	create_am_thread(demod_struct *demod);
void start_dsb(int mode, double ifrate, int pcmrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output);