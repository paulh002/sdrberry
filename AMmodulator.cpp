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
	float       mod_index          = 0.8f;      // modulation index (bandwidth)
    float		As = 60.0f;       // resampling filter stop-band attenuation [dB]
	float		r = (float) ptr->ifrate / (float)ptr->pcmrate ; 
	
	m_audio_mean = m_audio_rms = m_audio_level = m_if_level = 0.0;
	m_mod = ampmodem_create(mod_index, ptr->mode, ptr->suppressed_carrier);
	set_filter(ptr->ifrate, 5);
	
	if (r > 1.0001)
	{
		printf("resample rate %f \n", r);
		m_bresample = true;
		m_q = msresamp_crcf_create(r, As);
		msresamp_crcf_print(m_q);			
	}
}

void	AMmodulator::set_filter(double if_rate, int band_width)
{
	double	factor {0.0625};
	
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
	unsigned int				num_written;
	
	unique_lock<mutex> lock(m_mutex); 
	buf_mod.reserve(samples.size());
	buf_mod.resize(samples.size());
	for (auto& col : buf_mod)
	{
		col.real(0.0);
		col.imag(0.0);
	}
	int i = 0;
	// Modulate audio to USB, LSB or DSB
	
	
	for (auto& col : samples)
	{
		ampmodem_modulate(m_mod, col, &buf_mod[i++]);
	}
	
	double if_rms = rms_level_approx(buf_mod);
	m_if_level = 0.95 * m_if_level + 0.05 * if_rms;
	//printf("if level %f if_rms %f \n", m_if_level, if_rms);
	printf("level %f\n", if_rms);
	// Apply filter to reduce bandwidth
	for(auto& col : buf_mod)
	{
		complex<float> v;
		iirfilt_crcf_execute(m_lowpass, col, &v);
		buf_filter.insert(buf_filter.end(), v);
	}	
	


	// convert to output samplerate?
	if (m_bresample)
	{
		buf_out.reserve(m_resample);
		msresamp_crcf_execute(m_q, (complex<float> *)buf_filter.data(), buf_filter.size(), (complex<float> *)buf_out.data(), &num_written);
		buf_out.resize(num_written);
		//source_buffer->push(move(buf_out));
	}
	//else
		//source_buffer->push(move(buf_filter));
	
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
	
	unique_lock<mutex> lock(am_tx_finish); 
	ammod.init(mod_ptr);
	while (!stop_flag.load())
	{
		if (audio_input->read(audiosamples) == false)
		{
			continue;
		}
		audio_output->write(audiosamples);
		//audio_input->adjust_gain(audiosamples);	
		//ammod.process(audiosamples, mod_ptr->ifrate, mod_ptr->source_buffer);
		//Fft_calc.set_signal_strength(ammod.get_if_level()); 
		
		// transmitdata
		
		audiosamples.clear();
	}
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
		
	printf("pcmrate tx %u\n", mod_data.pcmrate);
	printf("ifrate tx %f\n", mod_data.ifrate);
				
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
	create_am_tx_thread(&mod_data);
}