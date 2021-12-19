#include "AudioInput.h"

static const auto startTime = std::chrono::high_resolution_clock::now();
static auto timeLastPrint = std::chrono::high_resolution_clock::now();
static unsigned long long totalSamples(0);
static double sampleRate {0.0};	

double get_audio_input_rate()
{
	return sampleRate;
}

int record(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status, void *userData)
{
	AudioInput					*audioinput = (AudioInput *)userData ;
	DataBuffer<Sample>			*databuffer = audioinput->get_databuffer();
	
	if (status)
		std::cout << "Stream overflow detected!" << std::endl;
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
	databuffer->push(move(buf));
	
	const auto now = std::chrono::high_resolution_clock::now();
	timeLastPrint = now;
	const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
	sampleRate = 1000000.0 * double(totalSamples) / timePassed.count();
	if (sampleRate < 38000.0 || sampleRate > 50000.0)
		sampleRate = 48000.0;
	
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
	m_stereo = stereo;
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

void AudioInput::ToneBuffer(int twotone)
{
	SampleVector	buf;
	for (int i = 0; i < bufferFrames; i++)
	{
		Sample f;
		
		if (twotone == 2)
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