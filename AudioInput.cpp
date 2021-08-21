#include "AudioInput.h"

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
		buf.push_back(f);
	}
	((DataBuffer<Sample> *)userData)->push(move(buf));
	return 0;
}

bool AudioInput::init(std::string device, int pcmrate)
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
	parameters.nChannels = 1;
	parameters.firstChannel = 0;
	sampleRate = pcmrate;
	bufferFrames = 1024;  // 256 sample frames
	databuffer = nullptr;
return true;
}

bool AudioInput::open(DataBuffer<Sample>	*AudioBuffer)
{
	databuffer = AudioBuffer;
	try {
		this->openStream(NULL, &parameters, RTAUDIO_FLOAT64, sampleRate, &bufferFrames, &record, (void *)AudioBuffer);
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
	if (databuffer->queued_samples() == 0)
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