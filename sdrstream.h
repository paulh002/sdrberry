#pragma once
#include "AudioInput.h"
#include "AudioOutput.h"
#include "DataBuffer.h"
#include "vfo.h"
#include <complex>
#include <liquid/liquid.h>
#include <semaphore.h>
#include <thread>

extern double get_rxsamplerate();
extern double get_txsamplerate();
extern int gettxNoSamples();

class RX_Stream
{
  public:
	RX_Stream(double ifrate_, std::string sradio, int chan, std::string antenna_, long bandwidth_, DataBuffer<IQSample> *source_buffer, unsigned int decimator_factor);
	static bool create_rx_streaming_thread(double ifrate_, std::string sradio, int chan, DataBuffer<IQSample> *source_buffer, unsigned int decimator_factor);
	static void destroy_rx_streaming_thread();
	void operator()();
	atomic_bool stop_flag{false};

  private:
	double ifrate;
	std::string radio;
	int channel;
	DataBuffer<IQSample> *receiveIQBuffer;
	msresamp2_crcf decimator{nullptr};
	unsigned int decimatorFactor;
	std::string antenna;
	long bandwidth;
};

extern atomic_bool pause_flag;

extern std::thread rx_thread;
extern shared_ptr<RX_Stream> ptr_rx_stream;

class TX_Stream
{
  public:
	TX_Stream(double ifrate_, std::string sradio, int chan, DataBuffer<IQSample> *source_buffer, unsigned int decimator_factor);
	static bool create_tx_streaming_thread(double ifrate_, std::string sradio, int chan, DataBuffer<IQSample> *source_buffer, double ifrate, unsigned int decimator_factor, bool restart);
	static void destroy_tx_streaming_thread(bool close_stream = false);
	void close_tx_stream();
	void operator()();
	atomic<bool> stop_flag{false};

  private:
	double ifrate;
	std::string radio;
	int channel;
	DataBuffer<IQSample> *receiveIQBuffer;
	msresamp2_crcf decimator{nullptr};
	unsigned int decimatorFactor;
};

extern std::thread tx_thread;
extern shared_ptr<TX_Stream> ptr_tx_stream;
