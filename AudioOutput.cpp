#include "AudioOutput.h"


int Audioout( void *outputBuffer,void *inputBuffer,unsigned int nBufferFrames,double streamTime,RtAudioStreamStatus status,	void *userData)
{
	double *buffer = (double *) outputBuffer;
	
	if (status)
		std::cout << "Stream underflow detected!" << std::endl;
	// Write interleaved audio data.
	
	if(((DataBuffer<Sample> *)userData)->queued_samples() == 0)
	{
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
	return 0;
}


bool AudioOutput::init(std::string device, int pcmrate)
{
	if (this->getDeviceCount() < 1) {
		std::cout << "\nNo audio devices found!\n";
		m_zombie = true;
		return false;
	}
	parameters.deviceId = this->getDefaultOutputDevice();
	//if (parameters.deviceId == 0)
//	{
//		m_zombie = true;
//		return false;
//	}
	parameters.nChannels = 2;
	parameters.firstChannel = 0;
	sampleRate = pcmrate;
	bufferFrames = 1024;   // 256 sample frames
	return true;
}


bool AudioOutput::open(DataBuffer<Sample>	*AudioBuffer)
{
	databuffer = AudioBuffer;
	try {
		this->openStream(&parameters, NULL, RTAUDIO_FLOAT64, sampleRate, &bufferFrames, &Audioout, (void *)AudioBuffer);
		this->startStream();
		printf("bufferFrames set: %d", bufferFrames);
	}
	catch (RtAudioError& e) {
		e.printMessage();
		return false;
	}
	return true;	
}

void AudioOutput::adjust_gain(SampleVector& samples)
{
	for (unsigned int i = 0, n = samples.size(); i < n; i++) {
		samples[i] *= m_volume;
	}
}

void AudioOutput::close()
{
	stop();
	if (isStreamOpen()) 
		closeStream();
}

AudioOutput::~AudioOutput()
{
	close();
}

void AudioOutput::stop()
{
	if (isStreamOpen())
	{	
		try {
			// Stop the stream
			stopStream();
		}
		catch (RtAudioError& e) {
			e.printMessage();
		}
	}
}


bool AudioOutput::write(SampleVector& audiosamples)
{
	if (databuffer)
	{
		databuffer->push(move(audiosamples));
		//printf("queued audio vectors %d\n", databuffer->queued_samples());
	}
	audiosamples.clear();
	return true;
}