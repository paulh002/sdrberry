#include "Demodulator.h"
#include "sdrberry.h"

Demodulator::Demodulator(double ifrate, int pcmrate, DataBuffer<IQSample16> *source_buffer, AudioInput *audio_input)
{	//  Transmit constructor
	m_ifrate = ifrate;
	m_pcmrate = pcmrate;
	m_transmit_buffer = source_buffer;
	m_audio_input = audio_input;

	// resampler and band filter assume pcmfrequency on the low side
	m_audio_mean = m_audio_rms = m_audio_level = m_if_level = 0.0;
	audio_input_on = true;	
}

Demodulator::Demodulator(double ifrate, int pcmrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output)
{	//  Receive constructor
	m_ifrate = ifrate;
	m_pcmrate = pcmrate;
	m_source_buffer = source_buffer;
	m_audio_output = audio_output;

	// resampler and band filter assume pcmfrequency on the low side
	m_audio_mean = m_audio_rms = m_audio_level = m_if_level = 0.0;
	tune_offset(vfo.get_vfo_offset());
	set_span(gsetup.get_span());
}

void Demodulator::set_span(int span)
{
	if (span < (m_ifrate/2) && span > 0)
	{
		if (m_span != span)
		{
			set_fft_reample_rate((float)span * 2);
		}
		m_span = span;
		int n =  (vfo.get_vfo_offset() / m_span);
		//printf("window: %d  offset %d\n", n, m_span * n);
		set_fft_mixer(m_span * n);
		Wf.set_fft_if_rate(2*m_span, n); 
		Wf.set_pos(vfo.get_vfo_offset());
	}
	else
	{
		m_span = span;
		set_fft_mixer(0);
		Wf.set_fft_if_rate(m_ifrate, 0);
		Wf.set_pos(vfo.get_vfo_offset());
	}
}

Demodulator::~Demodulator()
{
	auto startTime = std::chrono::high_resolution_clock::now();
	printf("destructor demod called\n");
	if (m_q)
		msresamp_crcf_destroy(m_q);
	if (m_fft_resample)
		msresamp_crcf_destroy(m_fft_resample);
	if (m_upnco != nullptr)
		nco_crcf_destroy(m_upnco);
	m_upnco = nullptr;
	if (m_lowpass)
		iirfilt_crcf_destroy(m_lowpass);
	m_lowpass = nullptr;
	if (m_fftmix)
		nco_crcf_destroy(m_fftmix);
	m_fftmix = nullptr;

	audio_input_on = false;

	auto now = std::chrono::high_resolution_clock::now();
	const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
	cout << "Stoptime demodulator:" << timePassed.count() << endl;
}

void Demodulator::set_reample_rate(float resample_rate)
{
	float As {60.0f};
		
	if (m_q)
		msresamp_crcf_destroy(m_q);
	m_resample_rate = resample_rate;
	m_q = msresamp_crcf_create(m_resample_rate, As);
	msresamp_crcf_print(m_q);	
}

void Demodulator::set_fft_reample_rate(float resample_rate)
{
	float As {60.0f};
		
	if (m_fft_resample)
	{
		msresamp_crcf_destroy(m_fft_resample);
		m_fft_resample = nullptr;
	}
	printf("resampe rate %f\n", resample_rate);
	m_fft_resample_rate = resample_rate / m_ifrate;
	m_fft_resample = msresamp_crcf_create(m_fft_resample_rate, As);
	msresamp_crcf_print(m_fft_resample);	
}

void Demodulator::fft_resample(const IQSampleVector& filter_in,
	IQSampleVector& filter_out)
{	
	unsigned int num_written;
	
	if (m_fft_resample)
	{
		float nx = (float)filter_in.size() * m_fft_resample_rate + 500;
		filter_out.reserve((int)ceilf(nx));
		filter_out.resize((int)ceilf(nx));
		msresamp_crcf_execute(m_fft_resample, (complex<float> *)filter_in.data(), filter_in.size(), (complex<float> *)filter_out.data(), &num_written);	
		filter_out.resize(num_written);
	}
	else
	{
		filter_out = filter_in;
	}
}

void Demodulator::calc_if_level(const IQSampleVector& samples_in)
{
	float y2 = 0.0;
	for (auto& con : samples_in)
	{
		y2 += std::real(con * std::conj(con));
	}
	// smooth energy estimate using single-pole low-pass filter
	y2 = y2 / samples_in.size();
	accuf = (1.0 - alpha)* accuf + alpha*y2;
	m_if_level = accuf;
}

