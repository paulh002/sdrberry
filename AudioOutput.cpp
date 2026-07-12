#include "AudioOutput.h"
/*
 * Audioout fills the audio output buffer.
 * If there are no samples available (underrun) mutted sound is send
 * Sound data is pulled from databuffer and copied to rtaudio buffer
 * A underrun counter is increased for adjusting samplerate of the radio
 **/

AudioOutput *audio_output;


void AudioOutput::CopyUnderrunSamples(bool copyUnderrun_)
{
	int i = 0;
	for (auto &col : underrunSamples)
		{
			underrunSamples.at(i++) = 0.0;
		}
	copyUnderrun = copyUnderrun_;
}

bool AudioOutput::createAudioDevice(int SampleRate, unsigned int bufferFrames, int deviceID)
{
	RtAudio::Api selectedApi = RtAudio::LINUX_PULSE;
	std::string dev;
	std::vector<std::string> devices;

	std::cout << "[Audio] Testing API: " << RtAudio::getApiName(RtAudio::LINUX_PULSE) << "\n";
	RtAudio tempAdc(RtAudio::LINUX_PULSE);
	unsigned int tmp_devices = tempAdc.getDeviceCount();
	if (tmp_devices == 0)
	{
		std::cout << "[Audio] API Error: No devices found. Pipewire may be inactive.\n";
		selectedApi = RtAudio::LINUX_ALSA;
	}

	audio_output = new AudioOutput(SampleRate, bufferFrames, selectedApi);
	if (audio_output)
	{
		if (!deviceID)
		{
			audio_output->listDevices(devices);
			if (devices.size() == 0)
				return 0;
			dev = devices.at(0);
			deviceID = audio_output->getAudioDevice(dev);
		}
		audio_output->set_volume(50);
		audio_output->open(deviceID);
		return true;
	}
	fprintf(stderr, "ERROR: AudioOutput\n");
	return false;
}

std::chrono::high_resolution_clock::time_point AudioOutput::GetSampleTime()
{
	return SampleTimeNow.load();
}

long AudioOutput::GetSampleDuration()
{
	return duration.load().count();
}

int AudioOutput::Audioout_class(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status)
{
	float *buffer = (float *) outputBuffer;

	if (status)
		std::cout << "Stream underflow detected!\n" << std::endl;
	
	// Measure cycle time and phase time
	SampleTimeNow.store(std::chrono::high_resolution_clock::now());
	duration.store(std::chrono::duration_cast<std::chrono::microseconds>(SampleTimeNow.load() - SampleTime));
	SampleTime = std::chrono::high_resolution_clock::now();

	// Write interleaved audio data.
	if (audio_out_buffer.size() == 0)
	{
		//Use previous samples incase of buffer underrun
		int bytes = nBufferFrames * std::min(audio_output->get_channels(), 2);
		if (underrunSamples.size() && copyUnderrun)
		{
			int i = 0;
			for (auto &col : underrunSamples)
			{
				Sample v = col;
				((float *)buffer)[i++] = v;
			}
		}
		else
		{
			for (int i = 0; i < bytes; i++)
			{
				((float *)buffer)[i] = 0.0;
			}
		}
		if (audio_output != nullptr)
			audio_output->inc_underrun();
		return 0;
	}
	
	std::span<float> samples = audio_out_buffer.get_buffer_view(audio_out_buffer.pop_block());
	underrunSamples.assign(samples.begin(), samples.end());
	//cout << "nBufferFrames " << nBufferFrames << " nSamples " << samples.size() << endl;
	int i = 0;
	for (auto& col : samples)
	{
		Sample v = col;
		((float *)buffer)[i++] = v;
	}
	return 0;
}

void AudioOutput::listDevices(std::vector<std::string> &devices)
{
	struct DeviceInfo dev;

	std::vector<unsigned int> ids = this->getDeviceIds();
	printf("List devices: \n");
	for (auto col : ids)
	{
		dev = getDeviceInfo(col);
		printf("device %s out %d in %d \n", dev.name.c_str(), dev.outputChannels, dev.inputChannels);
		if (dev.name.find("Default ALSA Device") != std::string::npos || dev.name.find("PulseAudio Sound Server") != std::string::npos)
		{
			printf("skip %d device: %s \n", col, dev.name.c_str());
			continue;
		}

		if (dev.outputChannels > 0 || dev.inputChannels > 0)
			devices.push_back(dev.name);
	}
}

