#pragma once
#include <cassert>
#include <cmath>
#include <complex>
#include <cstdio>
#include <liquid/liquid.h>
#include <thread>
#include <vector>
#include "AudioInput.h"
#include "AudioOutput.h"
#include "EnergyCalculator.h"
#include "Agc_class.h"
#include "DataBuffer.h"
#include "SharedQueue.h"
#include "NoiseFilter.h"
#include "AgcProcessor.h"

extern atomic<double> correlationMeasurement, errorMeasurement;

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
	static void set_dc_filter(bool state);
	static void set_autocorrection(int state);
	static void set_noise_filter(int noise);
	static void set_noise_threshold(int threshold);
	static float get_threshold() { return noiseThresshold; }
	static void set_filter_type(int type);
	static void set_filter_offset(int offset);
	static void set_filter_order(int order);

	void setLowPassAudioFilterCutOffFrequency(int band_width);

  protected:
	~Demodulator();
	Demodulator(double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output);
	Demodulator(double ifrate,  DataBuffer<IQSample> *source_buffer, AudioInput *audio_input);
	Demodulator(AudioOutput *audio_output, AudioInput *audio_input);
	void mono_to_left_right(const SampleVector &samples_mono, SampleVector &audio);

	void gain_phasecorrection(IQSampleVector &samples_in, float vol);
	void adjust_calibration(IQSampleVector &samples_in);
	float adjust_resample_rate1(float rateAjustFraction);
	void tune_offset(long offset);
	//virtual void process(const IQSampleVector &samples_in, SampleVector &audio) = 0;
	void Resample(IQSampleVector &filter_in, IQSampleVector &filter_out);
	//virtual void operator()() = 0;
	void lowPassAudioFilter(const IQSampleVector &filter_in, IQSampleVector &filter_out);
	void lowPassAudioFilter(IQSampleVector &filter_in);
	void set_resample_rate(float resample_rate);
	void mix_down(IQSampleVector &filter_in);
	void mix_up(const IQSampleVector &filter_in, IQSampleVector &filter_out);
	void calc_if_level(const IQSampleVector &samples_in);
	void calc_signal_level(const IQSampleVector& samples_in);
	double get_if_level() { return ifEnergy.getEnergyLevel(); }
	double get_af_level() { return afEnergy.getEnergyLevel(); }
	double get_if_levelI() { return ifEnergy.getEnergyLevelI(); }
	double get_if_levelQ() { return ifEnergy.getEnergyLevelQ(); }
	double get_if_Correlation() { return ifEnergy.getEnergyCorrelation();}
	double get_if_CorrelationNorm() { return ifEnergy.getEnergyCorrelationNorm(); }
	double get_signal_level() { return SignalStrength.getEnergyLevel(); }
	void set_signal_strength();
	void setLowPassAudioFilter(float samplerate, int band_width);
	void set_span(long span);
	void perform_fft(const IQSampleVector &iqsamples);
	void calc_af_level(const SampleVector &samples_in);
	void setBandPassFilter(float high, float mid_high, float mid_low, float low);
	void executeBandpassFilter(const IQSampleVector &filter_in, IQSampleVector &filter_out);
	void dc_filter(IQSampleVector &filter_in);
	int get_audioBufferSize() { return audioBufferSize; }
	bool get_dc_filter();
	bool get_gain_phase_correction();
	int get_lowPassAudioFilterCutOffFrequency();
	bool get_lowPassAudioFilterChange();
	float adjust_resample_rate(float rateAjustFraction);
	DataBuffer<IQSample> *receiveIQBuffer{nullptr};
	AudioOutput *audioOutputBuffer{nullptr};
	double ifSampleRate;
	int audioSampleRate;
	DataBuffer<IQSample> *transmitIQBuffer{nullptr};
	AudioInput *audioInputBuffer{nullptr};
	long m_span{0L};
	void adjust_gain_phasecorrection(IQSampleVector &samples_in, float vol);
	void auto_adjust_gain_phasecorrection(IQSampleVector &samples_in, float vol);
	void FlashGainSlider(float envelope);
	float getSuppression();
	int get_noise() { return noisefilter.load(); }
	void NoiseFilterProcess(IQSampleVector &filter_in, IQSampleVector &filter_out);
	float getResampleRate() { return resampleRate; }


	void SquelchProcess(IQSampleVector &filter);
	bool Squelch();
	void SquelchPrint();

  private:
	EnergyCalculator ifEnergy, afEnergy, SignalStrength;
	mode_enum rxTxMode;
	nco_crcf tuneNCO{nullptr};
	msresamp_crcf resampleHandle{nullptr};
	long tuneOffsetFrequency;
	float resampleRate;
	int audioBufferSize;
	int highfftquadrant;

	iirfilt_crcf lowPassAudioFilterHandle{nullptr};
	
	iirfilt_crcf bandPassHandle{nullptr};
	iirfilt_crcf lowPassHandle{nullptr};
	iirfilt_crcf highPassHandle{nullptr};
	firfilt_crcf dcBlockHandle{nullptr};

	static atomic<bool> dcBlockSwitch;
	static atomic<int> correction;
	const int lowPassFilterOrder = 6;
	
	
	const liquid_iirdes_filtertype butterwurthType{LIQUID_IIRDES_BUTTER};
	const liquid_iirdes_bandtype bandFilterType{LIQUID_IIRDES_BANDPASS};
	const liquid_iirdes_format sosFormat{LIQUID_IIRDES_SOS};

	unsigned int filterOrder; 
	float cutOffFrequency; 
	float centerFrequency; 
	float passBandRipple;
	float StopBandAttenuation;

	std::atomic<int> lowPassAudioFilterCutOffFrequency;
	static std::atomic<int> noisefilter;
	static std::atomic<int> filter_type;
	static std::atomic<int> filter_offset;
	static std::atomic<int> filter_order;
	static std::atomic<bool> filter_change;
	static std::atomic<float> noiseThresshold;
	std::chrono::high_resolution_clock::time_point timeLastFlashGainSlider;

	AgcProcessor AgcProc;
	int threshold = -100;
	int attack_release = 1;
	int squelch_mode = 0;
};