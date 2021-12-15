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
	audio_output->open();
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

Demodulator::~Demodulator()
{
	printf("destructor demod called\n");
	if (m_upnco != nullptr)
		nco_crcf_destroy(m_upnco);
	m_upnco = nullptr;
	if (m_lowpass)
		iirfilt_crcf_destroy(m_lowpass);
	m_lowpass = nullptr;
	audio_output->close(); 
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