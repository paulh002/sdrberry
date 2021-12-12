#include "sdrberry.h"

const auto startTime = std::chrono::high_resolution_clock::now();
auto timeLastPrint = std::chrono::high_resolution_clock::now();
unsigned long long totalSamples(0);
double sampleRate {0.0};	
mutex mSampleRate;

double get_audio_sample_rate()
{
	unique_lock<mutex> lock_stream(mSampleRate);
	return sampleRate;
}

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
	totalSamples += samples.size() /2; 
	samples.clear();
	samples.resize(0);
	
	// Calculate the real audio samplerate
	const auto now = std::chrono::high_resolution_clock::now();
	//if (timeLastPrint + std::chrono::seconds(5) < now)
	//{
		unique_lock<mutex> lock_stream(mSampleRate);
		timeLastPrint = now;
		const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
		sampleRate = 1000000.0 * double(totalSamples) / timePassed.count();
		if (sampleRate < 38000.0 || sampleRate > 50000.0)
			sampleRate = 48000.0;
		//printf("Audio: %g sps queued samples %d \n", sampleRate, audio_output->queued_samples());
	//}
	return 0;
}


bool AudioOutput::init(std::string device, int pcmrate, DataBuffer<Sample>	*AudioBuffer)
{
	if (this->getDeviceCount() < 1) {
		std::cout << "\nNo audio devices found!\n";
		m_zombie = true;
		return false;
	}
	parameters.deviceId = this->getDefaultOutputDevice();
	parameters.nChannels = 2;
	parameters.firstChannel = 0;
	m_sampleRate = pcmrate;
	bufferFrames = 1024;   // 256 sample frames
	sampleRate = 0.99 * pcmrate;
	databuffer = AudioBuffer;
	return true;
}


bool AudioOutput::open()
{
	try {
		this->openStream(&parameters, NULL, RTAUDIO_FLOAT64, m_sampleRate, &bufferFrames, &Audioout, (void *)databuffer);
		this->startStream();
		printf("bufferFrames set: %d\n", bufferFrames);
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
	//if (databuffer->queued_samples() > 4096)
	//	printf("audio buffer queued samples %u\n", databuffer->queued_samples());
	return true;
}

int	 AudioOutput::queued_samples()
{
	if (databuffer != nullptr)
		return databuffer->queued_samples();
	return 0;
}