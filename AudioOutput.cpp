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
	int noDevices = this->getDeviceCount();
	struct DeviceInfo	dev;
		
	if (noDevices < 1) {
		std::cout << "\nNo audio devices found!\n";
		return ;
	}
	for (int i = 0; i < noDevices; i++)
	{
		dev = getDeviceInfo(i);
		if ((dev.outputChannels > 0 || dev.inputChannels > 0) && !dev.name.find("Monitor"))
			devices.push_back(dev.name);
	}
}

int AudioOutput::getDevices(std::string device)
{
	int noDevices = this->getDeviceCount();
		
	if (noDevices < 1) {
		std::cout << "\nNo audio devices found!\n";
		return -1;
	}
	for (int i = 0; i < noDevices; i++)
	{
		info = getDeviceInfo(i);
		printf("%d device: %s input %d output %d\n", i, info.name.c_str(), info.inputChannels, info.outputChannels);
		if (info.name.find(device) != std::string::npos && info.outputChannels > 0)
		{
			if (info.outputChannels < parameters.nChannels)
				parameters.nChannels = info.outputChannels;
			printf("audio device = %s Samplerate %d\n", info.name.c_str(), info.preferredSampleRate);
			if (info.preferredSampleRate)
				m_sampleRate = info.preferredSampleRate;
			return i;
		}
	}
	return 0; // return default device
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
}

bool AudioOutput::open(std::string device)
{
	RtAudioErrorType err;
	StreamOptions option{{0}, {0}, {0}, {0}};

	option.flags = RTAUDIO_MINIMIZE_LATENCY;
	if (this->getDeviceCount() < 1)
	{
		std::cout << "\nNo audio devices found!\n";
		return false;
	}
	if (device != "default")
	{
		parameters.deviceId = getDevices(device);
	}
	else
	{
		parameters.deviceId = this->getDefaultOutputDevice();
		info = getDeviceInfo(parameters.deviceId);
		printf("%d device: %s input %d output %d\n", parameters.deviceId, info.name.c_str(), info.inputChannels, info.outputChannels);
	}
	printf("Default audio device = %d\n", parameters.deviceId);

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