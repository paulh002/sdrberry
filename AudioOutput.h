#pragma once
#include "DataBuffer.h"
#include "RtAudio.h"
#include "AudioHeader.h"
#include "SdrberryTypeDefs.h"
#include <map>
#include <string>
#include <chrono>
#include <atomic>
#include <vector>
#include "CircularQueue.h"

class AudioOutput : public RtAudio
{
  private:
	RtAudio::StreamParameters parameters;
	RtAudio::DeviceInfo info;
	unsigned int sampleRate;
	unsigned int bufferFrames; // 256 sample frames
	std::atomic<double> volume;
	std::string error;
	std::atomic<int> underrun;
	std::map<int, std::string> device_map;
	SampleVector audioFrames;
	CircularQueue<float, 10> audio_out_buffer;
	SampleVector underrunSamples;
	std::atomic<bool> copyUnderrun{false};
	std::atomic<std::chrono::high_resolution_clock::time_point> SampleTimeNow;
	std::atomic <std::chrono::microseconds> duration;
	std::chrono::high_resolution_clock::time_point SampleTime;
	int Audioout_class(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status);
	
  public:
	AudioOutput(int pcmrate, unsigned int bufferFrames_, RtAudio::Api api = UNSPECIFIED);
	static constexpr auto Audioout_ = AudioCallbackHandler<AudioOutput, &AudioOutput::Audioout_class>::staticCallbackHandler;
	void listDevices(std::vector<std::string> &devices);
	bool open(int deviceId);
	bool write(std::span<float>);
	void adjust_gain(std::span<float> samples);
	void adjust_gain(std::span<float> samples, std::span<float> samples_out);
	void close();
	~AudioOutput();
	float get_volume() { return volume; }
	void set_volume(int vol);
	unsigned int get_framesize() { return bufferFrames; }
	int queued_samples();
	int getAudioDevice(std::string device);
	void inc_underrun() { underrun++; }
	void clear_underrun() { underrun = 0; }
	int get_underrun() { return underrun.load(); }
	int get_channels() { return info.outputChannels; }
	unsigned int get_samplerate() { return sampleRate; }
	unsigned int get_device() { return parameters.deviceId; }
	void writeSamples(std::span<float> audiosamples);
	void mono_to_left_right(std::span<float> samples_mono,
		std::span<float> audio);

	static bool createAudioDevice(int Samplerate, unsigned int bufferFrames, int deviceID);
	void CopyUnderrunSamples(bool copySamples);
	long GetSampleDuration();
	std::chrono::high_resolution_clock::time_point GetSampleTime();

  protected:
	void samplesToInt16(const SampleVector &samples,
						std::vector<std::uint8_t> &bytes);
};

extern AudioOutput *audio_output;