#pragma once
#include "DataBuffer.h"
#include "RtAudio.h"
#include "AudioHeader.h"
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
  private:
	RtAudio::StreamParameters parameters;
	RtAudio::DeviceInfo info;
	unsigned int sampleRate;
	unsigned int bufferFrames;
	double volume, digitalvolume;
	string error;
	long asteps;
	bool stereo;
	double Nexttone();
	double NextTwotone();
	audioTone tune_tone;
	int gaindb;
	int bufferFramesSend;
	std::atomic<bool> digitalmode, bufferempty;
	std::vector<float> digitalmodesignal;
	DataBuffer<Sample> databuffer;
	int AudioIn_class(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status);

  public:
	AudioInput(unsigned int pcmrate, unsigned int bufferFrames, bool stereo, RtAudio::Api api = UNSPECIFIED);
	static int createAudioInputDevice(int Samplerate, unsigned int bufferFrames);
	static constexpr auto AudioIn = AudioCallbackHandler<AudioInput, &AudioInput::AudioIn_class>::staticCallbackHandler;

	bool open(int deviceId);
	void adjust_gain(SampleVector &samples);
	bool read(SampleVector &samples);
	void close();
	~AudioInput();
	double get_volume() { return volume; }
	void set_volume(int vol);
	void set_digital_volume(int vol);
	void ToneBuffer();
	bool get_stereo() { return stereo; };
	int queued_samples();
	int getAudioDevice(std::string device);
	void listDevices(std::vector<std::string> &devices);
	void set_tone(audioTone tone) { tune_tone = tone; }
	audioTone get_tone() { return tune_tone; }
	operator bool() const { return error.empty(); }
	void clear() { databuffer.clear(); }
	std::vector<RtAudio::Api> listApis();
	void set_gain(int g) { gaindb = g; }
	unsigned int get_samplerate() { return sampleRate; }
	bool IsdigitalMode();
	bool IsBufferEmpty();
	void doDigitalMode();
	void StartDigitalMode(vector<float> &signal);
	void StopDigitalMode();
	int getbufferFrames() { return bufferFrames; }
};

extern AudioInput *audio_input;