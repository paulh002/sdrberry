#pragma once

#include <cstdio>	//stdandard output
#include <cstdlib>
#include <alsa/asoundlib.h>
#include <pthread.h>
#include <semaphore.h>

class AudioSink
{
public:
	int			init(int isample_rate, int ichannels);
				AudioSink();
				~AudioSink();
	void		test_tone();
	
	int			write_period(snd_pcm_t *handle, signed short *samples, snd_pcm_channel_area_t *areas);
	void		create_audio_play_thread();
	void		generate_sine(const snd_pcm_channel_area_t *areas, snd_pcm_uframes_t offset, int count, double *_phase);
	
	
	sem_t					audio_mutex1;
	snd_pcm_t				*handle = NULL;
	signed short			*samples;
	snd_pcm_sframes_t		period_size;
	snd_pcm_channel_area_t	*areas;
		
	// period_size is the minimum number of frames necessary to fill the buffer to playback (played in one interupt)
private:	
	
	snd_pcm_uframes_t	frames;
	unsigned int		rate = 44100;
	snd_pcm_sframes_t	buffer_size;
	snd_output_t		*output = NULL;
	int					period_event = 0;
	snd_pcm_format_t	format = SND_PCM_FORMAT_S16;
	unsigned int		channels = 1;
	double				freq = 440;
	int					resample = 1; 
	unsigned int buffer_time = 500000;       /* ring buffer length in us */
	unsigned int period_time = 100000;       /* period time in us */
	char				device[10];			// = "plughw:0,0"; 
	snd_pcm_hw_params_t *hwparams;
	snd_pcm_sw_params_t *swparams;
	pthread_t			audio_threads[2];

	
	int		set_hwparams(snd_pcm_t *handle,	snd_pcm_hw_params_t *params,snd_pcm_access_t access);
	int		set_swparams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams);
	int		write_loop(snd_pcm_t *handle, signed short *samples, snd_pcm_channel_area_t *areas);
	int		direct_write_loop(snd_pcm_t *handle, signed short *samples, snd_pcm_channel_area_t *areas);	
	int		xrun_recovery(snd_pcm_t *handle, int err);
	
};

extern AudioSink	audio_player;

