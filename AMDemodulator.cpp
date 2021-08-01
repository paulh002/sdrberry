#include <cstdio>
#include <cassert>
#include <cmath>
#include <unistd.h>
#include <complex>
#include <vector>
#include <liquid.h>
#include <algorithm>
#include "DataBuffer.h"
#include "AudioOutput.h"
#include "AMDemodulator.h"
#include "Waterfall.h"

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
	float	mod_index  = 0.5f; 
	float	r = (float)ptr->pcmrate / (float)ptr->ifrate; 
	
	// resampler and band filter
	//float rate = (float)downsample / ifrate;
	//m_q = resamp_crcf_create(rate, h_len, bw, slsl, npfb);
	m_source_buffer = ptr->source_buffer;
	m_q = msresamp_crcf_create(r, As);
	msresamp_crcf_print(m_q);	
	m_demod = ampmodem_create(mod_index, ptr->mode, ptr->suppressed_carrier);
	
	m_lowpass = iirfilt_crcf_create_lowpass(6, 0.25);
	m_init = true;
}

AMDemodulator::~AMDemodulator()
{
	if (m_init)
	{
		ampmodem_destroy(m_demod);
		msresamp_crcf_destroy(m_q);
		iirfilt_crcf_destroy(m_lowpass);		
	}
}

void	AMDemodulator::set_filter(long long frequency, int band_width)
{
}

void	AMDemodulator::calc_if_level()
{
	double if_rms = rms_level_approx(m_buf_iffiltered);
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

void	AMDemodulator::process(const IQSampleVector& samples_in, SampleVector& audio)
{
	unsigned int			num_written;
	SampleVector			audio_tmp, audio_mono;
	IQSampleVector			filter;
	
	
	// Downsample to pcmrate (pcmrate will be 44100 or 48000)
	m_buf_iffiltered.reserve(samples_in.size());
	msresamp_crcf_execute(m_q, (complex<float> *)samples_in.data(), samples_in.size(), (complex<float> *)m_buf_iffiltered.data(), &num_written);
	m_buf_iffiltered.resize(num_written);
	calc_if_level();
	
	// apply audio filter set by user [2.2Khz, 2.4Khz, 2.6Khz, 3.0 Khz, ..]
	for(auto& col : m_buf_iffiltered)
	{
		complex<float> v;
		iirfilt_crcf_execute(m_lowpass, col, &v);
		filter.insert(filter.end(), v);
	}
	
	for (auto& col : filter)
	{
		float z {0};
		ampmodem_demodulate(get_am_demod(), (liquid_float_complex)col, &z);
		audio_mono.insert(audio_mono.end(), z);
	}
	mono_to_left_right(audio_mono, audio);
}

static pthread_t		am_thread;

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
	
	ammod.init(demod_ptr);
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
		
		if (iqsamples.size() >= nfft_samples && fft_block == 5)
		{
			fft_block = 0;
			Fft_calc.plan_fft((float *)iqsamples.data());
			Fft_calc.process_samples();
			Fft_calc.set_signal_strength(ammod.get_if_level()); 
		}
		fft_block++;
		
//process
		ammod.process(iqsamples, audiosamples);
		//printf("audiosample %d\n", audiosamples.size());
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


	