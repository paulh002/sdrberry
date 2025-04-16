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
	auto RtApi = RtAudio::LINUX_ALSA;
	std::string dev;
	std::vector<std::string> devices;

	audio_output = new AudioOutput(SampleRate,bufferFrames, RtApi);
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
	double *buffer = (double *) outputBuffer;

	if (status)
		std::cout << "Stream underflow detected!\n" << std::endl;
	
	// Measure cycle time and phase time
	SampleTimeNow.store(std::chrono::high_resolution_clock::now());
	duration.store(std::chrono::duration_cast<std::chrono::microseconds>(SampleTimeNow.load() - SampleTime));
	SampleTime = std::chrono::high_resolution_clock::now();

	// Write interleaved audio data.
	if (databuffer.queued_samples() == 0)
	{
		//Use previous samples incase of buffer underrun
		int bytes = nBufferFrames * min(audio_output->get_channels(), 2);
		if (underrunSamples.size() && copyUnderrun)
		{
			int i = 0;
			for (auto &col : underrunSamples)
			{
				Sample v = col;
				((double *)buffer)[i++] = v;
			}
		}
		else
		{
			for (int i = 0; i < bytes; i++)
			{
				((double *)buffer)[i] = 0.0;
			}
		}
		if (audio_output != nullptr)
			audio_output->inc_underrun();
		return 0;
	}
	SampleVector samples = databuffer.pull();
	underrunSamples = samples;
	//cout << "nBufferFrames " << nBufferFrames << " nSamples " << samples.size() << endl;
	int i = 0;
	for (auto& col : samples)
	{
		Sample v = col;
		((double *)buffer)[i++] = v;
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
		if (std::string(info.name).find(device) != string::npos && info.outputChannels > 1)
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
	option.flags = RTAUDIO_MINIMIZE_LATENCY;

	parameters.deviceId = deviceId;
	parameters.firstChannel = 0;
	parameters.nChannels = 2;
	info = getDeviceInfo(parameters.deviceId);
	if (info.preferredSampleRate)
		sampleRate = info.preferredSampleRate;
	//parameters.nChannels = info.outputChannels;
	printf("audio output device = %d %s samplerate %d channels %d\n", parameters.deviceId, info.name.c_str(), sampleRate, parameters.nChannels);
	err = openStream(&parameters, NULL, RTAUDIO_FLOAT64, sampleRate, &bufferFrames, (RtAudioCallback)Audioout_, (void *)this, NULL);
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
	volume.store(exp(((double)vol * 6.908) / 80.0) / 1000);
	//printf("vol %f\n", (float)m_volume.load());
} 

void AudioOutput::adjust_gain(SampleVector& samples)
{
	double gain = volume.load();
	for (unsigned int i = 0, n = samples.size(); i < n; i++) {
		samples[i] *= gain;
	}
}

void AudioOutput::adjust_gain(SampleVector &samples_in, SampleVector &samples_out)
{
	double gain = volume.load();
	for (auto sample  : samples_in)
	{
		samples_out.push_back(gain * sample);
	}
}

void AudioOutput::close()
{
	if (isStreamOpen()) 
	{
		stopStream();
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

bool AudioOutput::write(SampleVector& audiosamples)
{
	if (isStreamOpen())
		databuffer.push(std::move(audiosamples));
	else
		audiosamples.clear();
	return true;
}

int	 AudioOutput::queued_samples()
{
	return databuffer.queued_samples();
}

void AudioOutput::writeSamples(const SampleVector &audioSamples)
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
void AudioOutput::mono_to_left_right(const SampleVector &samples_mono,
									 SampleVector &audio)
{
	unsigned int n = samples_mono.size();

	if (audio_output->get_channels() < 2)
	{
		audio = samples_mono;
		return;
	}
	audio.resize(2 * n);
	for (unsigned int i = 0; i < n; i++)
	{
		Sample m = samples_mono[i];
		audio[2 * i] = m;
		audio[2 * i + 1] = m;
	}
}