#pragma once
#include "DataBuffer.h"
#include "RtAudio.h"
#include "AudioHeader.h"
#include "SdrberryTypeDefs.h"
#include "Settings.h"
#include <map>
#include <string>
#include <chrono>

class AudioOutput : public RtAudio
{
  private:
	RtAudio::StreamParameters parameters;
	RtAudio::DeviceInfo info;
	unsigned int sampleRate;
	unsigned int bufferFrames; // 256 sample frames
	atomic<double> volume;
	string error;
	atomic<int> underrun;
	map<int, std::string> device_map;
	SampleVector audioFrames;
	DataBuffer<Sample> databuffer;
	SampleVector underrunSamples;
	std::atomic<bool> copyUnderrun{false};
	std::atomic<std::chrono::high_resolution_clock::time_point> SampleTimeNow;
	std::atomic <std::chrono::microseconds> duration;
	std::chrono::high_resolution_clock::time_point SampleTime;
	int Audioout_class(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status);
	

  public:
	AudioOutput(int pcmrate, unsigned int bufferFrames_, RtAudio::Api api = UNSPECIFIED);
	static constexpr auto Audioout_ = AudioCallbackHandler<AudioOutput, &AudioOutput::Audioout_class>::staticCallbackHandler;
	bool open(std::string device);
	bool write(SampleVector &samples);
	void adjust_gain(SampleVector &samples);
	void adjust_gain(SampleVector &samples_in, SampleVector &samples_out);
	void close();
	~AudioOutput();
	double get_volume() { return volume; }
	void set_volume(int vol);
	unsigned int get_framesize() { return bufferFrames; }
	int queued_samples();
	int getDevices(std::string device);
	void inc_underrun() { underrun++; }
	void clear_underrun() { underrun = 0; }
	int get_underrun() { return underrun.load(); }
	int get_channels() { return info.outputChannels; }
	unsigned int get_samplerate() { return sampleRate; }
	unsigned int get_device() { return parameters.deviceId; }
	void writeSamples(const SampleVector &audiosamples);
	void mono_to_left_right(const SampleVector &samples_mono,
					   SampleVector &audio);

	static bool createAudioDevice(int Samplerate, unsigned int bufferFrames);
	void CopyUnderrunSamples(bool copySamples);
	long GetSampleDuration();
	std::chrono::high_resolution_clock::time_point GetSampleTime();

  protected:
	void samplesToInt16(const SampleVector &samples,
						std::vector<std::uint8_t> &bytes);
};

extern AudioOutput *audio_output;