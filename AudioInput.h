#pragma once
#include "RtAudio.h"
#include <string>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include "Audiodefs.h"
#include "DataBuffer.h"

extern double get_audio_input_rate();
	
class AudioInput : public RtAudio
{
public:
  AudioInput(int pcmrate, bool stereo, DataBuffer<Sample> *AudioBuffer);
  bool open(std::string device);
  void adjust_gain(SampleVector &samples);
  bool read(SampleVector &samples);
  void close();
  ~AudioInput();
  double get_volume() { return m_volume; }
  void set_volume(double vol) { m_volume = vol; }
  void ToneBuffer(int tone);
  DataBuffer<Sample> *get_databuffer() { return databuffer; };
  bool get_stereo() { return m_stereo; };
  float get_rms_level();
  void set_level(float f);
  int queued_samples();
  int getDevices(std::string device);
  void listDevices(std::vector<std::string> &devices);
  
  
  operator bool() const { return m_error.empty();}
	
private:
	RtAudio::StreamParameters	parameters;
	unsigned int				sampleRate;
	unsigned int				bufferFrames;
	double						m_volume;
	DataBuffer<Sample>			*databuffer;
	string						m_error;
	long						asteps;
	bool						m_stereo;
	double						Nexttone();
	double						NextTwotone();
	float						m_level;
};

extern  AudioInput  *audio_input;
extern atomic_bool	audio_input_on;