#include "lvgl_.h"
#include "Demodulator.h"
#include "gui_speech.h"
#include "gui_cal.h"
#include "Spectrum.h"
#include "vfo.h"
#include "sdrberry.h"
#include "NoiseFilter.h"
#include <tuple>
#include "gui_squelch.h"

#define dB2mag(x) pow(10.0, (x) / 20.0)

/*
 *
 ** Basic class for processing radio data for bith RX and TX
 **
 **/
std::atomic<bool> Demodulator::dcBlockSwitch = true;
std::atomic<int> Demodulator::correction = 0;
std::atomic<double> correlationMeasurement, errorMeasurement;
std::atomic<int> Demodulator::noisefilter = 0;
std::atomic<float> Demodulator::noiseThresshold = 0.0;
std::atomic<int> Demodulator::filter_type = LIQUID_IIRDES_BUTTER;
std::atomic<int> Demodulator::filter_offset = 0;
std::atomic<int> Demodulator::filter_order = 6;
std::atomic<bool> Demodulator::filter_change = true;

Demodulator::Demodulator(AudioOutput *audio_output, AudioInput *audio_input)
{ //  echo constructor
	ifSampleRate = 0;
	audioSampleRate = audio_output->get_samplerate();
	transmitIQBuffer = nullptr;
	audioInputBuffer = audio_input;
	audioOutputBuffer = audio_output;
	audioBufferSize = Settings_file.get_int(default_radio, "audiobuffer");
	if (!audioBufferSize)
		audioBufferSize = 4096;
	correction = Settings_file.get_int(default_radio, "correction");
	highfftquadrant = 0;
	timeLastFlashGainSlider = std::chrono::high_resolution_clock::now();
	noisefilter = 0;
	lowPassAudioFilterCutOffFrequency = 0;
	// resampler and band filter assume pcmfrequency on the low side;
}

// Transmit mode contructor
Demodulator::Demodulator(double ifrate, DataBuffer<IQSample> *source_buffer, AudioInput *audio_input)
{ //  Transmit constructor
	ifSampleRate = ifrate;
	audioSampleRate = audio_input->get_samplerate();
	transmitIQBuffer = source_buffer;
	audioInputBuffer = audio_input;
	audioBufferSize = Settings_file.get_int(default_radio, "audiobuffertx");
	if (!audioBufferSize)
		audioBufferSize = 4096;
	highfftquadrant = 0;
	correction = Settings_file.get_int(default_radio, "correction");
	timeLastFlashGainSlider = std::chrono::high_resolution_clock::now();
	lowPassAudioFilterCutOffFrequency = 0;
	// resampler and band filter assume pcmfrequency on the low side
}

// Receive mode contructor
Demodulator::Demodulator(double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output)
{ //  Receive constructor
	ifSampleRate = ifrate;
	audioSampleRate = audio_output->get_samplerate();
	receiveIQBuffer = source_buffer;
	audioOutputBuffer = audio_output;
	audioBufferSize = Settings_file.get_int(default_radio, "audiobuffer");
	highfftquadrant = 0;
	if (!audioBufferSize)
		audioBufferSize = 4096;

	// resampler and band filter assume pcmfrequency on the low side
	tune_offset(vfo.get_vfo_offset());
	dcBlockHandle = firfilt_crcf_create_dc_blocker(25, 30);
	timeLastFlashGainSlider = std::chrono::high_resolution_clock::now();
	correction = Settings_file.get_int(default_radio, "correction");
	lowPassAudioFilterCutOffFrequency = 0;
}

void Demodulator::set_signal_strength()
{
	SpectrumGraph.set_signal_strength(get_signal_level());
}

