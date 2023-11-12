#include "Demodulator.h"
#include "gui_speech.h"
#include "gui_cal.h"
#include "Spectrum.h"

#define dB2mag(x) pow(10.0, (x) / 20.0)

/*
 *
 ** Basic class for processing radio data for bith RX and TX
 **
 **/
atomic<int> Demodulator::lowPassAudioFilterCutOffFrequency = 0;

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
	adjustPhaseGain = get_gain_phase_correction();
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
	adjustPhaseGain = get_gain_phase_correction();

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
	if (!audioBufferSize)
		audioBufferSize = 4096;

	// resampler and band filter assume pcmfrequency on the low side
	tune_offset(vfo.get_vfo_offset());
	//set_span(gsetup.get_span());
	if (get_dc_filter())
		dcBlockHandle = firfilt_crcf_create_dc_blocker(25, 30);
	else
		dcBlockHandle = nullptr;
	adjustPhaseGain = get_gain_phase_correction();
}

void Demodulator::set_signal_strength()
{
	SpectrumGraph.set_signal_strength(get_if_level());
}

// decrease the span of the fft display by downmixing the bandwidth of the receiver
// Chop the bandwith in parts en display correct part
void Demodulator::set_span(int span)
{
	if ((span < ifSampleRate) && span > 0)
	{
		if (m_span != span)
		{
			set_fft_resample_rate((float)span * 2);
		}
		m_span = span;
		int n = (vfo.get_vfo_offset() / m_span);
		//printf("window: %d  offset %d\n", n, m_span * n);
		set_fft_mixer(m_span * n);
		SpectrumGraph.set_fft_if_rate(2 * m_span, n);
		guiQueue.push_back(GuiMessage(GuiMessage::action::setpos, 0));
		//SpectrumGraph.set_pos(vfo.get_vfo_offset(), true);
	}
	else
	{
		m_span = span;
		set_fft_resample_rate(0.0);
		set_fft_mixer(0);
		SpectrumGraph.set_fft_if_rate(ifSampleRate, 0);
		guiQueue.push_back(GuiMessage(GuiMessage::action::setpos, 0));
		//SpectrumGraph.set_pos(vfo.get_vfo_offset(), true);
	}
}

Demodulator::~Demodulator()
{
	auto startTime = std::chrono::high_resolution_clock::now();
	printf("destructor demod called\n");
	if (resampleHandle)
		msresamp_crcf_destroy(resampleHandle);
	resampleHandle = nullptr;
	if (fftResampleHandle)
		msresamp_crcf_destroy(fftResampleHandle);
	fftResampleHandle = nullptr;
	if (tuneNCO != nullptr)
		nco_crcf_destroy(tuneNCO);
	tuneNCO = nullptr;
	if (lowPassAudioFilterHandle)
		iirfilt_crcf_destroy(lowPassAudioFilterHandle);
	lowPassAudioFilterHandle = nullptr;
	if (fftNCOHandle)
		nco_crcf_destroy(fftNCOHandle);
	fftNCOHandle = nullptr;

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
	return resampleRate;
}



void Demodulator::set_fft_resample_rate(float resample_rate)
{
	float As{60.0f};

	if (fftResampleHandle)
	{
		msresamp_crcf_destroy(fftResampleHandle);
		fftResampleHandle = nullptr;
	}
	if (resample_rate > 0.0)
	{
		printf("fft resample rate %f\n", resample_rate);
		fftResampleRate = resample_rate / ifSampleRate;
		fftResampleHandle = msresamp_crcf_create(fftResampleRate, As);
		msresamp_crcf_print(fftResampleHandle);
	}
}

void Demodulator::fft_resample(const IQSampleVector &filter_in,
							   IQSampleVector &filter_out)
{
	unsigned int num_written;

	if (fftResampleHandle)
	{
		float nx = (float)filter_in.size() * fftResampleRate * 2;
		filter_out.reserve((int)ceilf(nx));
		filter_out.resize((int)ceilf(nx));
		msresamp_crcf_execute(fftResampleHandle, (complex<float> *)filter_in.data(), filter_in.size(), (complex<float> *)filter_out.data(), &num_written);
		filter_out.resize(num_written);
	}
	else
	{
		filter_out = filter_in;
	}
}

void Demodulator::calc_if_level(const IQSampleVector &samples_in)
{
	ifEnergy.calculateEnergyLevel(samples_in);
}

void Demodulator::calc_af_level(const SampleVector &samples_in)
{
	afEnergy.calculateEnergyLevel(samples_in);
}

// The vfo class calculates an offset within the bandwidth of the sdr radio
// tune offset configure the mixer to mix offset down to baseband
// use mix_down() to mix down to baseband during receive,mix_up() for transmit to mixup from baseband
void Demodulator::tune_offset(long offset)
{
	tuneOffsetFrequency = offset;
	float rad_per_sample = ((2.0f * (float)M_PI * (float)(vfo.get_vfo_offset())) / (float)ifSampleRate);
	if (tuneNCO == nullptr)
		tuneNCO = nco_crcf_create(LIQUID_NCO);
	nco_crcf_set_phase(tuneNCO, 0.0f);
	nco_crcf_set_frequency(tuneNCO, rad_per_sample);
}

