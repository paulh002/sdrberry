#pragma once
#include "RtAudio.h"
#include <string>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include "Audiodefs.h"
#include "DataBuffer.h"

class AudioInput : public RtAudio
{
public:
  AudioInput(unsigned int pcmrate, bool stereo, DataBuffer<Sample> *AudioBuffer, RtAudio::Api api = UNSPECIFIED);
  bool open(std::string device);
  void adjust_gain(SampleVector &samples);
  bool read(SampleVector &samples);
  void close();
  ~AudioInput();
  double get_volume() { return m_volume; }
  void set_volume(int vol);
  void ToneBuffer();
  DataBuffer<Sample> *get_databuffer() { return databuffer; };
  bool get_stereo() { return m_stereo; };
  int queued_samples();
  int getDevices(std::string device);
  void listDevices(std::vector<std::string> &devices);
  void set_tone(int tone) { tune_tone = tone; }
  int get_tone() { return tune_tone; }
  operator bool() const { return m_error.empty();}
  void clear() { databuffer->clear();}
  std::vector<RtAudio::Api> listApis();
  bool open(unsigned int device);

private:
	RtAudio::StreamParameters	parameters;
	RtAudio::DeviceInfo			info;
	unsigned int				sampleRate;
	unsigned int				bufferFrames;
	double						m_volume;
	DataBuffer<Sample>			*databuffer;
	string						m_error;
	long						asteps;
	bool						m_stereo;
	double						Nexttone();
	double						NextTwotone();
	int							tune_tone;
};

extern  AudioInput  *audio_input;
extern atomic_bool	audio_input_on;