Demodulator::~Demodulator()
{
	auto startTime = std::chrono::high_resolution_clock::now();
	printf("destructor demod called\n");
	if (resampleHandle)
		msresamp_crcf_destroy(resampleHandle);
	resampleHandle = nullptr;
	if (tuneNCO != nullptr)
		nco_crcf_destroy(tuneNCO);
	tuneNCO = nullptr;
	if (lowPassAudioFilterHandle)
		iirfilt_crcf_destroy(lowPassAudioFilterHandle);
	lowPassAudioFilterHandle = nullptr;

	if (bandPassHandle)
		iirfilt_crcf_destroy(bandPassHandle);
	if (lowPassHandle)
		iirfilt_crcf_destroy(lowPassHandle);
	if (highPassHandle)
		iirfilt_crcf_destroy(highPassHandle);
	if (dcBlockHandle)
		firfilt_crcf_destroy(dcBlockHandle);
	bandPassHandle = nullptr;
	lowPassHandle = nullptr;
	highPassHandle = nullptr;
	dcBlockHandle = nullptr;
	auto now = std::chrono::high_resolution_clock::now();
	const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
	cout << "Stoptime demodulator:" << timePassed.count() << endl;
}

void Demodulator::set_resample_rate(float resample_rate)
{
	float As{60.0f};

	if (resampleHandle)
		msresamp_crcf_destroy(resampleHandle);
	resampleRate = resample_rate;
	resampleHandle = msresamp_crcf_create(resampleRate, As);
	msresamp_crcf_print(resampleHandle);
}

float Demodulator::adjust_resample_rate(float rateAjustFraction)
{
	if ((resampleRate + resampleRate * rateAjustFraction) <= 0.0)
		return resampleRate;
	resampleRate = resampleRate + resampleRate * rateAjustFraction;
	struct msresamp_rrrf_s
	{
		// user-defined parameters
		float rate; // re-sampling rate
		float As;   // filter stop-band attenuation [dB]

		// type: interpolator or decimator
		int type; // run half-band resamplers as interp or decim

		// half-band resampler parameters
		unsigned int num_halfband_stages; // number of halfband stages
		msresamp2_rrrf halfband_resamp;	 // multi-stage halfband resampler
		float rate_halfband; // halfband rate

		// arbitrary resampler parameters
		resamp_rrrf		arbitrary_resamp;	 // arbitrary resampling object
		float rate_arbitrary; // clean-up resampling rate, in (0.5, 2.0)

		// internal buffers (ping-pong)
		unsigned int buffer_len;   // length of each buffer
		float *buffer;				   // buffer[0]
		unsigned int buffer_index; // index of buffer
	};

	msresamp_rrrf_s *_q = (msresamp_rrrf_s *)resampleHandle;
	if (_q != nullptr)
	{
		if (_q->type == LIQUID_RESAMP_DECIM)
		{
			float fraction = resampleRate / _q->rate , arbitraryRate;
			resamp_rrrf_adjust_rate(_q->arbitrary_resamp, fraction);
			_q->rate = resampleRate;
			arbitraryRate = resampleRate;
			for (int i = 0; i < _q->num_halfband_stages; i++)
				arbitraryRate *=  2.0;
			_q->rate_arbitrary = arbitraryRate;
		}
		msresamp_crcf_print(resampleHandle);
	}
	std::vector<std::string> resamplerate_setting;
	resamplerate_setting.push_back(std::to_string(resampleRate));
	resamplerate_setting.push_back(std::to_string(Settings_file.get_int(default_radio, "samplerate", 0)));
	resamplerate_setting.push_back(to_string(Settings_file.get_int(default_radio, "decimate", 0)));
	Settings_file.set_array_string(default_radio, "resample_rate", resamplerate_setting);
	Settings_file.write_settings();
	return resampleRate;
}

