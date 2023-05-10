#pragma once
#include <atomic>
#include <thread>
#include "DataBuffer.h"
#include "SdrberryTypeDefs.h"

class FT8Processor
{
  public:
	FT8Processor();
	~FT8Processor();
	static void destroy_modulator(std::shared_ptr<FT8Processor> &ft8processor);
	static bool create_modulator(std::shared_ptr<FT8Processor> &ft8processor);
	void AddaudioSample(SampleVector &audiosamples, time_t cycletime);
	

	std::atomic<bool> stop_flag{false};
	std::thread ft8processor_thread;

	void operator()();
	void Stop();

  private:
	DataBuffer<Sample> samplebuffer;
	float ft8_rate{12000.0};
};

