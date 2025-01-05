#include "sdrstream.h"
#include "DataBuffer.h"
#include "SdrDevice.h"
#include "gui_bar.h"
#include "gui_setup.h"
#include "gui_tx.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include "lvgl.h"
#include "sdrberry.h"
#include "vfo.h"
#include <SoapySDR/Device.h>
#include <SoapySDR/Formats.h>
#include <SoapySDR/Types.h>
#include <complex.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <math.h>
#include <mutex>
#include <stdint.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

std::atomic<double> rx_sampleRate{0.0};
std::atomic<double> tx_sampleRate{0};
std::atomic<int> rx_nosample{0};

double get_rxsamplerate()
{
	return rx_sampleRate.load();
}

double get_txsamplerate()
{
	return tx_sampleRate;
}

int gettxNoSamples()
{
	return rx_nosample;
}

std::thread rx_thread;
std::thread tx_thread;
shared_ptr<RX_Stream> ptr_rx_stream;
shared_ptr<TX_Stream> ptr_tx_stream;
std::mutex rxstream_mutex;
atomic_bool pause_flag{false};

void RX_Stream::operator()()
{
	const auto startTime = std::chrono::high_resolution_clock::now();
	auto timeLastPrint = std::chrono::high_resolution_clock::now();
	auto timeLastStatus = std::chrono::high_resolution_clock::now();
	std::chrono::time_point<std::chrono::high_resolution_clock> startReadTime, stoptReadTime;
	std::chrono::microseconds timePassed{};
	uint64_t timePassed_avg{}, samples_read{};

	unsigned long long totalSamples(0);
	std::unique_lock<mutex> lock_rx(rxstream_mutex);

	int default_block_length;
	SoapySDR::Stream *rx_stream;
	int ret;

	default_block_length = 1024;
	if ((ifrate < 192001) && (ifrate > 48000))
		default_block_length = 2048;
	if ((ifrate < 384001) && (ifrate > 192000))
		default_block_length = 8192;
	if ((ifrate > 384001) && (ifrate > 1000000))
		default_block_length = 16384; // 8192;
	if (ifrate > 1000000)
		default_block_length = 65536;
	rx_sampleRate = ifrate / 1000000.0;
	default_block_length = Settings_file.get_int(default_radio, "buffersize", default_block_length);

	printf("default block length is set to %d ifrate %f\n", default_block_length, ifrate);
	try
	{
		SdrDevices.SdrDevices.at(radio)->setSampleRate(SOAPY_SDR_RX, 0, ifrate);
		rx_stream = SdrDevices.SdrDevices.at(radio)->setupStream(SOAPY_SDR_RX, SOAPY_SDR_CF32);
		SdrDevices.SdrDevices.at(radio)->activateStream(rx_stream);
	}
	catch (const std::exception &e)
	{
		std::cout << e.what();
		stop_flag = true;
		return;
	}
	// size_t mtu = SdrDevices.SdrDevices.at(radio)->getStreamMTU(rx_stream);
	// cout << "mtu " << mtu << endl;
	// if (mtu > default_block_length)
	//	default_block_length = mtu;
	stop_flag = false;
	startReadTime = std::chrono::high_resolution_clock::now();
	while (!stop_flag.load())
	{
		unsigned int overflows(0);
		unsigned int underflows(0);
		int flags(0);
		long long time_ns(0);
		vector<complex<float>> buf(default_block_length), resampleData;
		void *buffs[1] = {buf.data()};
		buffs[0] = (void *)buf.data();
		int dfactor{0};

		if (decimator)
		{
			dfactor = floor(pow(2, decimatorFactor));
			resampleData.resize(default_block_length / dfactor);
		}
		try
		{
			stoptReadTime = std::chrono::high_resolution_clock::now();
			ret = SdrDevices.SdrDevices.at(radio)->readStream(rx_stream, buffs, default_block_length, flags, time_ns, 1e6);
			if (ret > 0)
			{
				timePassed = std::chrono::duration_cast<std::chrono::microseconds>(stoptReadTime - startReadTime);
				samples_read = ret; 
				rx_nosample = ret;
				rx_sampleRate = timePassed.count();
				startReadTime = stoptReadTime;
				if (decimator)
				{
					for (int i = 0; i < ret / dfactor; i++)
					{
						complex<float> y;

						msresamp2_crcf_execute(decimator, &buf.data()[i * dfactor], &y);
						resampleData[i] = y;
					}
					resampleData.resize(ret / dfactor);
				}
			}
		}
		catch (const std::exception &e)
		{
			std::cout << e.what();
			printf("Error readStream exception\n");
			stop_flag = true;
			continue;
		}
		if (ret == SOAPY_SDR_TIMEOUT)
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
			printf("Error readStream\n");
			stop_flag = true;
			continue;
		}
		if (ret > 0)
		{
			if (!pause_flag)
			{
				if (decimator)
				{
					resampleData.resize(ret / dfactor);
					receiveIQBuffer->push(std::move(resampleData));
				}
				else
				{
					buf.resize(ret);
					receiveIQBuffer->push(std::move(buf));
				}
			}
			else
			{
				buf.clear();
			}
		}

		totalSamples += ret;
		const auto now = std::chrono::high_resolution_clock::now();
		if (timeLastStatus + std::chrono::seconds(1) < now)
		{
			timeLastStatus = now;
			while (true)
			{
				size_t chanMask;
				int flags;
				long long timeNs;
				try
				{
					ret = SdrDevices.SdrDevices.at(radio)->readStreamStatus(rx_stream, chanMask, flags, timeNs, 0);
				}
				catch (const std::exception &e)
				{
					std::cout << e.what();
				}
				if (ret == SOAPY_SDR_OVERFLOW)
					overflows++;
				else if (ret == SOAPY_SDR_UNDERFLOW)
					underflows++;
				else if (ret == SOAPY_SDR_TIME_ERROR)
				{
				}
				else
					break;
			}
		}
		timeLastPrint = now;
	}
	receiveIQBuffer->clear();
	receiveIQBuffer->push_end();
	try
	{
		SdrDevices.SdrDevices.at(radio)->deactivateStream(rx_stream);
		SdrDevices.SdrDevices.at(radio)->closeStream(rx_stream);
	}
	catch (const std::exception &e)
	{
		std::cout << e.what();
	}
	if (decimator)
	{
		msresamp2_crcf_destroy(decimator);
		decimator = nullptr;
	}
}

