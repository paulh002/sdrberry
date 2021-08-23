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
	float       mod_index          = 0.9f;      // modulation index (bandwidth)
    float		As = 60.0f;       // resampling filter stop-band attenuation [dB]
	float		r = (float) ptr->ifrate / (float)ptr->pcmrate ; 
	
	m_audio_mean = m_audio_rms = m_audio_level = m_if_level = 0.0;
	m_mod = ampmodem_create(mod_index, ptr->mode, ptr->suppressed_carrier);
	set_filter(ptr->ifrate, 4);
	
	if (r > 1.0001)
	{
		printf("resample rate %f \n", r);
		m_bresample = true;
		m_q = msresamp_crcf_create(r, As);
		msresamp_crcf_print(m_q);			
	}
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

void	AMmodulator::process(const SampleVector& samples, double ifrate, DataBuffer<IQSample>	*source_buffer)
{
	IQSampleVector				buf_mod, buf_filter, buf_out;
	unsigned int				num_written, i = 0;
	
	unique_lock<mutex> lock(m_mutex); 
	// Modulate audio to USB, LSB or DSB
	for (auto& col : samples)
	{	complex<float> f;
		ampmodem_modulate(m_mod, col, &f);
		buf_mod.push_back(f);
	}
	double if_rms = rms_level_approx(buf_mod);
	m_if_level = 0.95 * m_if_level + 0.05 * if_rms;
	//printf("if level %f if_rms %f \n", m_if_level, if_rms);
	
	// Apply filter to reduce bandwidth
	for(auto& col : buf_mod)
	{
		complex<float> v;
		iirfilt_crcf_execute(m_lowpass, col, &v);
		buf_filter.push_back(v);
	}	
	Fft_calc.process_samples(buf_filter);
	// convert to output samplerate?
	if (m_bresample)
	{
		buf_out.reserve(m_resample);
		msresamp_crcf_execute(m_q, (complex<float> *)buf_filter.data(), buf_filter.size(), (complex<float> *)buf_out.data(), &num_written);
		buf_out.resize(num_written);
		source_buffer->push(move(buf_out));
	}
	else
	{
		source_buffer->push(move(buf_filter));
	}
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
	int						start_stream = 0;
	
	unique_lock<mutex> am_tx_lock(am_tx_finish); 
	printf("start am_mod_thread\n");
	ammod.init(mod_ptr);
	while (!stop_txmod_flag.load())
	{
		if (mod_ptr->tone)
		{
			audio_input->ToneBuffer();
			start_stream++;
			if (start_stream > 3)
			{
				usleep(20900);
				start_stream = 4;
			}
		}
		if (audio_input->read(audiosamples) == false)
		{
			printf("wait for input\n");
			usleep(22000); // wait 1024 audio sample time
			continue;
		}
		//audio_output->write(audiosamples);
		//audio_input->adjust_gain(audiosamples);	
		ammod.process(audiosamples, mod_ptr->ifrate, mod_ptr->source_buffer);
		
		//Fft_calc.set_signal_strength(ammod.get_if_level()); 
		audiosamples.clear();
	}
	printf("exit am_mod_thread\n");
	pthread_exit(NULL); 
}



int	create_am_tx_thread(modulator_struct *mod_struct)
{
	return pthread_create(&am_mod_pthread, NULL, am_mod_thread, (void *)mod_struct);
}


void start_dsb_tx(int mode, double ifrate, int pcmrate, DataBuffer<IQSample> *source_buffer, AudioInput *audio_input)
{	
	mod_data.source_buffer = source_buffer;
	mod_data.audio_input = audio_input;
	mod_data.pcmrate = pcmrate;
	mod_data.ifrate = ifrate;
	mod_data.tuner_offset = 0;      // not used 
	mod_data.downsample = 0;      //not used
	mod_data.tone = false;
		
	switch (mode)
	{
	case mode_usb:
		mod_data.suppressed_carrier = 1;
		mod_data.mode = LIQUID_AMPMODEM_USB;
		printf("tx mode LIQUID_AMPMODEM_USB carrier %d\n", mod_data.suppressed_carrier);		
		break;
	case mode_lsb:
		mod_data.suppressed_carrier = 1;
		mod_data.mode = LIQUID_AMPMODEM_LSB;
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
	audio_input->open();
	create_am_tx_thread(&mod_data);
}