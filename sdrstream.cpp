#include <unistd.h>
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
#include "sdrstream.h"
#include "vfo.h"
#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <SoapySDR/Types.h>
#include <iostream>
#include "SdrDevice.h"
#include "DataBuffer.h"
#include "sdrberry.h"

using namespace std;

double	rx_sampleRate = 0;
double	tx_sampleRate = 0;

double get_rxsamplerate()
{
	return rx_sampleRate;
}

double get_txsamplerate()
{
	return tx_sampleRate;

}
std::thread					rx_thread;
std::thread					tx_thread;
shared_ptr<RX_Stream>		ptr_rx_stream;
shared_ptr<TX_Stream>		ptr_tx_stream;
std::mutex					rxstream_mutex;

void RX_Stream::operator()()
{
	const auto startTime = std::chrono::high_resolution_clock::now();
	auto timeLastPrint = std::chrono::high_resolution_clock::now();
	auto timeLastSpin = std::chrono::high_resolution_clock::now();
	auto timeLastStatus = std::chrono::high_resolution_clock::now();
	unsigned long long		totalSamples(0);
	unique_lock<mutex>		lock_rx(rxstream_mutex);
	
	int						default_block_length;
	SoapySDR::Stream		*rx_stream;
	int						ret;

	default_block_length = 1024;
	if ((ifrate < 192001) && (ifrate > 48000))
		default_block_length = 2048;
	if ((ifrate < 384001) && (ifrate > 192000))
		default_block_length = 8192;
	if (ifrate > 384001)
		default_block_length = 32768/2;
	rx_sampleRate = ifrate / 1000000.0;
	printf("default block length is set to %d ifrate %f\n", default_block_length, ifrate);
	try
	{
		rx_stream = SdrDevices.SdrDevices.at(radio)->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32);
		SdrDevices.SdrDevices.at(radio)->setSampleRate(SOAPY_SDR_RX, channel, ifrate);
		SdrDevices.SdrDevices.at(radio)->activateStream(rx_stream);
		SdrDevices.SdrDevices.at(radio)->setGain(SOAPY_SDR_RX, 0, gbar.get_rf_gain());
	}
	catch (const std::exception& e)
	{
		std::cout << e.what();
		stop_flag = true;
		return;
	}
	stop_flag = false;
	while (!stop_flag.load())
	{
		unsigned int				overflows(0);
		unsigned int				underflows(0); 
		int							flags(0); 
		long long					time_ns(0);
		vector<complex<float>>		buf(default_block_length);
		vector<complex<float>>		buf2(default_block_length);

		void *buffs[] = { buf.data(), buf2.data() };
		try 
		{
			ret = SdrDevices.SdrDevices.at(radio)->readStream(rx_stream, buffs, default_block_length, flags, time_ns, 1e5);
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
			m_source_buffer->push(move(buf));
		}
		
		totalSamples += ret;
		const auto now = std::chrono::high_resolution_clock::now();
		if (timeLastStatus + std::chrono::seconds(1) < now)
		{
			timeLastStatus = now;
			while (true)
			{
				size_t chanMask; int flags; long long timeNs;
				try
				{
					ret = SdrDevices.SdrDevices.at(radio)->readStreamStatus(rx_stream, chanMask, flags, timeNs, 0);
				}
				catch (const std::exception& e)
				{
					std::cout << e.what();
				}
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
	m_source_buffer->clear(); //push_end();
	try
	{
		SdrDevices.SdrDevices.at(radio)->deactivateStream(rx_stream);
		SdrDevices.SdrDevices.at(radio)->closeStream(rx_stream);
	}
	catch (const std::exception& e)
	{
		std::cout << e.what();
	}
}

RX_Stream::RX_Stream(std::string sradio, int chan, DataBuffer<IQSample> *source_buffer)
{
	radio = sradio;
	channel = chan;
	m_source_buffer = source_buffer;
}

bool RX_Stream::create_rx_streaming_thread(std::string sradio, int chan, DataBuffer<IQSample> *source_buffer)
{	
	if (ptr_rx_stream != nullptr)
		return false;
	ptr_rx_stream = make_shared<RX_Stream>(sradio, chan, source_buffer);
	rx_thread = std::thread(&RX_Stream::operator(), ptr_rx_stream);
	return true;
}

void RX_Stream::destroy_rx_streaming_thread()
{
	auto startTime = std::chrono::high_resolution_clock::now();
	
	if (ptr_rx_stream == nullptr)
		return;
	ptr_rx_stream->stop_flag = true;
	rx_thread.join();
	ptr_rx_stream.reset();

	auto now = std::chrono::high_resolution_clock::now();
	const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
	cout << "Stoptime RX_Stream:" << timePassed.count() << endl;
}
	
void TX_Stream::operator()()
{
	const auto startTime = std::chrono::high_resolution_clock::now();
	auto timeLastPrint = std::chrono::high_resolution_clock::now();
	auto timeLastSpin = std::chrono::high_resolution_clock::now();
	auto timeLastStatus = std::chrono::high_resolution_clock::now();
	unsigned long long totalSamples(0);
	
	IQSampleVector16		iqsamples;
	SoapySDR::Stream		*tx_stream;
	int ret;
	complex<int16_t> *f{nullptr};

	try
	{
		SdrDevices.SdrDevices.at(radio)->setGain(SOAPY_SDR_TX, 0, Gui_tx.get_drv_pos());
		tx_stream = SdrDevices.SdrDevices.at(radio)->setupStream(SOAPY_SDR_TX, SOAPY_SDR_CS16);
		SdrDevices.SdrDevices.at(radio)->setSampleRate(SOAPY_SDR_TX, 0, m_ifrate);
		//SdrDevices.SdrDevices.at(radio)->setBandwidth(SOAPY_SDR_TX, 0, m_ifrate); //0.1
		//SdrDevices.SdrDevices.at(radio)->setAntenna(SOAPY_SDR_TX, 0, string("A"));
		//SdrDevices.SdrDevices.at(radio)->setFrequency(SOAPY_SDR_TX, 0, (double)vfo.get_tx_frequency());		
	}
	catch (const std::exception& e)
	{
		printf("Failed create receive stream\n");
		std::cout << e.what();
		return;
	}
	
	while (!stop_flag.load())
	{
		unsigned int				overflows(0);
		unsigned int				underflows(0); 
		int							flags(0); //SOAPY_SDR_END_BURST 
		long long					time_ns(0);
		int							samples_transmit;

		iqsamples = m_source_buffer->pull();
		if (iqsamples.empty())
		{
			// an empty vector is send when the stream is closed by suppying process
			// source_buffer->push_end()
			printf("Received Push_End Exit writeStream\n");
			try
			{
				SdrDevices.SdrDevices.at(radio)->setSampleRate(SOAPY_SDR_RX, 0, m_ifrate);
				//SdrDevices.SdrDevices.at(radio)->setGain(SOAPY_SDR_TX, 0, 0.0);
				SdrDevices.SdrDevices.at(radio)->deactivateStream(tx_stream);
				SdrDevices.SdrDevices.at(radio)->closeStream(tx_stream);
			}
			catch (const std::exception& e)
			{
				std::cout << e.what();
				return;
			}
			return;
		}
		//printf("samples %d %d %d \n", iqsamples.size(), iqsamples[0].real(), iqsamples[0].imag());
		samples_transmit = iqsamples.size();
		void *buffs[] = { iqsamples.data() };
		do
		{
			ret = SdrDevices.SdrDevices.at(radio)->writeStream(tx_stream, buffs, samples_transmit, flags, time_ns, 1e5);
			//printf("send samples %d %d\n", ret, samples_transmit);
			if (ret > 0)
			{
				totalSamples += ret;
				samples_transmit -= ret;
				f = iqsamples.data();
				buffs[0] = &f[iqsamples.size() - samples_transmit];
			}
		} while ((ret > 0) && (samples_transmit > 0) && !stop_flag.load());
		
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
			try
			{
				SdrDevices.SdrDevices.at(radio)->deactivateStream(tx_stream);
				SdrDevices.SdrDevices.at(radio)->closeStream(tx_stream);
			}
			catch (const std::exception& e)
			{
				std::cout << e.what();
			}
			iqsamples.clear();
			return;			
		}
		const auto now = std::chrono::high_resolution_clock::now();
		timeLastPrint = now;
		const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
		const auto sampleRate = double(totalSamples) / timePassed.count();
		tx_sampleRate = sampleRate;
		iqsamples.clear();		
	}
	printf("Exit writeStream\n");
	bool bempty = false;
	SdrDevices.SdrDevices.at(radio)->setSampleRate(SOAPY_SDR_RX, 0, m_ifrate);
	m_source_buffer->clear();
	try
	{
		SdrDevices.SdrDevices.at(radio)->deactivateStream(tx_stream);
		SdrDevices.SdrDevices.at(radio)->closeStream(tx_stream);
	}
	catch (const std::exception& e)
	{
		std::cout << e.what();
	}			
}

TX_Stream::TX_Stream(std::string sradio, int chan, DataBuffer<IQSample16> *source_buffer)
{
	radio = sradio;
	channel = chan;
	m_source_buffer = source_buffer;
}

bool TX_Stream::create_tx_streaming_thread(std::string sradio, int chan, DataBuffer<IQSample16> *source_buffer,  double ifrate)
{	
	if (ptr_tx_stream != nullptr)
		return false;
	ptr_tx_stream = make_shared<TX_Stream>(sradio, chan, source_buffer);
	ptr_tx_stream->set_if_rate(ifrate);
	tx_thread = std::thread(&TX_Stream::operator(), ptr_tx_stream);
	return true;
}

void TX_Stream::destroy_tx_streaming_thread()
{
	auto startTime = std::chrono::high_resolution_clock::now();

	if (ptr_tx_stream == nullptr)
		return;
	ptr_tx_stream->stop_flag = true;
	tx_thread.join();
	ptr_tx_stream.reset();

	auto now = std::chrono::high_resolution_clock::now();
	const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
	cout << "Stoptime TX_Stream:" << timePassed.count() << endl;
}