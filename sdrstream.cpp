#include <unistd.h>
#include <pthread.h>
#include <mutex>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <math.h>
#include <cstdio>
#include <cstdlib>
#include <complex.h>
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

#include "DataBuffer.h"
#include "sdrberry.h"

using namespace std;

void* rx_streaming_thread(void* psdr_dev)
{
	static const int		default_block_length {32768}; //32768
	SoapySDR::Stream		*rx_stream;
	
	unique_lock<mutex> lock_stream(stream_finish); 
	const auto startTime = std::chrono::high_resolution_clock::now();
	auto timeLastPrint = std::chrono::high_resolution_clock::now();
	auto timeLastSpin = std::chrono::high_resolution_clock::now();
	auto timeLastStatus = std::chrono::high_resolution_clock::now();
	unsigned long long totalSamples(0);
	
	struct device_structure *sdr_dev = (struct device_structure *)psdr_dev;
	int ret;

	try 
	{
		rx_stream = sdr_dev->sdr->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32);
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
		unsigned int				overflows(0);
		unsigned int				underflows(0); 
		int							flags(0); 
		long long					time_ns(0);
		vector<complex<float>>		buf(default_block_length);
		
		void *buffs[] = { buf.data() };
		ret = sdr_dev->sdr->readStream(rx_stream, buffs, default_block_length, flags, time_ns, 1e5);
		if (ret == SOAPY_SDR_TIMEOUT) continue;
		if (ret == SOAPY_SDR_OVERFLOW)
		{
			overflows++;
			continue;
		}
		if (ret == SOAPY_SDR_UNDERFLOW)
		{
			underflows++;
			continue;
		}
		if (ret < 0)
		{
			printf("Error readStream\n");
			pthread_exit(NULL);			
		}
		if (ret > 0)
		{
			//if (buf[0].imag() < 0.00000000001 &&  buf[0].imag() > -0.00000000001)
			//	{
			//  printf("samples %d %f %f \n", ret, buf[0].real() , buf[0].imag() );
			//	}
			buf.resize(ret);
			sdr_dev->channel_structure_rx[0].source_buffer->push(move(buf));	
		}
		
		totalSamples += ret;
		const auto now = std::chrono::high_resolution_clock::now();
		if (timeLastStatus + std::chrono::seconds(1) < now)
		{
			timeLastStatus = now;
			while (true)
			{
				size_t chanMask; int flags; long long timeNs;
				ret = sdr_dev->sdr->readStreamStatus(rx_stream, chanMask, flags, timeNs, 0);
				if (ret == SOAPY_SDR_OVERFLOW) overflows++;
				else if (ret == SOAPY_SDR_UNDERFLOW) underflows++;
				else if (ret == SOAPY_SDR_TIME_ERROR) {}
				else break;
			}
		}
		if (timeLastPrint + std::chrono::seconds(5) < now)
		{
			timeLastPrint = now;
			const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
			const auto sampleRate = double(totalSamples) / timePassed.count();
			printf("\b%g Msps\t%g MBps", sampleRate, sampleRate*1*sizeof(complex<float>));
			if (overflows != 0) printf("\tOverflows %u", overflows);
			if (underflows != 0) printf("\tUnderflows %u", underflows);
			printf("\n ");
		}
	}
	sdr_dev->channel_structure_rx[0].source_buffer->push_end();
	sdr_dev->sdr->deactivateStream(rx_stream);
	sdr_dev->sdr->closeStream(rx_stream);
	pthread_exit(NULL);
}

int create_rx_streaming_thread(struct device_structure *sdr_dev)
{
	return pthread_create(&sdr_dev->channel_structure_rx[0].thread, NULL, rx_streaming_thread, (void *)sdr_dev);
}

void stream_rx_set_frequency(struct device_structure *sdr_dev,unsigned long freq)
{
	if (sdr_dev->sdr != NULL)
	{
		sdr_dev->sdr->setFrequency(SOAPY_SDR_RX, 0, freq);		
	}
}

