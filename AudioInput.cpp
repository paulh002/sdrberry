#include "AudioInput.h"
bool g_stereo = false;

int record(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData)
{
	if (status)
		std::cout << "Stream overflow detected!" << std::endl;
	// Do something with the data in the "inputBuffer" buffer.
	
	//printf("frames %u \n", nBufferFrames);
	// add to databuffer
	SampleVector	buf;
	for (int i = 0; i < nBufferFrames; i++)
	{
		Sample f = ((double *)inputBuffer)[i];
		buf.push_back(f);
		if (g_stereo)
			buf.push_back(f);
	}
	((DataBuffer<Sample> *)userData)->push(move(buf));
	return 0;
}

bool AudioInput::init(std::string device, int pcmrate, bool stereo ,DataBuffer<Sample>	*AudioBuffer)
{
	if (this->getDeviceCount() < 1) {
		std::cout << "\nNo audio devices found!\n";
		m_zombie = true;
		return false;
	}		
	parameters.deviceId = this->getDefaultInputDevice();
/*	if (parameters.deviceId)
	{
		m_zombie = true;
		return false;
	}*/
	g_stereo = m_stereo = stereo;
	databuffer = AudioBuffer; 
	parameters.nChannels = 1;
	parameters.firstChannel = 0;
	sampleRate = pcmrate;
	bufferFrames = 1024;  // 256 sample frames
return true;
}

bool AudioInput::open()
{
	try {
		this->openStream(NULL, &parameters, RTAUDIO_FLOAT64, sampleRate, &bufferFrames, &record, (void *)databuffer);
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
const double cw_keyer_sidetone_frequency {1000.0};

double AudioInput::Nexttone()
{
	double angle = (asteps*cw_keyer_sidetone_frequency)*TWOPIOVERSAMPLERATE;
	if (++asteps == 48000) asteps = 0;
	return sin(angle);
}

void AudioInput::ToneBuffer()
{
	SampleVector	buf;
	for (int i = 0; i < bufferFrames; i++)
	{
		Sample f = (Sample) Nexttone();
		buf.push_back(f);
		if (m_stereo)
			buf.push_back(f);
	}
	databuffer->push(move(buf));
}