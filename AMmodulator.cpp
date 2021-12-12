#include <cstdio>
#include <cassert>
#include <cmath>
#include <complex>
#include <liquid.h>
#include <vector>
#include <algorithm>
#include <mutex>
#include "DataBuffer.h"
#include "AudioOutput.h"
#include "AudioInput.h"
#include "sdrberry.h"
#include "AMmodulator.h"
#include "Waterfall.h"

static shared_ptr<AMmodulator> sp_ammod;

AMmodulator::AMmodulator(int mode, double ifrate, int pcmrate, int tone, DataBuffer<IQSample16> *source_buffer, AudioInput *audio_input)
{
	float					mod_index = 0.99f;   // modulation index (bandwidth)
    float					As = 60.0f;				// resampling filter stop-band attenuation [dB]
	int						suppressed_carrier;
	liquid_ampmodem_type	am_mode;
		
	switch (mode)
	{
	case mode_usb:
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_LSB;
		printf("tx mode LIQUID_AMPMODEM_USB carrier %d\n", suppressed_carrier);		
		break;
	case mode_lsb:
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_USB;
		printf("tx mode LIQUID_AMPMODEM_LSB carrier %d\n", suppressed_carrier);		
		break;
	case mode_am:
		suppressed_carrier = 0;
		am_mode = LIQUID_AMPMODEM_DSB;
		printf("tx mode LIQUID_AMPMODEM_DSB carrier %d\n", suppressed_carrier);		
		break;
	case mode_dsb:
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_DSB;
		printf("tx mode LIQUID_AMPMODEM_DSB carrier %d\n", suppressed_carrier);		
		break;
	default:
		printf("Mode not correct\n");		
		return;
	}
	if (tone == 0)
		audio_input->open();
	
	m_tone = tone;
	m_source_buffer = source_buffer;
	m_source_buffer->restart_queue();
	m_audio_input = audio_input;
	m_ifrate = ifrate;
	m_r = (float) ifrate / (float)pcmrate ; 
	m_pcmrate = pcmrate;
	m_audio_mean = m_audio_rms = m_audio_level = m_if_level = 0.0;
	m_mod = ampmodem_create(mod_index, am_mode, suppressed_carrier);
	ampmodem_print(m_mod);
	set_filter(m_ifrate, 2500);
	m_bresample = false;
	if (m_r > 1.0001)
	{
		printf("resample rate %f \n", m_r);
		m_bresample = true;
		m_q = msresamp_crcf_create(m_r, As);
		msresamp_crcf_print(m_q);			
	}
	m_upssb = nco_crcf_create(LIQUID_NCO);
	float rad_per_sample   = ((2.0f * (float)M_PI * 3000.0f) / (float)pcmrate);
	nco_crcf_set_phase(m_upssb, 0.0f);
	nco_crcf_set_frequency(m_upssb, rad_per_sample);
	 
	//printf("tune TX to %f\n", BASEQRG * 1e3 + offset);
	rad_per_sample   = ((2.0f * (float)M_PI * (float)(vfo.get_vfo_offset())) / (float)m_ifrate);
	m_upnco = nco_crcf_create(LIQUID_NCO);
	nco_crcf_set_phase(m_upnco, 0.0f);
	nco_crcf_set_frequency(m_upnco, rad_per_sample);
	createBandpass(0);
}

void AMmodulator::createBandpass(int txfilter)
{
	const unsigned int tx_lp_order =   8;        // filter order
	const float        tx_lp_Ap    =   1.0f;      // pass-band ripple
	const float        tx_lp_As    =  80.0f;      // stop-band attenuation
	const unsigned int tx_lp_n     = 128;         // number of samples
	float        tx_lp_f0    =   0.09f;       // center frequency
	float        tx_lp_fc    =   0.066f;       // cutoff frequency
	
    if(m_tx_lp_q) iirfilt_crcf_destroy(m_tx_lp_q);

    // Frequencies are designed for 3kHz carrier
    switch(txfilter)
    {
        case 0 : {tx_lp_fc=0.079f; tx_lp_f0=0.09f;} break;
        case 1 : {tx_lp_fc=0.074f; tx_lp_f0=0.09f;} break;
        case 2 : {tx_lp_fc=0.070f; tx_lp_f0=0.09f;} break;
        case 3 : {tx_lp_fc=0.066f; tx_lp_f0=0.09f;} break;
    }

    m_tx_lp_q = iirfilt_crcf_create_prototype(LIQUID_IIRDES_ELLIP, LIQUID_IIRDES_BANDPASS, LIQUID_IIRDES_SOS,
                                         tx_lp_order, tx_lp_fc, tx_lp_f0, tx_lp_Ap, tx_lp_As);
}

AMmodulator::~AMmodulator()
{
	if (m_lowpass)
		iirfilt_crcf_destroy(m_lowpass); 
	if (m_q)
		msresamp_crcf_destroy(m_q);
	if (m_mod)
		ampmodem_destroy(m_mod);
	if (m_upssb) 
		nco_crcf_destroy(m_upssb);
	if (m_upnco) 
		nco_crcf_destroy(m_upnco); 
	if (m_tx_lp_q) 
		iirfilt_crcf_destroy(m_tx_lp_q);
	m_upssb = nullptr;
	m_q = 0;
	m_mod = 0;
	m_upnco = 0;
	m_tx_lp_q = 0;
}

void	AMmodulator::tone(bool tone)
{
	unique_lock<mutex> lock(m_mutex); 
	if (tone)
	{
		m_audio_input->close();
		m_tone = tone;
	}
	else
	{
		m_tone = false;
		m_audio_input->open();
	}
}