void stream_tx_set_frequency(struct device_structure *sdr_dev, unsigned long freq)
{
	if (sdr_dev->sdr != NULL)
	{
		sdr_dev->sdr->setFrequency(SOAPY_SDR_TX, 0, freq);	
	}
}

void* tx_streaming_thread(void* psdr_dev)
{
	static const int		default_block_length {32768}; //32768
	SoapySDR::Stream		*tx_stream;
	
	unique_lock<mutex> lock_stream(stream_finish); 
	const auto startTime = std::chrono::high_resolution_clock::now();
	auto timeLastPrint = std::chrono::high_resolution_clock::now();
	auto timeLastSpin = std::chrono::high_resolution_clock::now();
	auto timeLastStatus = std::chrono::high_resolution_clock::now();
	unsigned long long totalSamples(0);
	
	struct device_structure *sdr_dev = (struct device_structure *)psdr_dev;
	int ret;

	try 
	{
		tx_stream = sdr_dev->sdr->setupStream(SOAPY_SDR_TX, SOAPY_SDR_CF32);
	}
	catch (const std::exception& e)
	{
		std::cout << e.what();
		pthread_exit(NULL);
	}
	if (tx_stream == NULL)
	{
		fprintf(stderr, "Failed create receive stream\n");
		pthread_exit(NULL);
	}

	sdr_dev->sdr->activateStream(tx_stream, 0, 0, 0);
	while (!stop_flag.load())
	{
		unsigned int				overflows(0);
		unsigned int				underflows(0); 
		int							flags(0); 
		long long					time_ns(0);
		
		
		IQSampleVector iqsamples = sdr_dev->channel_structure_tx[0].source_buffer->pull();		
		//printf("samples %d %f %f \n", iqsamples.size()/2, iqsamples[0].real(), iqsamples[0].imag());
		void *buffs[] = { iqsamples.data() };
		ret = sdr_dev->sdr->writeStream(tx_stream, buffs, iqsamples.size()/2, flags, time_ns, 1e5);
		if (ret == SOAPY_SDR_TIMEOUT) continue;
		if (ret == SOAPY_SDR_OVERFLOW)
		{
			overflows++;
			continue;
		}
		if (ret == SOAPY_SDR_UNDERFLOW)
		{
			underflows++;
			continue;
		}
		if (ret < 0)
		{
			printf("Error writeStream\n");
			pthread_exit(NULL);			
		}
		iqsamples.clear();
		
		totalSamples += ret;
		const auto now = std::chrono::high_resolution_clock::now();
		if (timeLastStatus + std::chrono::seconds(1) < now)
		{
			timeLastStatus = now;
			while (true)
			{
				size_t chanMask; int flags; long long timeNs;
				ret = sdr_dev->sdr->readStreamStatus(tx_stream, chanMask, flags, timeNs, 0);
				if (ret == SOAPY_SDR_OVERFLOW) overflows++;
				else if (ret == SOAPY_SDR_UNDERFLOW) underflows++;
				else if (ret == SOAPY_SDR_TIME_ERROR) {}
				else break;
			}
		}
		if (timeLastPrint + std::chrono::seconds(5) < now)
		{
			timeLastPrint = now;
			const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
			const auto sampleRate = double(totalSamples) / timePassed.count();
			printf("\b%g Msps\t%g MBps", sampleRate, sampleRate * 1*sizeof(complex<float>));
			if (overflows != 0) printf("\tOverflows %u", overflows);
			if (underflows != 0) printf("\tUnderflows %u", underflows);
			printf("\n ");
		}
	}
	printf("Exit writeStream\n");
	sdr_dev->sdr->deactivateStream(tx_stream);
	sdr_dev->sdr->closeStream(tx_stream);
	pthread_exit(NULL);
}

int create_tx_streaming_thread(struct device_structure *sdr_dev)
{
	return pthread_create(&sdr_dev->channel_structure_tx[0].thread, NULL, tx_streaming_thread, (void *)sdr_dev);
}