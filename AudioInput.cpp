#include "AudioInput.h"

int record(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData)
{
	AudioInput					*audioinput = (AudioInput *)userData ;
	DataBuffer<Sample>			*databuffer = audioinput->get_databuffer();
	
	if (status)
		std::cout << "Stream overflow detected!" << std::endl;
	if (audioinput->get_tone())
	{
		audioinput->ToneBuffer();
		return 0;
	}

	// Do something with the data in the "inputBuffer" buffer.
	//printf("frames %u \n", nBufferFrames);
	SampleVector	buf;
	Sample			l = 0.0;
	for (int i = 0; i < nBufferFrames; i++)
	{
		Sample f = ((double *)inputBuffer)[i];
		buf.push_back(f);
		if (audioinput->get_stereo())
			buf.push_back(f);
		l += f*f;
	}
	audioinput->set_level(l);
	databuffer->clear();
	databuffer->push(move(buf));
	return 0;
}

void AudioInput::listDevices(std::vector<std::string> &devices)
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
		if (dev.outputChannels > 0 || dev.inputChannels > 0)
			devices.push_back(dev.name);
	}
}

int AudioInput::getDevices(std::string device)
{
	int noDevices = this->getDeviceCount();
	struct DeviceInfo	dev;
		
	if (noDevices < 1) {
		std::cout << "\nNo audio devices found!\n";
		return -1;
	}
	for (int i = 0; i < noDevices; i++)
	{
		dev = getDeviceInfo(i);
		if (dev.name.find(device) != std::string::npos)
		{
			return i;
		}
	}
	return 0; // return default device
}

AudioInput::AudioInput(int pcmrate, bool stereo, DataBuffer<Sample> *AudioBuffer)
	: parameters{}, m_volume{0.5}, asteps{}, m_level{}, tune_tone{0}
{
	m_stereo = stereo;
	databuffer = AudioBuffer; 
	parameters.nChannels = 1;
	parameters.firstChannel = 0;
	sampleRate = pcmrate;
	bufferFrames = 512;
}

bool AudioInput::open(std::string device)
{
	if (this->getDeviceCount() < 1)
	{
		std::cout << "\nNo audio devices found!\n";
		return false;
	}
	if (device != "default")
		parameters.deviceId = getDevices(device);
	else
		parameters.deviceId = this->getDefaultInputDevice();
	
	try {
		this->openStream(NULL, &parameters, RTAUDIO_FLOAT64, sampleRate, &bufferFrames, &record, (void *)this);
		this->startStream();
	}
	catch (RtAudioError& e) {
		e.printMessage();
		return false;
	}
	return true;	
}

void AudioInput::adjust_gain(SampleVector& samples)
{
	for (unsigned int i = 0, n = samples.size(); i < n; i++) {
		samples[i] *= m_volume;
	}
}


bool AudioInput::read(SampleVector& samples)
{
	if (databuffer == nullptr)
		return false;
	samples = databuffer->pull();
	if (samples.empty())
		return false;
	return true;
}

void AudioInput::close()
{
	if (isStreamOpen()) 
		closeStream();
}

AudioInput::~AudioInput()
{
	close();
}

#define TWOPIOVERSAMPLERATE 0.0001308996938995747;  // 2 Pi / 48000
const double cw_keyer_sidetone_frequency {750.0};
const double cw_keyer_sidetone_frequency2 {1500.0};

double AudioInput::Nexttone()
{
	double angle = (asteps*cw_keyer_sidetone_frequency)*TWOPIOVERSAMPLERATE;
	if (++asteps >= 48000) asteps = 0;
	return sin(angle);
}

void AudioInput::ToneBuffer()
{
	SampleVector	buf;
	printf("tone %d \n", tune_tone);
	for (int i = 0; i < bufferFrames; i++)
	{
		Sample f;

		if (tune_tone == 2)
		{
			f = (Sample) NextTwotone();
		}
		else
		{
			f = (Sample) Nexttone();	
		}
		buf.push_back(f);
		if (m_stereo)
			buf.push_back(f);
	}
	databuffer->push(move(buf));
}



double AudioInput::NextTwotone()
{
	double angle = (asteps*cw_keyer_sidetone_frequency)*TWOPIOVERSAMPLERATE;
	double angle2 = (asteps*cw_keyer_sidetone_frequency2)*TWOPIOVERSAMPLERATE;
	if (++asteps >= 48000) asteps = 0;
	return (sin(angle) + sin(angle)) /2.0;
}

float  AudioInput::get_rms_level()
{
	return m_level / (float)bufferFrames;
}

void AudioInput::set_level(float f)
{
	m_level = f;
}

int	 AudioInput::queued_samples()
{
	if (databuffer != nullptr)
		return databuffer->queued_samples();
	return 0;
}