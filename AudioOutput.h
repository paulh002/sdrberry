#pragma once
#include "RtAudio.h"
#include "Audiodefs.h"
#include "DataBuffer.h"
#include <map>
#include <string>

class AudioOutput :
    public RtAudio
{
public:
  AudioOutput(int pcmrate, DataBuffer<Sample> *AudioBuffer, RtAudio::Api api = UNSPECIFIED);
  bool open(std::string device);
  bool write(SampleVector &samples);
  void adjust_gain(SampleVector &samples);
  void close();
  ~AudioOutput();
  double get_volume() { return m_volume; }
  void set_volume(int vol);
  unsigned int get_framesize() { return bufferFrames; }
  int queued_samples();
  void listDevices(std::vector<std::string> &devices);
  int getDevices(std::string device);
  void inc_underrun() { underrun++; }
  void clear_underrun() { underrun = 0; }
  int  get_underrun() { return underrun.load(); }
  int get_channels() { return info.outputChannels; }
  unsigned int get_samplerate() { return m_sampleRate; }
  unsigned int getDevices();
  unsigned int get_device() { return parameters.deviceId;}

protected:
	void samplesToInt16(const SampleVector& samples,
		std::vector<std::uint8_t>& bytes);

private:
	RtAudio::StreamParameters	parameters;
	RtAudio::DeviceInfo			info;
	DataBuffer<Sample>			*databuffer;
	unsigned int				m_sampleRate;
	unsigned int				bufferFrames;  // 256 sample frames
	double						m_volume;
	string						m_error;
	atomic<int>					underrun;
	map<int, std::string> device_map;
};

extern  AudioOutput *audio_output;