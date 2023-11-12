#pragma once
#include "DataBuffer.h"
#include "RtAudio.h"
#include "SdrberryTypeDefs.h"
#include "Settings.h"
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

enum audioTone
{
	NoTone = 0,
	SingleTone = 1,
	TwoTone = 2,
	FourTone = 4
} ;

class AudioInput : public RtAudio
{
  public:
	AudioInput(unsigned int pcmrate, unsigned int bufferFrames, bool stereo, DataBuffer<Sample> *AudioBuffer, RtAudio::Api api = UNSPECIFIED);
	static bool createAudioInputDevice(int Samplerate, unsigned int bufferFrames);

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
	void set_tone(audioTone tone) { tune_tone = tone; }
	audioTone get_tone() { return tune_tone; }
	operator bool() const { return m_error.empty(); }
	void clear() { databuffer->clear(); }
	std::vector<RtAudio::Api> listApis();
	bool open(int SampleRate,unsigned int device);
	void set_gain(int g) { gaindb = g; }
	unsigned int get_samplerate() { return sampleRate; }
	bool IsdigitalMode();
	bool IsBufferEmpty();
	void doDigitalMode();
	void StartDigitalMode(vector<float> &signal);
	void StopDigitalMode();
	int getbufferFrames() { return bufferFrames; }

  private:
	RtAudio::StreamParameters parameters;
	RtAudio::DeviceInfo info;
	unsigned int sampleRate;
	unsigned int bufferFrames;
	double m_volume;
	DataBuffer<Sample> *databuffer;
	string m_error;
	long asteps;
	bool m_stereo;
	double Nexttone();
	double NextTwotone();
	audioTone tune_tone;
	int gaindb;
	int bufferFramesSend;
	std::atomic<bool> digitalmode, bufferempty;
	std::vector<float> digitalmodesignal;
};

extern AudioInput *audio_input;
extern atomic_bool audio_input_on;