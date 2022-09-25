#pragma once
#include <cassert>
#include <cmath>
#include <complex>
#include <cstdio>
#include <liquid.h>
#include <thread>
#include <vector>
#include "EnergyCalculator.h"
#include "Agc_class.h"
#include "AudioInput.h"
#include "AudioOutput.h"
#include "DataBuffer.h"

enum mode_enum
{
	usb,
	cw,
	lsb,
	am,
	dsb,
	ft8,
	ft4,
	broadband_fm,
	narrowband_fm,
	rtty,
	echo
};

class Demodulator
{
  public:
	static void setLowPassAudioFilterCutOffFrequency(int band_width);

	//std::thread demod_thread;
	//atomic<bool> stop_flag{false};
	
	//static void destroy_demodulator();
	//static bool create_demodulator(int mode, double ifrate, int pcmrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output);
	
	
  protected:
	~Demodulator();
	Demodulator(double ifrate, int pcmrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output);
	Demodulator(double ifrate, int pcmrate, DataBuffer<IQSample> *source_buffer, AudioInput *audio_input);
	Demodulator(int pcmrate, AudioOutput *audio_output, AudioInput *audio_input);
	void mono_to_left_right(const SampleVector &samples_mono, SampleVector &audio);
	void adjust_gain(IQSampleVector &samples_in, float vol);
	void tune_offset(long offset);
	virtual void process(const IQSampleVector &samples_in, SampleVector &audio) = 0;
	void Resample(const IQSampleVector &filter_in, IQSampleVector &filter_out);
	virtual void operator()() = 0;
	void lowPassAudioFilter(const IQSampleVector &filter_in, IQSampleVector &filter_out);
	void set_resample_rate(float resample_rate);
	void mix_down(const IQSampleVector &filter_in, IQSampleVector &filter_out);
	void mix_up(const IQSampleVector &filter_in, IQSampleVector &filter_out);
	void calc_if_level(const IQSampleVector &samples_in);
	double get_if_level() { return ifEnergy.getEnergyLevel(); }
	double get_af_level() { return afEnergy.getEnergyLevel(); }
	void set_fft_mixer(float offset);
	void setLowPassAudioFilter(float samplerate, float band_width);
	void fft_mix(int dir, const IQSampleVector &filter_in, IQSampleVector &filter_out);
	void set_fft_resample_rate(float resample_rate);
	void fft_resample(const IQSampleVector &filter_in, IQSampleVector &filter_out);
	void set_span(int span);
	void perform_fft(const IQSampleVector &iqsamples);
	void calc_af_level(const SampleVector &samples_in);
	void setBandPassFilter(float high, float mid_high, float mid_low, float low);
	void executeBandpassFilter(const IQSampleVector &filter_in, IQSampleVector &filter_out);
	void dc_filter(const IQSampleVector &filter_in, IQSampleVector &filter_out);
	int get_audioBufferSize() { return audioBufferSize; }
	bool get_dc_filter();
	int get_lowPassAudioFilterCutOffFrequency() { return lowPassAudioFilterCutOffFrequency.load(); }

	DataBuffer<IQSample> *receiveIQBuffer{nullptr};
	AudioOutput *audioOutputBuffer{nullptr};
	double ifSampleRate;
	int audioSampleRate;
	DataBuffer<IQSample> *transmitIQBuffer{nullptr};
	AudioInput *audioInputBuffer{nullptr};
	int m_span{0};

  private:
	EnergyCalculator ifEnergy, afEnergy;
	mode_enum rxTxMode;
	nco_crcf tuneNCO{nullptr};
	msresamp_crcf resampleHandle{nullptr};
	long tuneOffsetFrequency;
	float resampleRate;
	float fftResampleRate;
	msresamp_crcf fftResampleHandle{nullptr};
	nco_crcf fftNCOHandle{nullptr};
	int audioBufferSize;

	iirfilt_crcf lowPassAudioFilterHandle{nullptr};
	
	iirfilt_crcf bandPassHandle{nullptr};
	iirfilt_crcf lowPassHandle{nullptr};
	iirfilt_crcf highPassHandle{nullptr};
	firfilt_crcf dcBlockHandle{nullptr};

	const int lowPassFilterOrder = 6;
	
	const liquid_iirdes_filtertype butterwurthType{LIQUID_IIRDES_BUTTER};
	const liquid_iirdes_bandtype bandFilterType{LIQUID_IIRDES_BANDPASS};
	const liquid_iirdes_format sosFormat{LIQUID_IIRDES_SOS};

	unsigned int filterOrder; 
	float cutOffFrequency; 
	float centerFrequency; 
	float passBandRipple;
	float StopBandAttenuation;

	static atomic<int> lowPassAudioFilterCutOffFrequency;
};