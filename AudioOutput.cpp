#include "AudioOutput.h"
/*
 * Audioout fills the audio output buffer.
 * If there are no samples available (underrun) mutted sound is send
 * Sound data is pulled from databuffer and copied to rtaudio buffer
 * A underrun counter is increased for adjusting samplerate of the radio
 **/

AudioOutput *audio_output;
DataBuffer<Sample> audiooutput_buffer;
SampleVector underrunSamples;

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
		//Use previous samples incase of buffer underrun
		int bytes = nBufferFrames * min(audio_output->get_channels(), 2);
		if (underrunSamples.size())
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
	SampleVector samples = ((DataBuffer<Sample> *)userData)->pull();
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

int AudioOutput::getDevices(std::string device)
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
		parameters.deviceId = getDefaultOutputDevice(); //getDefaultOutputDevice(); 
	else
		parameters.deviceId = getDevices(device);
	info = getDeviceInfo(parameters.deviceId);
	if (info.preferredSampleRate)
		m_sampleRate = info.preferredSampleRate;
	parameters.nChannels = info.outputChannels;
	printf("audio device = %d %s samplerate %d channels %d\n", parameters.deviceId, device.c_str(), m_sampleRate, parameters.nChannels);
	err = openStream(&parameters, NULL, RTAUDIO_FLOAT64, m_sampleRate, &bufferFrames, &Audioout, (void *)databuffer, NULL);
	if (err != RTAUDIO_NO_ERROR)
	{
		printf("Cannot open audio output stream\n");
		return false;
	}
	startStream();
	return true;	
}

/*
 * Set volume of output use log scale
 **/
void AudioOutput::set_volume(int vol) 
{
	// log volume
	m_volume.store(exp(((double)vol * 6.908) / 100.0) / 1000);
	//printf("vol %f\n", (float)m_volume.load());
} 

void AudioOutput::adjust_gain(SampleVector& samples)
{
	double gain = m_volume.load();
	for (unsigned int i = 0, n = samples.size(); i < n; i++) {
		samples[i] *= gain;
	}
}

void AudioOutput::adjust_gain(SampleVector &samples_in, SampleVector &samples_out)
{
	double gain = m_volume.load();
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