void Demodulator::adjust_gain_phasecorrection(IQSampleVector &samples_in, float vol)
{
	if (adjustPhaseGain)
	{
		float phase = (float)gcal.getRxPhase();
		float gain = (float)gcal.getRxGain();
		for (auto& col : samples_in)
		{
			col.real(col.real() * vol * gain);
			col.imag(col.imag() * vol);
			if (phase < 0.0)
				col.real(col.real() + col.imag() * phase);
			if (phase > 0.0)
				col.imag(col.imag() + col.real() * phase);
		}
	}
	else
	{
		for (auto& col : samples_in)
		{
			col.real(col.real() * vol);
			col.imag(col.imag() * vol);
		}
	}
}

void Demodulator::adjust_calibration(IQSampleVector &samples_in)
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
		float nx = (float)filter_in.size() * resampleRate * 2;
		filter_out.reserve((int)ceilf(nx));
		filter_out.resize((int)ceilf(nx));
		msresamp_crcf_execute(resampleHandle, (complex<float> *)filter_in.data(), filter_in.size(), (complex<float> *)filter_out.data(), &num_written);
		filter_out.resize(num_written);
	}
	else
	{
		filter_out = std::move(filter_in);
	}
}

// audio filter 500 hz - 4.0 Khz
void Demodulator::lowPassAudioFilter(const IQSampleVector &filter_in,
									 IQSampleVector &filter_out)
{
	for (auto &col : filter_in)
	{
		complex<float> v, z;

		iirfilt_crcf_execute(lowPassAudioFilterHandle, col, &v);
		filter_out.insert(filter_out.end(), v);
	}
}

void Demodulator::dc_filter(IQSampleVector &filter_in,
							IQSampleVector &filter_out)
{
	if (dcBlockHandle)
	{
		for (auto &col : filter_in)
		{
			complex<float> v;

			firfilt_crcf_push(dcBlockHandle, col);
			firfilt_crcf_execute(dcBlockHandle, &v);
			filter_out.insert(filter_out.end(), v);
		}
	}
	else
	{
		filter_out = std::move(filter_in);
	}
}

void Demodulator::mix_down(const IQSampleVector &filter_in,
						   IQSampleVector &filter_out)
{
	if (tuneNCO)
	{
		for (auto &col : filter_in)
		{
			complex<float> v;

			nco_crcf_step(tuneNCO);
			nco_crcf_mix_down(tuneNCO, col, &v);
			filter_out.push_back(v);
		}
	}
	else
	{
		filter_out = filter_in;
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
		filter_out = filter_in;
	}
}

void Demodulator::fft_mix(int dir, const IQSampleVector &filter_in,
						  IQSampleVector &filter_out)
{
	if (fftNCOHandle)
	{
		for (auto &col : filter_in)
		{
			complex<float> v;

			nco_crcf_step(fftNCOHandle);
			if (dir)
				nco_crcf_mix_up(fftNCOHandle, col, &v);
			else
				nco_crcf_mix_down(fftNCOHandle, col, &v);
			filter_out.push_back(v);
		}
	}
	else
	{
		filter_out = filter_in;
	}
}

void Demodulator::set_fft_mixer(float offset)
{
	if (fftNCOHandle != nullptr)
	{
		nco_crcf_destroy(fftNCOHandle);
		fftNCOHandle = nullptr;
	}
	if (!offset)
		return;
	float rad_per_sample = ((2.0f * (float)M_PI * (float)offset) / (float)ifSampleRate);
	fftNCOHandle = nco_crcf_create(LIQUID_NCO);
	nco_crcf_set_phase(fftNCOHandle, 0.0f);
	nco_crcf_set_frequency(fftNCOHandle, rad_per_sample);
}

void Demodulator::setLowPassAudioFilter(float samplerate, float band_width)
{
	if (lowPassAudioFilterHandle)
		iirfilt_crcf_destroy(lowPassAudioFilterHandle);
	float factor = band_width / samplerate;
	lowPassAudioFilterHandle = iirfilt_crcf_create_lowpass(lowPassFilterOrder, factor);
	iirfilt_crcf_print(lowPassAudioFilterHandle);
}

void Demodulator::setLowPassAudioFilterCutOffFrequency(int fc)
{
	lowPassAudioFilterCutOffFrequency = fc;
}

void Demodulator::perform_fft(const IQSampleVector &iqsamples)
{
	IQSampleVector iqsamples_filter, iqsamples_resample;

	if (m_span < ifrate)
	{
		fft_mix(0, iqsamples, iqsamples_filter);
		fft_resample(iqsamples_filter, iqsamples_resample);
	}
	else
		fft_resample(iqsamples, iqsamples_resample);
	SpectrumGraph.ProcessWaterfall(iqsamples_resample);
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

void Demodulator::executeBandpassFilter(const IQSampleVector &filter_in,
										IQSampleVector &filter_out)
{
	if (bandPassHandle == nullptr || lowPassHandle == nullptr || highPassHandle == nullptr)
	{
		filter_out = filter_in;
		return;
	}
	float bass_gain = dB2mag(gspeech.get_bass());
	float treble_gain = dB2mag(gspeech.get_treble());
	for (auto &col : filter_in)
	{
		complex<float> v, w, u, z;

		iirfilt_crcf_execute(bandPassHandle, col, &v);
		iirfilt_crcf_execute(lowPassHandle, col, &w);
		iirfilt_crcf_execute(highPassHandle, col, &u);
		v = v + w * bass_gain + u * treble_gain;
		filter_out.insert(filter_out.end(), v);
	}
}

bool Demodulator::get_dc_filter()
{
	if (Settings_file.get_int(default_radio, "dc"))
		return true;
	else
		return false;
}

bool Demodulator::get_gain_phase_correction()
{
	if (Settings_file.get_int(default_radio, "correction"))
		return true;
	else
		return false;
}