float Demodulator::adjust_resample_rate1(float rateAjustFraction)
{
	if ((abs(resampleRate - rateAjustFraction) / resampleRate) < 0.01)
		return resampleRate;
	resampleRate = rateAjustFraction;
	struct msresamp_rrrf_s
		{
			// user-defined parameters
			float rate; // re-sampling rate
			float As;	// filter stop-band attenuation [dB]

			// type: interpolator or decimator
			int type; // run half-band resamplers as interp or decim

			// half-band resampler parameters
			unsigned int num_halfband_stages; // number of halfband stages
			msresamp2_rrrf halfband_resamp;	  // multi-stage halfband resampler
			float rate_halfband;			  // halfband rate

			// arbitrary resampler parameters
			resamp_rrrf arbitrary_resamp; // arbitrary resampling object
			float rate_arbitrary;		  // clean-up resampling rate, in (0.5, 2.0)

			// internal buffers (ping-pong)
			unsigned int buffer_len;   // length of each buffer
			float *buffer;			   // buffer[0]
			unsigned int buffer_index; // index of buffer
		};

	msresamp_rrrf_s *_q = (msresamp_rrrf_s *)resampleHandle;
	if (_q != nullptr)
	{
		if (_q->type == LIQUID_RESAMP_DECIM)
		{
			float fraction = resampleRate / _q->rate, arbitraryRate;
			resamp_rrrf_adjust_rate(_q->arbitrary_resamp, fraction);
			_q->rate = resampleRate;
			arbitraryRate = resampleRate;
			for (int i = 0; i < _q->num_halfband_stages; i++)
				arbitraryRate *= 2.0;
			_q->rate_arbitrary = arbitraryRate;
		}
		msresamp_crcf_print(resampleHandle);
	}
	//std::string str1 = std::to_string(resampleRate);
	//Settings_file.save_string(default_radio, "resamplerate", str1);
	//Settings_file.write_settings();
	return resampleRate;
}

void Demodulator::calc_signal_level(const IQSampleVector& samples_in)
{
	SignalStrength.calculateEnergyLevel(samples_in);
}

void Demodulator::calc_if_level(const IQSampleVector &samples_in)
{
	ifEnergy.calculateEnergyLevel(samples_in);
}

void Demodulator::calc_af_level(const SampleVector &samples_in)
{
	afEnergy.calculateEnergyLevel(samples_in);
}

void Demodulator::FlashGainSlider(float envelope)
{
	auto now = std::chrono::high_resolution_clock::now();
	if (timeLastFlashGainSlider + std::chrono::milliseconds(500) < now)
	{ // toggle collor of gain slider when signal is limitted
		if (envelope > 0.99)
			guiQueue.push_back(GuiMessage(GuiMessage::action::blink, 1));
		else
			guiQueue.push_back(GuiMessage(GuiMessage::action::blink, 0));
		timeLastFlashGainSlider = now;
	}
}

// The vfo class calculates an offset within the bandwidth of the sdr radio
// tune offset configure the mixer to mix offset down to baseband
// use mix_down() to mix down to baseband during receive,mix_up() for transmit to mixup from baseband
void Demodulator::tune_offset(long offset)
{
	if (offset)
	{
		tuneOffsetFrequency = offset;
		float rad_per_sample = ((2.0f * (float)M_PI * (float)(tuneOffsetFrequency)) / (float)ifSampleRate);
		if (tuneNCO == nullptr)
			tuneNCO = nco_crcf_create(LIQUID_NCO);
		nco_crcf_set_phase(tuneNCO, 0.0f);
		nco_crcf_set_frequency(tuneNCO, rad_per_sample);
	}
	else
	{
		if (tuneNCO != nullptr)
			nco_crcf_destroy(tuneNCO);
		tuneNCO = nullptr;		
	}
}

void Demodulator::gain_phasecorrection(IQSampleVector &samples_in, float vol)
{
	double error, correlation;
	float autophase{0};
	float autogain{1.0};
	
	float gainManual = (float)gcal.getRxGain();
	float phaseManual = (float)gcal.getRxPhase();

	std::tuple<float, float, float> result = ifEnergy.ResultsMoseleyIQ();
	autophase = std::get<1>(result);
	autogain = std::get<2>(result);
	if (correction > 0)
	{
		for (auto &col : samples_in)
		{
			if (correction > 1)
			{
				col.real(col.real() + col.imag() * autophase);
				col.imag(col.imag() * autogain);
			}
			
			if (correction == 1 || correction == 3)
			{
				col.real(col.real() * gainManual);
				if (phaseManual < 0.0)
					col.real(col.real() + col.imag() * phaseManual);
				if (phaseManual > 0.0)
					col.imag(col.imag() + col.real() * phaseManual);
			}

			col.real(col.real() * vol);
			col.imag(col.imag() * vol);
		}
	}
	else
	{
		for (auto &col : samples_in)
		{
			col.real(col.real() * vol);
			col.imag(col.imag() * vol);
		}
	}
}