void Demodulator::tune_offset(long offset)
{
	if (m_upnco != nullptr)
		nco_crcf_destroy(m_upnco);
	m_offset = offset;
	float rad_per_sample   = ((2.0f * (float)M_PI * (float)(vfo.get_vfo_offset())) / (float)m_ifrate);
	m_upnco = nco_crcf_create(LIQUID_NCO);
	nco_crcf_set_phase(m_upnco, 0.0f);
	nco_crcf_set_frequency(m_upnco, rad_per_sample);
}

void Demodulator::adjust_gain(IQSampleVector& samples_in, float vol)
{
	for (auto& col : samples_in)
	{
		col.real(col.real() * vol);
		col.imag(col.imag() * vol);
	}
}

void Demodulator::mono_to_left_right(const SampleVector& samples_mono,
	SampleVector& audio)
{
	unsigned int n = samples_mono.size();

	audio.resize(2*n);
	for (unsigned int i = 0; i < n; i++) {
		Sample m = samples_mono[i];
		audio[2*i]   = m;
		audio[2*i + 1] = m;
	}
}

void Demodulator::Resample(const IQSampleVector& filter_in,
	IQSampleVector& filter_out)
{	
	unsigned int num_written;
	
	if (m_q)
	{
		float nx = (float)filter_in.size() * m_resample_rate + 500;
		filter_out.reserve((int)ceilf(nx));
		filter_out.resize((int)ceilf(nx));
		msresamp_crcf_execute(m_q, (complex<float> *)filter_in.data(), filter_in.size(), (complex<float> *)filter_out.data(), &num_written);	
		filter_out.resize(num_written);
	}
	else
	{
		filter_out = filter_in;
	}
}	

void Demodulator::filter(const IQSampleVector& filter_in,
	IQSampleVector& filter_out)
{	
	for (auto& col : filter_in)
	{
		complex<float> v;
		iirfilt_crcf_execute(m_lowpass, col, &v);
		filter_out.insert(filter_out.end(), v);
	}
}

void Demodulator::mix_down(const IQSampleVector& filter_in,
	IQSampleVector& filter_out)
{	
	if (m_upnco)
	{
		for (auto& col : filter_in)
		{
			complex<float> v;
		
			nco_crcf_step(m_upnco);
			nco_crcf_mix_down(m_upnco, col, &v);
			filter_out.push_back(v);
		}
	}
	else
	{
		filter_out = filter_in;
	}
}

void Demodulator::mix_up(const IQSampleVector& filter_in,
	IQSampleVector& filter_out)
{	
	if (m_upnco)
	{
		for (auto& col : filter_in)
		{
			complex<float> v;
		
			nco_crcf_step(m_upnco);
			nco_crcf_mix_up(m_upnco, col, &v);
			filter_out.push_back(v);
		}
	}
	else
	{
		filter_out = filter_in;
	}
}

void Demodulator::fft_mix(int dir, const IQSampleVector& filter_in,
	IQSampleVector& filter_out)
{	
	if (m_fftmix)
	{
		for (auto& col : filter_in)
		{
			complex<float> v;
		
			nco_crcf_step(m_fftmix);
			if (dir)
				nco_crcf_mix_up(m_fftmix, col, &v);
			else
				nco_crcf_mix_down(m_fftmix, col, &v);				
			filter_out.push_back(v);
		}
	}
	else
	{
		filter_out = filter_in;
	}
}

void	Demodulator::set_fft_mixer(float offset)
{
	if (m_fftmix != nullptr)
	{
		nco_crcf_destroy(m_fftmix);
		m_fftmix = nullptr;
	}
	if (!offset)
		return;
	float rad_per_sample   = ((2.0f * (float)M_PI * (float)offset) / (float)m_ifrate);
	m_fftmix = nco_crcf_create(LIQUID_NCO);
	nco_crcf_set_phase(m_fftmix, 0.0f);
	nco_crcf_set_frequency(m_fftmix, rad_per_sample);
}

void	Demodulator::set_filter(float samplerate, float band_width)
{
	if (m_lowpass)
		iirfilt_crcf_destroy(m_lowpass);
	float factor =  band_width / samplerate;
	m_lowpass = iirfilt_crcf_create_lowpass(m_order, factor);
	iirfilt_crcf_print(m_lowpass);
}

void Demodulator::set_filter(int band_width)
{
	m_fcutoff = band_width;
}

void Demodulator::perform_fft(const IQSampleVector& iqsamples)
{
	IQSampleVector	iqsamples_filter, iqsamples_resample;
	
	if (m_span < (ifrate/2))
	{
		fft_mix(0, iqsamples, iqsamples_filter);
		fft_resample(iqsamples_filter, iqsamples_resample);
		Fft_calc.process_samples(iqsamples_resample);
	}
	else
	{
		Fft_calc.process_samples(iqsamples);
	}
}