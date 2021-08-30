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

pthread_t					am_mod_pthread;
static	modulator_struct	mod_data;

void	AMmodulator::init(modulator_struct * ptr)
{
	float       mod_index = 0.99f;   // modulation index (bandwidth)
    float		As = 60.0f;				// resampling filter stop-band attenuation [dB]
	
	m_r = (float) ptr->ifrate / (float)ptr->pcmrate ; 
	m_audio_mean = m_audio_rms = m_audio_level = m_if_level = 0.0;
	m_mod = ampmodem_create(mod_index, ptr->mode, ptr->suppressed_carrier);
	ampmodem_print(m_mod);
	set_filter(ptr->ifrate, 5);
	if (m_r > 1.0001)
	{
		printf("resample rate %f \n", m_r);
		m_bresample = true;
		m_q = msresamp_crcf_create(m_r, As);
		msresamp_crcf_print(m_q);			
	}
	m_upssb = nco_crcf_create(LIQUID_NCO);
	float rad_per_sample   = ((2.0f * (float)M_PI * 3000.0f) / (float)ptr->pcmrate);
	nco_crcf_set_phase(m_upssb, 0.0f);
	nco_crcf_set_frequency(m_upssb, rad_per_sample);
	 
	//printf("tune TX to %f\n", BASEQRG * 1e3 + offset);
	rad_per_sample   = ((2.0f * (float)M_PI * (float)(vfo.get_vfo_offset())) / (float)ptr->ifrate);
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


void AMmodulator::am_exit()
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
		audio_input->close();
		m_modulator.tone = tone;
	}
	else
	{
		m_modulator.tone = false;
		m_modulator.audio_input->open();
	}
}
	
void	AMmodulator::set_filter(double if_rate, int band_width)
{
	double	factor {0.03125};
	
	unique_lock<mutex> lock(m_mutex); 
	if (m_lowpass)
		iirfilt_crcf_destroy(m_lowpass);
	switch (band_width)
	{
	case 0:
		// 500hz
		factor = 0.00520833333333333;
		break;
	case 1:
		// 1Khz
		factor = 0.010416667;
		break;
	case 2:
		// 1.5 khz
		factor = 0.015625;
		break;
	case 3:
		// 2khz
		factor = 0.0208333333333333;
		break;
	case 4:
		// 2.5 khz
		factor = 0.0260416666666667;
		break;
	case 5:
		factor = 0.03125;
		// 3 Khz
		break;
	case 6:
		factor = 0.0364583333333333;
		//  3.5Khz
		break;
	case 7:
		factor = 0.0416666666666667;
		//  4Khz
		break;
	}
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
	float rad_per_sample   = ((2.0f * (float)M_PI * (float)(vfo.get_vfo_offset())) / (float)ifrate);
	m_upnco = nco_crcf_create(LIQUID_NCO);
	nco_crcf_set_phase(m_upnco, 0.0f);
	nco_crcf_set_frequency(m_upnco, rad_per_sample);
}


void	AMmodulator::process(const SampleVector& samples, double ifrate, DataBuffer<IQSample16>	*source_buffer)
{
	IQSampleVector				buf_mod, buf_filter, buf_out;
	IQSampleVector16			buf_out16;
	unsigned int				num_written;
	
	unique_lock<mutex> lock(m_mutex); 
	// Modulate audio to USB, LSB or DSB
	buf_mod.clear(); 
	for (auto& col : samples)
	{	complex<float> f;	
		ampmodem_modulate(m_mod, col, &f);
		buf_mod.push_back(f);
	}
	double if_rms = rms_level_approx(buf_mod);
	m_if_level = 0.95 * m_if_level + 0.05 * if_rms;
	
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
			i = (int16_t)round(col.real() * 32767.999f);
			q = (int16_t)round(col.imag() * 32767.999f);
			IQSample16 s16 {i, q};
			buf_out16.push_back(s16);
		}
		Fft_calc.process_samples(buf_out);
		source_buffer->push(move(buf_out16));
	}
	else
	{
		buf_out16.clear();
		for (auto& col : buf_mod)
		{
			int16_t i, q;
			
			i = (int16_t)round(col.real() * 32767.999f);
			q = (int16_t)round(col.imag() * 32767.999f);
			IQSample16 s16 {i, q};
			buf_out16.push_back(s16);
		}
		Fft_calc.process_samples(buf_mod); 
		source_buffer->push(move(buf_out16));
	}
	//if (source_buffer->size() > 5)
	//	printf("queue length %d\n", source_buffer->size());
	buf_mod.clear();
	buf_out.clear();
	buf_mod.clear();
	buf_filter.clear();
}

