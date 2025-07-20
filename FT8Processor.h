#pragma once
#include <atomic>
#include <thread>
#include "DataBuffer.h"
#include "SdrberryTypeDefs.h"
#include "FT8UdpClient.h"

class FT8Processor
{
  public:
	FT8Processor(int wmode);
	~FT8Processor();
	static void destroy_modulator(std::shared_ptr<FT8Processor> &ft8processor);
	static bool create_modulator(std::shared_ptr<FT8Processor> &ft8processor, int wsjtxmode);
	void AddaudioSample(SampleVector &audiosamples);

	std::atomic<bool> stop_flag{false};
	std::thread ft8processor_thread;

	void operator()();
	void Stop();

  private:
	DataBuffer<Sample> samplebuffer;
	float ft8_rate{12000.0};
	int wsjtxmode;
};