RX_Stream::RX_Stream(double ifrate_, std::string sradio, int chan, DataBuffer<IQSample> *source_buffer, unsigned int decimator_factor)
{
	ifrate = ifrate_;
	radio = sradio;
	channel = chan;
	receiveIQBuffer = source_buffer;
	if (decimator_factor)
	{
		int type = LIQUID_RESAMP_DECIM;
		// unsigned int num_stages = 4; // decimate by 2^4=16
		float fc = 0.2f;  // signal cut-off frequency
		float f0 = 0.0f;  // (ignored)
		float As = 60.0f; // stop-band attenuation

		ifrate = ifrate_ * pow(2, decimator_factor);
		decimatorFactor = decimator_factor;
		decimator = msresamp2_crcf_create(type, decimator_factor, fc, f0, As);
		msresamp2_crcf_print(decimator);
	}
}

bool RX_Stream::create_rx_streaming_thread(double ifrate_, std::string radio, int chan, DataBuffer<IQSample> *source_buffer, unsigned int decimator_factor)
{
	if (ptr_rx_stream != nullptr)
		return false;
	if (SdrDevices.get_rx_channels(default_radio) < 1)
		return false;
	SdrDevices.SdrDevices.at(radio)->setSampleRate(SOAPY_SDR_RX, chan, ifrate_);
	SdrDevices.SdrDevices.at(radio)->setGain(SOAPY_SDR_RX, chan, gbar.get_rf_gain());
	ptr_rx_stream = make_shared<RX_Stream>(ifrate_, radio, chan, source_buffer, decimator_factor);
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
	auto timeLastStatus = std::chrono::high_resolution_clock::now();
	unsigned long long totalSamples(0);

	IQSampleVector iqsamples, resampleData;
	SoapySDR::Stream *tx_stream;
	int ret, dfactor, streammtu;
	
	try
	{
		// SdrDevices.SdrDevices.at(radio)->setBandwidth(SOAPY_SDR_TX, 0, m_ifrate); //0.1
		// SdrDevices.SdrDevices.at(radio)->setAntenna(SOAPY_SDR_TX, 0, string("A"));
		tx_stream = SdrDevices.SdrDevices.at(radio)->setupStream(SOAPY_SDR_TX, SOAPY_SDR_CF32);
		SdrDevices.SdrDevices.at(radio)->setSampleRate(SOAPY_SDR_TX, 0, ifrate);
		SdrDevices.SdrDevices.at(radio)->setFrequency(SOAPY_SDR_TX, 0, (double)vfo.get_tx_frequency());
		SdrDevices.SdrDevices.at(radio)->setGain(SOAPY_SDR_TX, 0, Gui_tx.get_drv_pos());
		SdrDevices.SdrDevices.at(radio)->activateStream(tx_stream);
	}
	catch (const std::exception &e)
	{
		printf("Failed create transmit stream\n");
		std::cout << e.what();
		return;
	}
	receiveIQBuffer->clear();

	streammtu = SdrDevices.SdrDevices.at(radio)->getStreamMTU(tx_stream);
	dfactor = floor(pow(2, decimatorFactor));
	while (!stop_flag.load())
	{
		unsigned int overflows(0);
		unsigned int underflows(0);
		int flags(0); // SOAPY_SDR_END_BURST
		long long time_ns(0);
		int samples_transmit = 0;

		iqsamples = receiveIQBuffer->pull();
		if (iqsamples.empty())
			continue;
		// printf("samples %d %d %d \n", iqsamples.size(), iqsamples[0].real(), iqsamples[0].imag());
		samples_transmit = iqsamples.size();
		complex<float> *buffs[5]{};
		buffs[0] = (complex<float> *)iqsamples.data();
		if (decimator)
		{
			const int shunckSize = streammtu; //131072; // 65536;
			const int shunckdiv = shunckSize / dfactor;
			const int shuncks = samples_transmit / shunckdiv;
			int size = dfactor * samples_transmit;
			resampleData.resize(size);
			buffs[0] = (complex<float> *)resampleData.data();
			for (int ii = 0; ii < shuncks; ii++)
			{
				for (int i = 0; i < shunckdiv; i++)
				{
					msresamp2_crcf_execute(decimator, &iqsamples.data()[i + shunckdiv * ii], &resampleData.data()[i * dfactor]);
				}
				ret = SdrDevices.SdrDevices.at(radio)->writeStream(tx_stream, (const void *const *)buffs, shunckSize, flags, time_ns, 1e5);
			}
			int reststart = shuncks * shunckdiv;
			int amount = samples_transmit * dfactor - reststart * dfactor;
			for (int i = 0; i < samples_transmit - reststart; i++)
			{
				msresamp2_crcf_execute(decimator, &iqsamples.data()[i + reststart], &resampleData.data()[i * dfactor]);
			}
			ret = SdrDevices.SdrDevices.at(radio)->writeStream(tx_stream, (const void *const *)buffs, amount, flags, time_ns, 1e5);
		}
		else
		{
			ret = SdrDevices.SdrDevices.at(radio)->writeStream(tx_stream, (const void *const *)buffs, samples_transmit, flags, time_ns, 1e5);
		}
		if (ret == SOAPY_SDR_TIMEOUT)
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
			catch (const std::exception &e)
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
	try
	{
		SdrDevices.SdrDevices.at(radio)->deactivateStream(tx_stream);
		SdrDevices.SdrDevices.at(radio)->closeStream(tx_stream);
	}
	catch (const std::exception &e)
	{
		std::cout << e.what();
	}
	if (decimator)
	{
		msresamp2_crcf_destroy(decimator);
		decimator = nullptr;
	}
	printf("Exit writeStream\n");
}

TX_Stream::TX_Stream(double ifrate_, std::string sradio, int chan, DataBuffer<IQSample> *source_buffer, unsigned int decimator_factor)
{
	radio = sradio;
	channel = chan;
	receiveIQBuffer = source_buffer;
	ifrate = ifrate_;
	if (decimator_factor)
	{
		int type = LIQUID_RESAMP_INTERP;
		// unsigned int num_stages = 4; // decimate by 2^4=16
		float fc = 0.45f;  // signal cut-off frequency
		float f0 = 0.0f;  // (ignored)
		float As = 60.0f; // stop-band attenuation

		ifrate = ifrate_ * pow(2, decimator_factor);
		decimatorFactor = decimator_factor;
		decimator = msresamp2_crcf_create(type, decimator_factor, fc, f0, As);
		msresamp2_crcf_print(decimator);
		printf("TX STREAM Interpoloation %d\n", (int)pow(2, decimator_factor));
	}
}

bool TX_Stream::create_tx_streaming_thread(double ifrate_, std::string radio, int chan, DataBuffer<IQSample> *source_buffer, double ifrate, unsigned int decimator_factor)
{
	if (ptr_tx_stream != nullptr)
		return false;
	ptr_tx_stream = make_shared<TX_Stream>(ifrate_, radio, chan, source_buffer, decimator_factor);
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