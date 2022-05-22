#pragma once

#include <complex>
#include <semaphore.h>
#include "vfo.h"
#include "DataBuffer.h"
#include "sdrberry.h"

extern double get_rxsamplerate();
extern double get_txsamplerate();

class RX_Stream
{
public:	
	RX_Stream(std::string sradio, int chan, DataBuffer<IQSample> *source_buffer);
	static  bool	create_rx_streaming_thread(std::string sradio, int chan, DataBuffer<IQSample> *source_buffer);
	static	void	destroy_rx_streaming_thread();
	void	operator()();
	atomic_bool		stop_flag {false};

  private:
	std::string				radio;
	int						channel;
	DataBuffer<IQSample>	*m_source_buffer;
};

extern std::thread					rx_thread;
extern shared_ptr<RX_Stream>		ptr_rx_stream;

class TX_Stream
{
public:
	TX_Stream(std::string sradio, int chan, DataBuffer<IQSample16> *source_buffer);
	static  bool	create_tx_streaming_thread(std::string sradio, int chan, DataBuffer<IQSample16> *source_buffer, double ifrate);
	static	void	destroy_tx_streaming_thread();
	void	operator()();
	atomic<bool>		stop_flag {false};
	
private:
	std::string				radio;
	int						channel;
	DataBuffer<IQSample16> *m_source_buffer;
};

extern std::thread					tx_thread;
extern shared_ptr<TX_Stream>		ptr_tx_stream;
