#include "sdrberry.h"

static const auto startTime = std::chrono::high_resolution_clock::now();
static auto timeLastPrint = std::chrono::high_resolution_clock::now();
static unsigned long long totalSamples(0);
static double sampleRate {0.0};	
mutex mSampleRate;
atomic<int> underrun{0};

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
		for (int i = 0; i < 2*nBufferFrames; i++)
		{
			((double *)buffer)[i] = 0.0;
		}
		underrun++;
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
		if (dev.outputChannels > 0 || dev.inputChannels > 0)
			devices.push_back(dev.name);
	}
}

int AudioOutput::getDevices(std::string device)
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

void AudioOutput::init_device(std::string device)
{
	if (device != "default")
		parameters.deviceId = getDevices(device);
	else
		parameters.deviceId = this->getDefaultOutputDevice(); 
}


bool AudioOutput::init(std::string device, int pcmrate, DataBuffer<Sample>	*AudioBuffer)
{
	if (this->getDeviceCount() < 1) {
		std::cout << "\nNo audio devices found!\n";
		return false;
	}
	if (device != "default")
		parameters.deviceId = getDevices(device);
	else
		parameters.deviceId = this->getDefaultOutputDevice();
	parameters.nChannels = 2;
	parameters.firstChannel = 0;
	m_sampleRate = pcmrate;
	bufferFrames = 1024;   // 256 sample frames
	sampleRate = 0.99 * pcmrate;
	databuffer = AudioBuffer;
	printf("Default audio device = %d\n", parameters.deviceId);
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