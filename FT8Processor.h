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
	std::queue<time_t> cycletimes;
	float ft8_rate{12000.0};
};

class FT8Message
{
  public:
	FT8Message(int chh, int cmm, int css, int csnr, int cc, double coff, double chz0, std::string cmsg)
		: hh{chh}, min{cmm}, sec{css}, snr{csnr}, correct_bits{cc},
		  off{coff}, hz0{chz0}, msg{cmsg} {};

	FT8Message()
		: hh{0}, min{0}, sec{0}, snr{0}, correct_bits{0},
		  off{0.0}, hz0{0.0}, msg{""} {};

	void display();

  private:
	int hh;
	int min;
	int sec;
	int snr;
	int correct_bits;
	double off;
	double hz0;
	std::string msg;
};

extern DataQueue<FT8Message> FT8Queue;