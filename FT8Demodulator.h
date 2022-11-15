#pragma once
#include <cstdio>
#include <cassert>
#include <cmath>
#include <complex>
#include <liquid.h>
#include <vector>
#include "DataBuffer.h"
#include "AudioOutput.h"
#include "Demodulator.h"

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

class FT8Demodulator : public Demodulator
{
  public:
	static bool create_demodulator(double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output);
	static void destroy_demodulator();
	static std::string getName() { return "FT8Demodulator"; }

	FT8Demodulator(double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output);
	~FT8Demodulator();
	void process(const IQSampleVector &samples_in, SampleVector &audio) ;
	void operator()() override;

	atomic<bool> stop_flag{false};
	std::thread amdemod_thread;

  private:
	ampmodem m_demod{nullptr};
	float m_bandwidth;
	float ft8_rate{12000.0};
};

extern DataQueue<FT8Message> FT8Queue;