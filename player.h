#pragma once
#include "Demodulator.h"
#include "WavReader.h"

class player : public Demodulator
{
  private:
	WavReader wavReader;

  public:
	static bool create_player(AudioOutput *audio_ouput, AudioInput *audio_input, const std::string &_filename);
	static void destroy_player();

	player(AudioOutput *audio_ouput, AudioInput *audio_input);
	~player();

	void operator()();

	std::atomic<bool> stop_flag{false};
	std::thread player_thread;
	std::string filename;
};