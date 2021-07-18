#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <math.h>
#include <cstdio>
#include <cstdlib>
#include <complex.h>
#include "wstring.h"
#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include "devices.h"
#include "sdrstream.h"
#include "vfo.h"
#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <SoapySDR/Types.h>
#include <pthread.h>
#include <semaphore.h>

#include <iostream>

std::complex<float> rx_buff1[1024], rx_buff2[1024];
std::complex<float> tx_buff1[1024], tx_buff2[1024]; 

sem_t		rx_buffer_mutex1;
pthread_t	threads[NUM_THREADS];


void* rx_streaming_thread(void* psdr_dev)
{
	struct device_structure *sdr_dev = (struct device_structure *)psdr_dev;
		
	SoapySDR::Stream *rx_stream = sdr_dev->sdr->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32);
	if (rx_stream == NULL)
	{
		fprintf(stderr, "Failed create receive stream\n");
		pthread_exit(NULL);
	}
	sdr_dev->sdr->activateStream(rx_stream, 0, 0, 0);

	while (1)
	{
		// Read buffer and signal output process to process it
		// Use 2 buffers which will be swapped every time
		void *buffs1[] = { rx_buff1 };
		void *buffs2[] = { rx_buff2 };
		int flags; 
		int ii = 0;
		long long time_ns;
		
		if (ii == 0)
		{
			int ret = sdr_dev->sdr->readStream(rx_stream, buffs1, 1024, flags, time_ns, 1e5);
			ii = 1;	
		}
		else
		{
			int ret = sdr_dev->sdr->readStream(rx_stream, buffs2, 1024, flags, time_ns, 1e5);
			ii = 0;				
		}
		
	}
}

void create_rx_streaming_thread(struct device_structure *sdr_dev, vfo_settings_struct	*vfo, double samplerate)
{
	sem_init(&rx_buffer_mutex1, 0, 1);
	
	
	sdr_dev->sdr->setSampleRate(SOAPY_SDR_RX, 0, samplerate);
	int i = RX_THREAD;
	int rc = pthread_create(&threads[RX_THREAD], NULL, rx_streaming_thread, (void *)sdr_dev);
	
}

void stream_rx_set_frequency(struct device_structure *sdr_dev,unsigned long freq)
{
	if (sdr_dev->sdr != NULL)
		sdr_dev->sdr->setFrequency(SOAPY_SDR_RX, 0, freq);		
}