void* am_mod_thread(void* ptr)
{
	unsigned int            fft_block = 0;
	bool                    inbuf_length_warning = false;
	modulator_struct		*mod_ptr = (modulator_struct *)ptr;
	AudioInput				*audio_input = mod_ptr->audio_input;
	SampleVector            audiosamples;
	AMmodulator				ammod;
	int						ifilter {-1};
	
	unique_lock<mutex> am_tx_lock(am_tx_finish); 
	printf("start am_mod_thread\n");
	ammod.init(mod_ptr);
	Fft_calc.plan_fft(nfft_samples * 10); 
	while (!stop_txmod_flag.load())
	{
		const auto startTime = std::chrono::high_resolution_clock::now(); 
		if (vfo.tune_flag == true)
		{
			vfo.tune_flag = false;
			ammod.tune_offset(vfo.get_vfo_offset());
		}
		
		if (mod_ptr->tone)
		{
			audio_input->ToneBuffer(mod_ptr->tone);
			mod_ptr->source_buffer->wait_queue_empty(2);
		}
		if (audio_input->read(audiosamples) == false)
		{
			printf("wait for input\n");
			usleep(1000); // wait 1024 audio sample time
			continue;
		}
		Fft_calc.set_signal_strength(audio_input->get_rms_level());
		//audio_output->write(audiosamples);
		//audio_input->adjust_gain(audiosamples);	
		ammod.process(audiosamples, mod_ptr->ifrate, mod_ptr->source_buffer);
		
		//Fft_calc.set_signal_strength(ammod.get_if_level()); 
		audiosamples.clear();
		const auto now = std::chrono::high_resolution_clock::now();
		const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
		//printf("am mod time %lld\n", timePassed.count());
	}
	printf("am_mod_thread push_end()\n");
	mod_ptr->source_buffer->push_end();
	ammod.am_exit();
	printf("exit am_mod_thread\n");
	pthread_exit(NULL); 
}



int	create_am_tx_thread(modulator_struct *mod_struct)
{
	return pthread_create(&am_mod_pthread, NULL, am_mod_thread, (void *)mod_struct);
}


void start_dsb_tx(int mode, double ifrate, int pcmrate, int tone ,DataBuffer<IQSample16> *source_buffer, AudioInput *audio_input)
{	
	mod_data.source_buffer = source_buffer;
	mod_data.audio_input = audio_input;
	mod_data.pcmrate = pcmrate;
	mod_data.ifrate = ifrate;
	mod_data.tuner_offset = 0;      // not used 
	mod_data.downsample = 0;      //not used
	mod_data.tone = tone;
	source_buffer->restart_queue();
	
	switch (mode)
	{
	case mode_usb:
		mod_data.suppressed_carrier = 1;
		mod_data.mode = LIQUID_AMPMODEM_LSB;
		printf("tx mode LIQUID_AMPMODEM_USB carrier %d\n", mod_data.suppressed_carrier);		
		break;
	case mode_lsb:
		mod_data.suppressed_carrier = 1;
		mod_data.mode = LIQUID_AMPMODEM_USB;
		printf("tx mode LIQUID_AMPMODEM_LSB carrier %d\n", mod_data.suppressed_carrier);		
		break;
	case mode_am:
		mod_data.suppressed_carrier = 0;
		mod_data.mode = LIQUID_AMPMODEM_DSB;
		printf("tx mode LIQUID_AMPMODEM_DSB carrier %d\n", mod_data.suppressed_carrier);		
		break;
	case mode_dsb:
		mod_data.suppressed_carrier = 1;
		mod_data.mode = LIQUID_AMPMODEM_DSB;
		printf("tx mode LIQUID_AMPMODEM_DSB carrier %d\n", mod_data.suppressed_carrier);		
		break;
	default:
		printf("Mode not correct\n");		
		return;
	}
	if (mod_data.tone == 0)
		audio_input->open();
	create_tx_streaming_thread(&soapy_devices[0]);
	create_am_tx_thread(&mod_data);
}