void Demodulator::adjust_calibration(IQSampleVector &samples_in)
{
	if (correction > 0)
	{
		float gain = (float)gcal.getTxGain();
		for (auto &col : samples_in)
		{
			col.real(col.real() * gain);
		}
		float phase = (float)gcal.getTxPhase();
		if (phase < 0.0)
		{
			for (auto &col : samples_in)
			{
				col.real(col.real() + col.imag() * phase);
			}
		}
		if (phase > 0.0)
		{
			for (auto &col : samples_in)
			{
				col.imag(col.imag() + col.real() * phase);
			}
		}
	}
}


// copy mono signal to both sereo channels
void Demodulator::mono_to_left_right(const SampleVector &samples_mono,
									 SampleVector &audio)
{
	unsigned int n = samples_mono.size();

	if (audio_output->get_channels() < 2)
	{
		audio = samples_mono;
		return;
	}
	audio.resize(2 * n);
	for (unsigned int i = 0; i < n; i++)
	{
		Sample m = samples_mono[i];
		audio[2 * i] = m;
		audio[2 * i + 1] = m;
	}
}

void Demodulator::Resample(IQSampleVector &filter_in,
						   IQSampleVector &filter_out)
{
	unsigned int num_written;

	if (resampleHandle)
	{
		if (filter_out.size() == 0)
		{
			float nx = (float)filter_in.size() * resampleRate * 2;
			filter_out.reserve((int)ceilf(nx));
			filter_out.resize((int)ceilf(nx));
		}
		msresamp_crcf_execute(resampleHandle, (complex<float> *)filter_in.data(), filter_in.size(), (complex<float> *)filter_out.data(), &num_written);
		filter_out.resize(num_written);
	}
	else
	{
		filter_out = std::move(filter_in);
	}
}

IQSampleVector Demodulator::Resample(IQSampleVector &filter_in)
{
	unsigned int num_written;
	IQSampleVector filter_out;
	
	if (resampleHandle)
	{
		if (filter_out.size() == 0)
		{
			float nx = (float)filter_in.size() * resampleRate * 2;
			filter_out.reserve((int)ceilf(nx));
			filter_out.resize((int)ceilf(nx));
		}
		msresamp_crcf_execute(resampleHandle, (complex<float> *)filter_in.data(), filter_in.size(), (complex<float> *)filter_out.data(), &num_written);
		filter_out.resize(num_written);
	}
	else
	{
		return filter_in;
	}
	return filter_out;
}
// audio filter 500 hz - 4.0 Khz
void Demodulator::lowPassAudioFilter(const IQSampleVector &filter_in,
									 IQSampleVector &filter_out)
{
	if (get_lowPassAudioFilterChange())
		setLowPassAudioFilter(audioSampleRate, lowPassAudioFilterCutOffFrequency);

	for (auto &col : filter_in)
	{
		complex<float> v, z;

		iirfilt_crcf_execute(lowPassAudioFilterHandle, col, &v);
		filter_out.insert(filter_out.end(), v);
	}
}

void Demodulator::lowPassAudioFilter(IQSampleVector &filter_in)
{
	if (get_lowPassAudioFilterChange())
		setLowPassAudioFilter(audioSampleRate, lowPassAudioFilterCutOffFrequency);
	
	for (auto &col : filter_in)
	{
		complex<float> v, z;

		iirfilt_crcf_execute(lowPassAudioFilterHandle, col, &v);
		col =  v;
	}
}

