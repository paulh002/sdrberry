#include "AudioOutput.h"
/*
 * Audioout fills the audio output buffer.
 * If there are no samples available (underrun) mutted sound is send
 * Sound data is pulled from databuffer and copied to rtaudio buffer
 * A underrun counter is increased for adjusting samplerate of the radio
 **/

AudioOutput *audio_output;
DataBuffer<Sample> audiooutput_buffer;

bool AudioOutput::createAudioDevice(int SampleRate)
{
	auto RtApi = RtAudio::LINUX_ALSA;

	string s = Settings_file.find_audio("device");
	audio_output = new AudioOutput(SampleRate, &audiooutput_buffer, RtApi);
	if (audio_output)
	{
		audio_output->set_volume(50);
		audio_output->open(s);
		return true;
	}
	fprintf(stderr, "ERROR: AudioOutput\n");
	return false;
}

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
	return 0;
}

/*
* List alsa audio devices but skip the Monitor ones 
*
**/

void AudioOutput::listDevices(std::vector<std::string> &devices)
{
	for (auto const &dev : device_map)
	{
		if (dev.second.size() > 0 && dev.second.find("Monitor") == std::string::npos)
			devices.push_back(dev.second);
	}
}

/*
* Search for device number based on device name
* Issue -> very slow when user is not allowed to open device
* 
*/

int AudioOutput::getDevices(std::string device)
{
	RtAudio::DeviceInfo		dev_info;
	int noDevices = this->getDeviceCount();
	int retval = 0;

	if (noDevices < 1) {
		std::cout << "\nNo audio devices found!\n";
		return -1;
	}
	for (int i = 0; i < noDevices; i++)
	{
		dev_info = getDeviceInfo(i);
		printf("%d device: %s input %d output %d\n", i, info.name.c_str(), info.inputChannels, info.outputChannels);
		if (dev_info.name.find(device) != std::string::npos && dev_info.outputChannels > 0)
		{
			printf("audio device = %s Samplerate %d\n", info.name.c_str(), info.preferredSampleRate);
			info = dev_info;
			retval = i;
		}
	}
	return retval; // return default device
}

AudioOutput::AudioOutput(int pcmrate, DataBuffer<Sample> *AudioBuffer, RtAudio::Api api)
	: RtAudio(api),
	  parameters{}, bufferFrames{}, m_volume{}, underrun{0}, info{0}
{
	m_sampleRate = pcmrate;
	databuffer = AudioBuffer;
	bufferFrames = 1024;
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

bool AudioOutput::open(std::string device)
{
	int retry{0};
	RtAudioErrorType err;
	StreamOptions option{{0}, {0}, {0}, {0}};
	option.flags = RTAUDIO_MINIMIZE_LATENCY;

	parameters.deviceId = 0;
	parameters.firstChannel = 0;
	parameters.nChannels = 2;
	if (device == "default")
		parameters.deviceId = this->getDefaultInputDevice();
	else
		parameters.deviceId = find_device(device);
	info = this->getDeviceInfo(parameters.deviceId);
	if (info.preferredSampleRate)
		m_sampleRate = info.preferredSampleRate;
	parameters.nChannels = info.outputChannels;
	printf("audio device = %d %s samplerate %d channels %d\n", parameters.deviceId, device.c_str(), m_sampleRate, parameters.nChannels);
	err = this->openStream(&parameters, NULL, RTAUDIO_FLOAT64, m_sampleRate, &bufferFrames, &Audioout, (void *)databuffer, NULL);
	if (err != RTAUDIO_NO_ERROR)
	{
		printf("Cannot open audio output stream\n");
		return false;
	}
	this->startStream();
	return true;	
}

/*
 * Set volume of output use log scale
 **/
void AudioOutput::set_volume(int vol) 
{
	// log volume
	m_volume.store(exp(((double)vol * 6.908) / 100.0) / 1000);
	printf("vol %f\n", (float)m_volume.load());
} 

void AudioOutput::adjust_gain(SampleVector& samples)
{
	double gain = m_volume.load();
	for (unsigned int i = 0, n = samples.size(); i < n; i++) {
		samples[i] *= gain;
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

/*
 * Code is copied from RTAUDIO to return a list of devices and their ID
 * The ID is card number + 1 
 *
 **/


unsigned int AudioOutput::find_device(std::string name)
{
	int devices = getDeviceCount();
	unsigned int device = 0;
	
	RtAudio::DeviceInfo info;
	for (int i = 1; i <= devices; i++)
	{
		info = getDeviceInfo(i);
		// Print, for example, the maximum number of output channels for each device
		std::cout << "device = " << i << " device name " << info.name;
		std::cout << ": maximum output channels = " << info.outputChannels << "\n";
		if (std::string(info.name).find(name) != string::npos && info.outputChannels > 1)
			device = i;
	}
	return device;
}