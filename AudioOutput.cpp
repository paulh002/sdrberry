#include "sdrberry.h"

int Audioout( void *outputBuffer,void *inputBuffer,unsigned int nBufferFrames,double streamTime,RtAudioStreamStatus status,	void *userData)
{
	double *buffer = (double *) outputBuffer;
	
	if (status)
		std::cout << "Stream underflow detected!\n" << std::endl;
	// Write interleaved audio data.
	
	if(((DataBuffer<Sample> *)userData)->queued_samples() == 0)
	{
		int bytes = nBufferFrames * min(audio_output->get_channels(), 2);
		for (int i = 0; i < bytes; i++)
		{
			((double *)buffer)[i] = 0.0;
		}
		if (audio_output != nullptr)
			audio_output->inc_underrun();
		return 0;
	}
	SampleVector samples = ((DataBuffer<Sample> *)userData)->pull();
	int i = 0;
	for (auto& col : samples)
	{
		Sample v = col;
		((double *)buffer)[i++] = v;
	}
	samples.clear();
	samples.resize(0);
	return 0;
}

void AudioOutput::listDevices(std::vector<std::string> &devices)
{
	for (auto const &dev : device_map)
	{
		if (dev.second.size() > 0)
			devices.push_back(dev.second);
	}
} // && !dev.second.find("Monitor")

int AudioOutput::getDevices(std::string device)
{
	int noDevices = this->getDeviceCount();
	int retval = 0;

	if (noDevices < 1) {
		std::cout << "\nNo audio devices found!\n";
		return -1;
	}
	for (int i = 0; i < noDevices; i++)
	{
		info = getDeviceInfo(i);
		printf("%d device: %s input %d output %d\n", i, info.name.c_str(), info.inputChannels, info.outputChannels);
		device_map[i] = info.name;
		if (info.name.find(device) != std::string::npos && info.outputChannels > 0)
		{
			if (info.outputChannels < parameters.nChannels)
				parameters.nChannels = info.outputChannels;
			printf("audio device = %s Samplerate %d\n", info.name.c_str(), info.preferredSampleRate);
			if (info.preferredSampleRate)
				m_sampleRate = info.preferredSampleRate;
			retval = i;
		}
	}
	return retval; // return default device
}

AudioOutput::AudioOutput(int pcmrate, DataBuffer<Sample> *AudioBuffer, RtAudio::Api api)
	: RtAudio(api),
	parameters{}, bufferFrames{}, m_volume{}, underrun{0}
{
	m_sampleRate = pcmrate;
	databuffer = AudioBuffer;
	bufferFrames = 1024;
	parameters.nChannels = 2;
	parameters.firstChannel = 0;
	parameters.deviceId = 0;
	device_open = 0;
}

bool AudioOutput::open(std::string device)
{
	RtAudioErrorType err;
	StreamOptions option{{0}, {0}, {0}, {0}};
	option.flags = RTAUDIO_MINIMIZE_LATENCY;

	parameters.deviceId = 0;
	getDevices();
	for (auto const &it : device_map)
	{
		if (it.second.find(device) != std::string::npos)
		{
			device_open = it.first;
		}
	}
	
	info = getDeviceInfo(device_open);
	if (info.preferredSampleRate)
		m_sampleRate = info.preferredSampleRate;
	printf("audio device = %d %s samplerate %d\n", parameters.deviceId, device.c_str(), m_sampleRate);
	parameters.deviceId = device_open;
	err = this->openStream(&parameters, NULL, RTAUDIO_FLOAT64, m_sampleRate, &bufferFrames, &Audioout, (void *)databuffer, &option);
	if (err != RTAUDIO_NO_ERROR)
	{
		printf("Cannot open audio output stream\n");
		return false;
	}
	this->startStream();
	return true;	
}

bool AudioOutput::open(unsigned int device)
{
	RtAudioErrorType err;
	StreamOptions option{{0}, {0}, {0}, {0}};
	option.flags = RTAUDIO_MINIMIZE_LATENCY;

	device_open = device;
	info = getDeviceInfo(device_open);
	if (info.preferredSampleRate)
		m_sampleRate = info.preferredSampleRate;
	printf("audio device = %d %s samplerate %d\n", parameters.deviceId, info.name.c_str(), m_sampleRate);
	parameters.deviceId = device_open;
	err = this->openStream(&parameters, NULL, RTAUDIO_FLOAT64, m_sampleRate, &bufferFrames, &Audioout, (void *)databuffer, &option);
	if (err != RTAUDIO_NO_ERROR)
	{
		printf("Cannot open audio output stream\n");
		return false;
	}
	this->startStream();
	return true;
}

void AudioOutput::set_volume(int vol) 
{
	// log volume
	m_volume = exp(((double)vol * 6.908) / 100.0) / 1000;
	//fprintf(stderr,"vol %f\n", (float)m_volume);
} 

void AudioOutput::adjust_gain(SampleVector& samples)
{
	for (unsigned int i = 0, n = samples.size(); i < n; i++) {
		samples[i] *= m_volume;
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

bool AudioOutput::write(SampleVector& audiosamples)
{
	if (databuffer && isStreamOpen())
		databuffer->push(move(audiosamples));
	else
		audiosamples.clear();
	return true;
}

int	 AudioOutput::queued_samples()
{
	if (databuffer != nullptr)
		return databuffer->queued_samples();
	return 0;
}

unsigned int AudioOutput::getDevices()
{
	unsigned nDevices = 0;
	int result, subdevice, card;
	char name[64];
	snd_ctl_t *handle = 0;

	strcpy(name, "default");
	result = snd_ctl_open(&handle, "default", 0);
	if (result == 0)
	{
		nDevices++;
		snd_ctl_close(handle);
	}

	// Count cards and devices
	card = -1;
	snd_card_next(&card);
	while (card >= 0)
	{
		sprintf(name, "hw:%d", card);
		result = snd_ctl_open(&handle, name, 0);
		if (result < 0)
		{
			handle = 0;
			cout << "RtApiAlsa::getDeviceCount: control open, card = " << card << ", " << snd_strerror(result) << "." << endl;
			goto nextcard;
		}
		subdevice = -1;
		// Get the device name
		if (strncmp(name, "default", 7) != 0)
		{
			char *cardname;
			result = snd_card_get_name(card, &cardname);
			if (result >= 0)
			{
				device_map[card+1] = string(cardname);
				free(cardname);
			}
		}
		while (1)
		{
			result = snd_ctl_pcm_next_device(handle, &subdevice);
			if (result < 0)
			{
				cout << "RtApiAlsa::getDeviceCount: control next device, card = " << card << ", " << snd_strerror(result) << "." << endl;
				break;
			}
			if (subdevice < 0)
				break;
			nDevices++;
		}
	nextcard:
		if (handle)
			snd_ctl_close(handle);
		snd_card_next(&card);
	}
	return nDevices;
}