void Demodulator::dc_filter(IQSampleVector &filter_in)
{
	if (dcBlockHandle && dcBlockSwitch)
	{
		for (auto &col : filter_in)
		{
			complex<float> v;

			firfilt_crcf_push(dcBlockHandle, col);
			firfilt_crcf_execute(dcBlockHandle, &v);
			col = v;
		}
	}
}

void Demodulator::mix_down(IQSampleVector &filter_in)
{
	if (tuneNCO)
	{
		for (auto &col : filter_in)
		{
			complex<float> v;

			nco_crcf_step(tuneNCO);
			nco_crcf_mix_down(tuneNCO, col, &v);
			col = v;
		}
	}
}

void Demodulator::mix_up(const IQSampleVector &filter_in,
						 IQSampleVector &filter_out)
{
	if (tuneNCO)
	{
		for (auto &col : filter_in)
		{
			complex<float> v;

			nco_crcf_step(tuneNCO);
			nco_crcf_mix_up(tuneNCO, col, &v);
			filter_out.push_back(v);
		}
	}
	else
	{
		filter_out = std::move(filter_in);
	}
}

void Demodulator::mix_up(IQSampleVector &in)
{
	if (tuneNCO)
	{
		for (auto &col : in)
		{
			complex<float> v;

			nco_crcf_step(tuneNCO);
			nco_crcf_mix_up(tuneNCO, col, &v);
			col = v;
		}
	}
}

void Demodulator::setLowPassAudioFilter(float samplerate, int band_width)
{
	lowPassAudioFilterCutOffFrequency = band_width;
	if (lowPassAudioFilterHandle)
		iirfilt_crcf_destroy(lowPassAudioFilterHandle);

	int filtertype = LIQUID_IIRDES_LOWPASS;
	float cutOffFrequency = band_width / samplerate;
	float centerFrequency = 0.0;
	if (filter_offset)
	{
		centerFrequency = filter_offset / samplerate;
		cutOffFrequency = (band_width + filter_offset) / samplerate;
		filtertype = LIQUID_IIRDES_BANDPASS;
	}

	lowPassAudioFilterHandle = iirfilt_crcf_create_prototype((liquid_iirdes_filtertype)filter_type.load(), (liquid_iirdes_bandtype)filtertype, LIQUID_IIRDES_SOS, filter_order.load(), cutOffFrequency, centerFrequency, 0.1f, 60.0f);

	//lowPassAudioFilterHandle = iirfilt_crcf_create_lowpass(lowPassFilterOrder, factor);
	iirfilt_crcf_print(lowPassAudioFilterHandle);
}

void Demodulator::setLowPassAudioFilterCutOffFrequency(int band_width)
{
	lowPassAudioFilterCutOffFrequency = band_width;
}

void Demodulator::perform_fft(const IQSampleVector &iqsamples)
{
	SpectrumGraph.ProcessWaterfall(iqsamples);
}

float Demodulator::getSuppression()
{
	return SpectrumGraph.getSuppression();
}

void Demodulator::setBandPassFilter(float high, float mid_high, float mid_low, float low)
{
	cutOffFrequency = mid_low / (float)audioSampleRate;  // cutoff frequency
	centerFrequency = mid_high / (float)audioSampleRate; // center frequency
	passBandRipple = 1.0f;								 // pass-band ripple
	StopBandAttenuation = 40.0f;						 // stop-band attenuation
	filterOrder = 4;

	if (bandPassHandle)
		iirfilt_crcf_destroy(bandPassHandle);
	if (lowPassHandle)
		iirfilt_crcf_destroy(lowPassHandle);
	if (highPassHandle)
		iirfilt_crcf_destroy(highPassHandle);

	bandPassHandle = iirfilt_crcf_create_prototype(butterwurthType, bandFilterType, sosFormat, filterOrder, cutOffFrequency, centerFrequency, passBandRipple, StopBandAttenuation);
	iirfilt_crcf_print(bandPassHandle);

	cutOffFrequency = low / (float)audioSampleRate;		// cutoff frequency
	centerFrequency = mid_low / (float)audioSampleRate; // center frequency
	lowPassHandle = iirfilt_crcf_create_prototype(butterwurthType, bandFilterType, sosFormat, filterOrder, cutOffFrequency, centerFrequency, passBandRipple, StopBandAttenuation);
	iirfilt_crcf_print(lowPassHandle);

	cutOffFrequency = mid_high / (float)audioSampleRate; // cutoff frequency
	centerFrequency = high / (float)audioSampleRate;	 // center frequency
	highPassHandle = iirfilt_crcf_create_prototype(butterwurthType, bandFilterType, sosFormat, filterOrder, cutOffFrequency, centerFrequency, passBandRipple, StopBandAttenuation);
	iirfilt_crcf_print(highPassHandle);
}