int AudioOutput::getAudioDevice(std::string device)
{
	std::vector<unsigned int> ids = getDeviceIds();
	if (ids.size() == 0)
	{
		std::cout << "No devices found." << std::endl;
		return 0;
	}

	RtAudio::DeviceInfo info;
	for (auto col : ids)
	{
		info = getDeviceInfo(col);

		// Print, for example, the name and maximum number of output channels for each device
		std::cout << "device name = " << info.name << std::endl;
		std::cout << ": maximum output channels = " << info.outputChannels << std::endl;
		if (std::string(info.name).find(device) != std::string::npos && info.outputChannels > 1)
			return col;
	}
	std::cout << "No matching device found." << std::endl;
	return 0;
}

AudioOutput::AudioOutput(int pcmrate, unsigned int bufferFrames_, RtAudio::Api api)
	: RtAudio(api),
	  parameters{}, bufferFrames{bufferFrames_}, volume{}, underrun{0}, info{0}
{
	sampleRate = pcmrate;
	parameters.nChannels = 2;
	parameters.firstChannel = 0;
	parameters.deviceId = 0;
	audio_out_buffer.reserve(bufferFrames * 2); // stereo
	audioFrames.resize(bufferFrames * 2); // stereo
}

/*
 * Open sound device based on name
 * if name is default open default device
 * GetDevics() fills the map with device names and ID's
 * Use samplerate which is optimized for device 
 **/

bool AudioOutput::open(int deviceId)
{
	int retry{0};
	RtAudioErrorType err;
	StreamOptions option{{0}, {0}, {0}, {0}};
	//option.flags = RTAUDIO_MINIMIZE_LATENCY;
	option.streamName = "Sdrberry";

	parameters.deviceId = deviceId;
	parameters.firstChannel = 0;
	parameters.nChannels = 2;
	info = getDeviceInfo(parameters.deviceId);
	printf("audio output device = %d %s samplerate %d channels %d\n", parameters.deviceId, info.name.c_str(), sampleRate, parameters.nChannels);
	err = openStream(&parameters, NULL, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, (RtAudioCallback)Audioout_, (void *)this, &option);
	if (err != RTAUDIO_NO_ERROR)
	{
		printf("Cannot open audio output stream\n");
		return false;
	}
	startStream();
	SampleTime = std::chrono::high_resolution_clock::now();
	return true;	
}

/*
 * Set volume of output use log scale
 **/
void AudioOutput::set_volume(int vol) 
{
	// log volume
	volume.store(expf(((float)vol * 6.908) / 80.0) / 1000);
	//printf("vol %f\n", (float)m_volume.load());
} 

void AudioOutput::adjust_gain(std::span<float> samples)
{
	float gain = volume.load();
	for (auto &con : samples) {
		con *= gain;
	}
}

void AudioOutput::adjust_gain(std::span<float> samples, std::span<float> samples_out)
{
	float gain = volume.load();
	int i = 0;
	for (auto con : samples) {
		samples_out[i++] = con * gain;
	}
}

void AudioOutput::close()
{
	if (isStreamOpen()) 
	{
		stopStream();
		abortStream();
		closeStream();
	}
}

AudioOutput::~AudioOutput()
{
	close();
}

/*
 * Write data to audio buffer
 **/

bool AudioOutput::write(std::span<float> audiosamples)
{
	if (isStreamOpen())
		{
			audio_out_buffer.push_block(bufferFrames *2, [&](std::span<float> buffer) {
				int i = 0;
				for (auto con : audiosamples)
				{
					buffer[i++] = con;					
				}
			});
		}
	return true;
}

int	 AudioOutput::queued_samples()
{
	return audio_out_buffer.size()  * bufferFrames;
}

void AudioOutput::writeSamples(std::span<float> audioSamples)
{
	for (auto &col : audioSamples)
	{
		// split the stream in blocks of samples of the size framesize
		audioFrames.insert(audioFrames.end(), col);
		if (audioFrames.size() == get_framesize())
		{
			if ((queued_samples() / 2) < 2048)
			{
				SampleVector audioStereoSamples;

				mono_to_left_right(audioFrames, audioStereoSamples);
				write(audioStereoSamples);
				audioFrames.clear();
			}
			else
			{
				audioFrames.clear();
			}
		}
	}
}

// copy mono signal to both sereo channels
void AudioOutput::mono_to_left_right(std::span<float> samples_mono,
	std::span<float> audio)
{
	unsigned int n = samples_mono.size();

	if (audio_output->get_channels() < 2)
	{
		audio = samples_mono;
		return;
	}
	//audio.resize(2 * n);
	for (unsigned int i = 0; i < n; i++)
	{
		Sample m = samples_mono[i];
		audio[2 * i] = m;
		audio[2 * i + 1] = m;
	}
}