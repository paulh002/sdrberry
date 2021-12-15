#pragma once

#include <complex>
#include <pthread.h>
#include <semaphore.h>
#include "devices.h"
#include "vfo.h"

#include "DataBuffer.h"
#include "sdrberry.h"

extern double get_rxsamplerate();

class RX_Stream
{
public:	
	RX_Stream(struct device_structure *dev);
	static  bool	create_rx_streaming_thread(struct device_structure *dev);
	static	void	destroy_rx_streaming_thread();
	void	operator()();

private:
	struct device_structure *sdr_dev;	
};

extern std::thread					rx_thread;
extern shared_ptr<RX_Stream>		ptr_rx_stream;

class TX_Stream
{
public:
	TX_Stream(struct device_structure *dev);
	static  bool	create_tx_streaming_thread(struct device_structure *dev, double ifrate);
	static	void	destroy_tx_streaming_thread();
	void	operator()();
	void	set_if_rate(double ifrate) {m_ifrate = ifrate;} 
	atomic<bool>		stop_flag {false};
	
private:
	struct device_structure *sdr_dev;	
	double m_ifrate;
};

extern std::thread					tx_thread;
extern shared_ptr<TX_Stream>		ptr_tx_stream;
