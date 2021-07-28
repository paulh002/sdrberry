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
#include <iostream>
#include "linear_ringbuffer.hpp"


using namespace std;
using namespace bev;

pthread_t				threads[NUM_THREADS];

void* rx_streaming_thread(void* psdr_dev)
{
	static const int		default_block_length = 65536;
	SoapySDR::Stream		*rx_stream;
	size_t					mtu;
	
	
	struct device_structure *sdr_dev = (struct device_structure *)psdr_dev;
	int ret;
	
	try 
	{
		rx_stream = sdr_dev->sdr->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32);
		mtu = sdr_dev->sdr->getStreamMTU(rx_stream);
	}
	catch (const std::exception& e)
	{
		std::cout << e.what();
		pthread_exit(NULL);
	}
	if (rx_stream == NULL)
	{
		fprintf(stderr, "Failed create receive stream\n");
		pthread_exit(NULL);
	}
	sdr_dev->sdr->activateStream(rx_stream, 0, 0, 0);
	while (!stop_flag.load())
	{
		int flags; 
		long long time_ns;
		void	*ptr = rbuffer.write_head();
		void *buffs[] = { ptr };
		
		if (rbuffer.free_size() < mtu)
			continue;
		
		ret = sdr_dev->sdr->readStream(rx_stream, buffs, default_block_length, flags, time_ns, 1e5);
		if (ret < 0)
		{
			printf("cannot stream data \n");
			pthread_exit(NULL);
		}
		rbuffer.commit(ret * sizeof(complex<float>));
		printf("readStream %d \n", ret);
		pthread_yield();
	}
	sdr_dev->sdr->deactivateStream(rx_stream);
	pthread_exit(NULL);
}

void create_rx_streaming_thread(struct device_structure *sdr_dev, vfo_settings_struct	*vfo, double samplerate)
{
	//sdr_dev->channel_structure_rx[0].source_buffer = source_buffer;
	sdr_dev->sdr->setSampleRate(SOAPY_SDR_RX, 0, samplerate);
	int i = RX_THREAD;
	int rc = pthread_create(&threads[RX_THREAD], NULL, rx_streaming_thread, (void *)sdr_dev);
	
}

void stream_rx_set_frequency(struct device_structure *sdr_dev,unsigned long freq)
{
	if (sdr_dev->sdr != NULL)
		sdr_dev->sdr->setFrequency(SOAPY_SDR_RX, 0, freq);		
}


/*
 *int		size_of_samples_counter = 0;
 *if (size_of_samples_counter > 100)
{
	size_of_samples_counter = 0;
	printf("samples %d source buffer %d queu length %zu\n",  ret, sdr_dev->channel_structure_rx[0].source_buffer->size(),
		sdr_dev->channel_structure_rx[0].source_buffer->get_qlen());
}
size_of_samples_counter++; */