void Demodulator::executeBandpassFilter(IQSampleVector &filter_in, bool conjugation)
{
	if (bandPassHandle != nullptr && lowPassHandle != nullptr && highPassHandle != nullptr)
	{
		float bass_gain = dB2mag(gspeech.get_bass());
		float treble_gain = dB2mag(gspeech.get_treble());
		for (auto &col : filter_in)
		{
			complex<float> x, v, w, u, z;

			if (conjugation)
				x = col;
			else
				x = col;
			iirfilt_crcf_execute(bandPassHandle, x, &v);
			iirfilt_crcf_execute(lowPassHandle, x, &w);
			iirfilt_crcf_execute(highPassHandle, x, &u);
			v = v + w * bass_gain + u * treble_gain;
			col = v;
		}
	}
}

bool Demodulator::get_dc_filter()
{
	if (Settings_file.get_int(default_radio, "dc"))
		return true;
	else
		return false;
}

void Demodulator::set_dc_filter(bool state)
{
	if (state)
		dcBlockSwitch = true;
	else
		dcBlockSwitch = false;
}

void Demodulator::set_autocorrection(int state)
{
	correction.store(state);
	printf("auto correction %d\n", correction.load());
}

void Demodulator::set_noise_filter(int noise)
{
	noisefilter = noise;
}

void Demodulator::set_filter_type(int type)
{
	filter_change = true;
	filter_type = type;
}

void Demodulator::set_filter_offset(int offset)
{
	filter_change = true;
	filter_offset = offset;
}

void Demodulator::set_filter_order(int order)
{
	filter_change = true;
	filter_order = order;
}

int Demodulator::get_lowPassAudioFilterCutOffFrequency() 
{ 	
return lowPassAudioFilterCutOffFrequency.load(); 
}

bool Demodulator::get_lowPassAudioFilterChange()
{
	if (filter_change)
	{
		filter_change = false;
		return true;
	}
return false;
}

void Demodulator::set_noise_threshold(int threshold)
{
	noiseThresshold = threshold;
}

void Demodulator::NoiseFilterProcess(IQSampleVector &filter_in,
						   IQSampleVector &filter_out)
{
	NoiseFilter nf;

	nf.Process(filter_in, filter_out);
}

void Demodulator::SquelchProcess(IQSampleVector &filter)
{
	int s = guisquelch.get_mode();
	if (s != squelch_mode || s > 0)
	{
		if (s == 2 && s != squelch_mode)
		{
			squelch_mode = s;
			AgcProc.SetSquelch(true);
		}
		if ((s == 0 || s == 1) && s != squelch_mode)
		{
			squelch_mode = s;
			AgcProc.SetSquelch(false);
		}
		int t = guisquelch.get_threshold();
		if (t != threshold)
		{
			AgcProc.SetSquelchThreshold(t);
			threshold = t;
		}
		int a = guisquelch.get_attack_release();
		if (a!= attack_release)
		{
			AgcProc.set_bandwidth((float)a / 1000.0);
			attack_release = a;
		}
			
		squelch_mode = s;
		if (s > 0)
			AgcProc.Process(filter);
	}
}

bool Demodulator::Squelch()
{
	return AgcProc.squelch();
}

void Demodulator::SquelchPrint()
{
	AgcProc.print();
}
