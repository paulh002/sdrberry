#pragma once
#include <atomic>
#include <thread>
#include "DataBuffer.h"
#include "SdrberryTypeDefs.h"
#include "FT8UdpClient.h"
#include <wsjtx_lib.h>

class WSPRProcessor
{
  public:
	WSPRProcessor(int wmode);
	~WSPRProcessor();
	static void destroy_modulator(std::shared_ptr<WSPRProcessor> &WSPRprocessor);
	static bool create_modulator(std::shared_ptr<WSPRProcessor> &WSPRprocessor, int wsjtxmode);
	void AddAudioSample(SampleVector samples);
	std::atomic<bool> stop_flag{false};
	std::thread WSPRprocessor_thread;

	void operator()();
	void Stop();

  private:
	DataBuffer<Sample> samplebuffer;
	float WSPR_rate{12000.0};
	int wsjtxmode;
};

extern DataQueue<std::vector<decoder_results>> WsprMessageQueue;