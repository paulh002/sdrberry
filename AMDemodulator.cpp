#include <cstdio>
#include <cassert>
#include <cmath>
#include <unistd.h>
#include <complex>
#include <vector>
#include <liquid.h>
#include <algorithm>
#include <mutex>
#include "DataBuffer.h"
#include "AudioOutput.h"
#include "AMDemodulator.h"
#include "Waterfall.h"
#include "sdrberry.h"
#include "vfo.h"

void* am_demod_thread(void* ptr);
/** Compute RMS level over a small prefix of the specified sample vector. */

static IQSample::value_type rms_level_approx(const IQSampleVector& samples)
{
	unsigned int n = samples.size();
	n = (n + 63) / 64;

	IQSample::value_type level = 0;
	for (unsigned int i = 0; i < n; i++) {
		const IQSample& s = samples[i];
		IQSample::value_type re = s.real(), im = s.imag();
		level += re * re + im * im;
	}

	return sqrt(level / n);
}


void	AMDemodulator::init(demod_struct * ptr)
{
	float	As = 60.0f;      // resampling filter stop-band attenuation [dB]
	float	mod_index  = 0.8f; 
	float	r = (float)ptr->pcmrate / (float)ptr->ifrate; 
	
	// resampler and band filter
	m_source_buffer = ptr->source_buffer;
	if (r < 0.5)
	{
		printf("resample rate %f \n", r);
		m_bresample = true;
		m_q = msresamp_crcf_create(r, As);
		msresamp_crcf_print(m_q);			
	}
	m_demod = ampmodem_create(mod_index, ptr->mode, ptr->suppressed_carrier);
	
	m_lowpass = iirfilt_crcf_create_lowpass(6, 0.0625);
	m_init = true;
}

void AMDemodulator::adjust_gain(IQSampleVector& samples_in, float vol)
{
	for (auto& col : samples_in)
	{
		col.real(col.real() * vol);
		col.imag(col.imag() * vol);
	}
}

AMDemodulator::~AMDemodulator()
{
	if (m_init)
	{
		ampmodem_destroy(m_demod);
		if (m_q)
			msresamp_crcf_destroy(m_q);
		iirfilt_crcf_destroy(m_lowpass);		
	}
}

void	AMDemodulator::set_filter(long long frequency, int band_width)
{
}

void	AMDemodulator::calc_if_level(const IQSampleVector& samples_in)
{
	double if_rms = rms_level_approx(samples_in);
	m_if_level = 0.95 * m_if_level + 0.05 * if_rms;
}

void AMDemodulator::mono_to_left_right(const SampleVector& samples_mono,
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

void	AMDemodulator::process(const IQSampleVector&	samples_in, SampleVector& audio)
{
	unsigned int			num_written;
	SampleVector			audio_tmp, audio_mono;
	IQSampleVector			filter;
	
	
	// Downsample to pcmrate (pcmrate will be 44100 or 48000)
	m_buf_iffiltered.reserve(samples_in.size());
	if (m_bresample)
	{
		msresamp_crcf_execute(m_q, (complex<float> *)samples_in.data(), samples_in.size(), (complex<float> *)m_buf_iffiltered.data(), &num_written);
		m_buf_iffiltered.resize(num_written);
	}
	else
		m_buf_iffiltered.insert(m_buf_iffiltered.begin(), samples_in.begin(), samples_in.end());
	
	// apply audio filter set by user [2.2Khz, 2.4Khz, 2.6Khz, 3.0 Khz, ..]
    for(auto& col : m_buf_iffiltered)
	{
		complex<float> v;
		iirfilt_crcf_execute(m_lowpass, col, &v);
		filter.insert(filter.end(), v);
	}
	calc_if_level(filter);
	    
	for (auto& col : filter)  
	{
		float z {0};
		ampmodem_demodulate(get_am_demod(), (liquid_float_complex)col, &z);
		audio_mono.insert(audio_mono.end(), z);
	}
	mono_to_left_right(audio_mono, audio);
	m_buf_iffiltered.clear();
	filter.clear();
}

pthread_t		am_thread;

int	create_am_thread(demod_struct *demod)
{
	return pthread_create(&am_thread, NULL, am_demod_thread, (void *)demod);
}


void* am_demod_thread(void* ptr)
{
	unsigned int            fft_block = 0;
	bool                    inbuf_length_warning = false;
	demod_struct			*demod_ptr = (demod_struct *)ptr;
	AudioOutput             *audio_output = demod_ptr->audio_output;
	SampleVector            audiosamples;
	AMDemodulator			ammod;
	
	unique_lock<mutex> lock(am_finish); 
	vfo.set_tuner_offset(0);
	ammod.init(demod_ptr);
	Fft_calc.plan_fft(512); //
	while (!stop_flag.load())
	{
		if (!inbuf_length_warning && ammod.m_source_buffer->queued_samples() > 10 * 530000) {
			printf("\nWARNING: Input buffer is growing (system too slow) queued samples %u\n", ammod.m_source_buffer->queued_samples());
			inbuf_length_warning = true;
		}
		
		if (ammod.m_source_buffer->queued_samples() == 0)
		{
			usleep(5000);
			continue;
		}
		
		IQSampleVector iqsamples = ammod.m_source_buffer->pull();
		if (iqsamples.empty())
		{
			usleep(5000);
			continue;
		}
		Fft_calc.process_samples(iqsamples);
		Fft_calc.set_signal_strength(ammod.get_if_level()); 
		
//process
		ammod.process(iqsamples, audiosamples);
		// Measure audio level.
		samples_mean_rms(audiosamples, ammod.m_audio_mean, ammod.m_audio_rms);
		ammod.m_audio_level = 0.95 * ammod.m_audio_level + 0.05 * ammod.m_audio_rms;

		// Set nominal audio volume.
		audio_output->adjust_gain(audiosamples);	
		audio_output->write(audiosamples);
		iqsamples.clear();
		audiosamples.clear();
	}
	pthread_exit(NULL); 
}

static demod_struct	demod;

void start_dsb(int mode, double ifrate, int pcmrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output)
{	
	demod.source_buffer = source_buffer;
	demod.audio_output = audio_output;
	demod.pcmrate = pcmrate;
	demod.ifrate = ifrate;
	demod.tuner_offset = 0;    // not used 
	demod.downsample = 0;    //not used
		
	printf("pcmrate %u\n", demod.pcmrate);
	printf("ifrate %f\n", demod.ifrate);
				
	switch (mode)
	{
	case mode_usb:
		demod.suppressed_carrier = 1;
		demod.mode = LIQUID_AMPMODEM_USB;
		printf("mode LIQUID_AMPMODEM_USB carrier %d\n", demod.suppressed_carrier);		
		break;
	case mode_lsb:
		demod.suppressed_carrier = 1;
		demod.mode = LIQUID_AMPMODEM_LSB;
		printf("mode LIQUID_AMPMODEM_LSB carrier %d\n", demod.suppressed_carrier);		
		break;
	case mode_am:
		demod.suppressed_carrier = 0;
		demod.mode = LIQUID_AMPMODEM_DSB;
		printf("mode LIQUID_AMPMODEM_DSB carrier %d\n", demod.suppressed_carrier);		
		break;
	case mode_dsb:
		demod.suppressed_carrier = 1;
		demod.mode = LIQUID_AMPMODEM_DSB;
		printf("mode LIQUID_AMPMODEM_DSB carrier %d\n", demod.suppressed_carrier);		
		break;
	default:
		printf("Mode not correct\n");		
		return;
	}
	create_am_thread(&demod);
}
	