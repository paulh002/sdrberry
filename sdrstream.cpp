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

double	rx_sampleRate = 0;

double get_rxsamplerate()
{
	return rx_sampleRate;
}

std::thread					rx_thread;
std::thread					tx_thread;
shared_ptr<RX_Stream>		ptr_rx_stream;
shared_ptr<TX_Stream>		ptr_tx_stream;

void RX_Stream::operator()()
{
	const auto startTime = std::chrono::high_resolution_clock::now();
	auto timeLastPrint = std::chrono::high_resolution_clock::now();
	auto timeLastSpin = std::chrono::high_resolution_clock::now();
	auto timeLastStatus = std::chrono::high_resolution_clock::now();
	unsigned long long totalSamples(0);
	
	int						default_block_length;
	SoapySDR::Stream		*rx_stream;
	int						ret;

	default_block_length = 1024;
	if ((ifrate < 192001) && (ifrate > 48000))
		default_block_length = 2048;
	if ((ifrate < 384001) && (ifrate > 192000))
		default_block_length = 4096;
	if (ifrate > 384001)
		default_block_length = 32768;
	rx_sampleRate = ifrate / 1000000.0;
	printf("default block length is set to %d\n", default_block_length);
	sdr_dev->sdr->setSampleRate(SOAPY_SDR_RX, 0, ifrate);
	rx_stream = sdr_dev->sdr->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32);
	if (rx_stream == NULL)
	{
		fprintf(stderr, "Failed create receive stream\n");
		stop_flag = true;
		return;
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
		try 
		{
			ret = sdr_dev->sdr->readStream(rx_stream, buffs, default_block_length, flags, time_ns, 1e5);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what();
			printf("Error readStream exception\n");
			stop_flag = true;
			continue;
		}
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
			stop_flag = true;
			continue;		
		}
		if (ret > 0)
		{
			buf.resize(ret);
			sdr_dev->channel_structure_rx[0].source_buffer_rx->push(move(buf));
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
		timeLastPrint = now;
		const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
		const auto sampleRate = double(totalSamples) / timePassed.count();
		rx_sampleRate = sampleRate;
	}
	sdr_dev->channel_structure_rx[0].source_buffer_rx->push_end();
	sdr_dev->sdr->deactivateStream(rx_stream);
	sdr_dev->sdr->closeStream(rx_stream);
}

RX_Stream::RX_Stream(struct device_structure *dev)
{
	sdr_dev = dev;
}

bool RX_Stream::create_rx_streaming_thread(struct device_structure *dev)
{	
	if (ptr_rx_stream != nullptr)
		return false;
	ptr_rx_stream = make_shared<RX_Stream>(dev);
	rx_thread = std::thread(&RX_Stream::operator(), ptr_rx_stream);
	return true;
}

void RX_Stream::destroy_rx_streaming_thread()
{
	if (ptr_rx_stream == nullptr)
		return;
	stop_flag = true;
	rx_thread.join();
	ptr_rx_stream.reset();
}
	
void TX_Stream::operator()()
{
	const auto startTime = std::chrono::high_resolution_clock::now();
	auto timeLastPrint = std::chrono::high_resolution_clock::now();
	auto timeLastSpin = std::chrono::high_resolution_clock::now();
	auto timeLastStatus = std::chrono::high_resolution_clock::now();
	unsigned long long totalSamples(0);
	
	SoapySDR::Stream		*tx_stream;
	int ret;

	try 
	{
		tx_stream = sdr_dev->sdr->setupStream(SOAPY_SDR_TX, SOAPY_SDR_CS16);
	}
	catch (const std::exception& e)
	{
		std::cout << e.what();
		return;
	}
	if (tx_stream == NULL)
	{
		fprintf(stderr, "Failed create receive stream\n");
		return;
	}

	sdr_dev->sdr->setSampleRate(SOAPY_SDR_TX, 0, ifrate);
	sdr_dev->sdr->setBandwidth(SOAPY_SDR_TX, 0, 0.1);
	sdr_dev->sdr->setAntenna(SOAPY_SDR_TX, 0, string("A"));
	sdr_dev->sdr->setFrequency(SOAPY_SDR_TX, 0, (double)vfo.get_tx_frequency());		
	sdr_dev->sdr->setGain(SOAPY_SDR_TX, 0, sdr_dev->channel_structure_tx[0].gain);
	while (1)
	{
		unsigned int				overflows(0);
		unsigned int				underflows(0); 
		int							flags(0); //SOAPY_SDR_END_BURST 
		long long					time_ns(0);
		int							samples_transmit;
		
		IQSampleVector16 iqsamples = sdr_dev->channel_structure_tx[0].source_buffer_tx->pull();
		if (iqsamples.empty())
		{
			// an empty vector is send when the stream is closed by suppying process
			// source_buffer->push_end()
			printf("Received Push_End Exit writeStream\n");
			sdr_dev->sdr->setGain(SOAPY_SDR_TX, 0, 0.0);
			sdr_dev->sdr->deactivateStream(tx_stream);
			sdr_dev->sdr->closeStream(tx_stream);
			pthread_exit(NULL);
		}
		//printf("samples %d %d %d \n", iqsamples.size(), iqsamples[0].real(), iqsamples[0].imag());
		samples_transmit = iqsamples.size();
		void *buffs[] = { iqsamples.data() };
		do
		{
			ret = sdr_dev->sdr->writeStream(tx_stream, buffs, samples_transmit, flags, time_ns, 1e5);
			//printf("send samples %d %d\n", ret, samples_transmit);
			if (ret > 0)
			{
				totalSamples += ret;
				samples_transmit -= ret;
				complex<int16_t> *f = iqsamples.data();
				buffs[0] = &f[iqsamples.size() - samples_transmit];
			}
		} while ((ret > 0) && (samples_transmit > 0));
		
		if(ret == SOAPY_SDR_TIMEOUT) 
		{
			continue;
		}
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
			sdr_dev->sdr->deactivateStream(tx_stream);
			sdr_dev->sdr->closeStream(tx_stream);
			iqsamples.clear();
			return;			
		}
		const auto now = std::chrono::high_resolution_clock::now();
		if (timeLastPrint + std::chrono::seconds(5) < now)
		{
			timeLastPrint = now;
			const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
			const auto sampleRate = double(totalSamples) / timePassed.count();
			printf("tx \b%g Msps\t%g MBps", sampleRate, sampleRate * 1*sizeof(complex<float>));
			if (overflows != 0) printf("\tOverflows %u", overflows);
			if (underflows != 0) printf("\tUnderflows %u", underflows);
			printf("\n ");
		}
		iqsamples.clear();		
	}
	printf("Exit writeStream\n");
	sdr_dev->sdr->setGain(SOAPY_SDR_TX, 0, 0.0);
	sdr_dev->sdr->deactivateStream(tx_stream);
	sdr_dev->sdr->closeStream(tx_stream);
}

TX_Stream::TX_Stream(struct device_structure *dev)
{
	sdr_dev = dev;
}

bool TX_Stream::create_tx_streaming_thread(struct device_structure *dev)
{	
	if (ptr_tx_stream != nullptr)
		return false;
	ptr_tx_stream = make_shared<TX_Stream>(dev);
	tx_thread = std::thread(&TX_Stream::operator(), ptr_tx_stream);
	return true;
}

void TX_Stream::destroy_tx_streaming_thread()
{
	if (ptr_tx_stream == nullptr)
		return;
	stop_flag = true;
	tx_thread.join();
	ptr_tx_stream.reset();
}