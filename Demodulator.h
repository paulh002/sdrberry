#pragma once
#include <cstdio>
#include <cassert>
#include <cmath>
#include <complex>
#include <thread>
#include <liquid.h>
#include <vector>
#include "DataBuffer.h"
#include "AudioOutput.h"
#include "AudioInput.h"
#include "Agc_class.h"

class Demodulator
{
public:
	void			set_filter(int band_width);
	
protected:
	~Demodulator();
	Demodulator(double ifrate, int pcmrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output);
	Demodulator(double ifrate, int pcmrate, DataBuffer<IQSample16> *source_buffer, AudioInput *audio_input);
	void			mono_to_left_right(const SampleVector& samples_mono, SampleVector& audio);
	void			adjust_gain(IQSampleVector& samples_in, float vol);
	void			tune_offset(long offset);
	virtual void	process(const IQSampleVector& samples_in, SampleVector& audio) = 0;
	void			Resample(const IQSampleVector& filter_in, IQSampleVector& filter_out);
	virtual void	operator()() = 0;
	void			filter(const IQSampleVector& filter_in, IQSampleVector& filter_out);
	void			set_reample_rate(float resample_rate);
	void			mix_down(const IQSampleVector& filter_in,IQSampleVector& filter_out);
	void			mix_up(const IQSampleVector& filter_in, IQSampleVector& filter_out);
	void			calc_if_level(const IQSampleVector& samples_in);
	double			get_if_level() 	{return m_if_level;}
	void			set_fft_mixer(float offset);
	void			set_filter(float samplerate, float band_width);
	void			fft_mix(int dir, const IQSampleVector& filter_in, IQSampleVector& filter_out);
	void			set_fft_reample_rate(float resample_rate);
	void			fft_resample(const IQSampleVector& filter_in, IQSampleVector& filter_out);
	void			set_span(int span);
	void			perform_fft(const IQSampleVector& iqsamples);
	
	double                      m_audio_mean, m_audio_rms, m_audio_level;
	DataBuffer<IQSample>		*m_source_buffer {nullptr};
	AudioOutput					*m_audio_output {nullptr};
	double						m_ifrate;
	int							m_pcmrate;
	atomic<int>					m_fcutoff;
	DataBuffer<IQSample16>		*m_transmit_buffer {nullptr};
	AudioInput					*m_audio_input {nullptr};
	double						m_if_level {0};
	int							m_span {0};
	
private:
	nco_crcf					m_upnco {nullptr};
	msresamp_crcf 				m_q {nullptr};
	long						m_offset;
	float						m_resample_rate;
	float						m_fft_resample_rate;
	iirfilt_crcf				m_lowpass {nullptr};
	msresamp_crcf 				m_fft_resample {nullptr};
	nco_crcf					m_fftmix {nullptr};
	int							m_order {6};
	float						alpha {0.1};
	float						accuf {0};
};