void AMmodulator::set_filter(float samplerate, float band_width)
{
	if (m_lowpass)
		iirfilt_crcf_destroy(m_lowpass);
	float factor =  band_width / samplerate;
	m_lowpass = iirfilt_crcf_create_lowpass(m_order, factor);
	iirfilt_crcf_print(m_lowpass);
}

template <typename D, typename S> std::complex<D> cast(const std::complex<S> s)
{
	return std::complex<D>(s.real(), s.imag());
}

void AMmodulator::tune_offset(long offset)
{	// get lock on modulator process
	unique_lock<mutex> lock(m_mutex);
	
	nco_crcf_destroy(m_upnco);
	m_offset = offset;
	float rad_per_sample   = ((2.0f * (float)M_PI * (float)(vfo.get_vfo_offset())) / (float)m_ifrate);
	m_upnco = nco_crcf_create(LIQUID_NCO);
	nco_crcf_set_phase(m_upnco, 0.0f);
	nco_crcf_set_frequency(m_upnco, rad_per_sample);
}

void AMmodulator::process(const SampleVector& samples, double ifrate, DataBuffer<IQSample16>	*source_buffer)
{
	IQSampleVector				buf_mod, buf_filter, buf_out;
	IQSampleVector16			buf_out16;
	unsigned int				num_written;
	
	unique_lock<mutex> lock(m_mutex); 
	// Modulate audio to USB, LSB or DSB
	buf_mod.clear(); 
	for (auto& col : samples)
	{	complex<float> f ;	
		
		ampmodem_modulate(m_mod, col, &f);
		//printf("%f;%f;%f \n", col, f.real(), f.imag());
		buf_mod.push_back(f);
	}
	double if_rms = rms_level_approx(buf_mod);
	m_if_level = 0.95 * m_if_level + 0.05 * if_rms;
	
	// Low pass filter 500 Hz - 4 Khz to be selected
	// Maybe 4 Khz only is enough als add high pass for 300 Hz
	buf_filter.clear(); 
	for (auto& col : buf_mod)
	{
		complex<float> f;	
		iirfilt_crcf_execute(m_lowpass, col, &f);
		buf_filter.push_back(f);
	}

	// convert to output samplerate?
	if (m_bresample)
	{
		num_written = buf_filter.size() * m_r + 1000;
		buf_out.clear();
		buf_out.reserve(num_written);
		buf_out.resize(num_written);
		//printf("filter %d rate %f buffer size %d\n", buf_filter.size(), m_r, num_written);
		// execute resampler
		msresamp_crcf_execute(m_q, (complex<float> *)buf_filter.data(), buf_filter.size(), (complex<float> *)buf_out.data(), &num_written);
		buf_out.resize(num_written);	
		
		buf_out16.clear();
		for (auto& col : buf_out)
		{
			complex<float> f;
			int16_t i, q;
			
			nco_crcf_step(m_upnco);
			nco_crcf_mix_up(m_upnco, col, &f);
			col = f; // still need a correcy buf_out
			i = (int16_t)round(col.real() * 16384.0f);
			q = (int16_t)round(col.imag() * 16384.0f);
			IQSample16 s16 {i, q};
			buf_out16.push_back(s16);
		}
		Fft_calc.process_samples(buf_out);
		source_buffer->push(move(buf_out16));
	}
	else
	{	// No resampling ifrate = 48K, in this case no offset use
		buf_out16.clear();
		for (auto& col : buf_mod)
		{
			int16_t i, q;
			
			i = (int16_t)round(col.real() * 16384.0f);  //32768.999f  16384
			q = (int16_t)round(col.imag() * 16384.0f);
			IQSample16 s16 {i, q};
			buf_out16.push_back(s16);
		}
		Fft_calc.process_samples(buf_mod); 
		source_buffer->push(move(buf_out16));
	}
	buf_mod.clear();
	buf_out.clear();
	buf_mod.clear();
	buf_filter.clear();
}

void AMmodulator::operator()()
{
	unsigned int            fft_block = 0;
	bool                    inbuf_length_warning = false;
	SampleVector            audiosamples;
	int						ifilter {-1};
	 
	Fft_calc.plan_fft(nfft_samples * 10); 
	while (!stop_txmod_flag.load())
	{
		if (vfo.tune_flag == true)
		{
			vfo.tune_flag = false;
			tune_offset(vfo.get_vfo_offset());
		}
		
		if (m_tone)
		{
			m_audio_input->ToneBuffer(m_tone);
			m_source_buffer->wait_queue_empty(2);
		}
	
		if (m_audio_input->read(audiosamples) == false)
		{
			printf("wait for input\n");
			usleep(1000); // wait 1024 audio sample time
			continue;
		}
		Fft_calc.set_signal_strength(m_audio_input->get_rms_level());
		process(audiosamples, m_ifrate, m_source_buffer);
		audiosamples.clear();
	}
	m_source_buffer->push_end();
	printf("exit am_mod_thread\n");
}

bool AMmodulator::create_modulator(int mode, double ifrate, int pcmrate, int tone, DataBuffer<IQSample16> *source_buffer, AudioInput *audio_input)
{	
	if (sp_ammod != nullptr)
		return false;
	sp_ammod = make_shared<AMmodulator>(mode, ifrate, pcmrate, tone, source_buffer, audio_input);
	sp_ammod->ammod_thread = std::thread(&AMmodulator::operator(), sp_ammod);
	return true;
}

void AMmodulator::destroy_modulator()
{
	if (sp_ammod == nullptr)
		return;
	sp_ammod->stop_txmod_flag = true;
	sp_ammod->ammod_thread.join();
	sp_ammod.reset();
}