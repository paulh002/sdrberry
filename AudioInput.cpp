#include "AudioInput.h"
#include "AudioOutput.h"

AudioInput *audio_input;
DataBuffer<Sample> audioinput_buffer;

#define dB2mag(x) pow(10.0, (x) / 20.0)

bool AudioInput::createAudioInputDevice(int SampleRate)
{
	auto RtApi = RtAudio::LINUX_ALSA;
	audio_input = new AudioInput(SampleRate, false, &audioinput_buffer, RtApi);
	if (audio_input)
	{
		string s = Settings_file.find_audio("device");
		audio_input->open(s);
		audio_input->set_volume(Settings_file.micgain());
		return true;
	}
	fprintf(stderr, "ERROR: Cannot create AudioInputDevice\n");
	return false;
}

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
	if (audioinput->IsdigitalMode())
	{
		audioinput->doDigitalMode();
		return 0;
	}

	// Do something with the data in the "inputBuffer" buffer.
	//printf("frames %u \n", nBufferFrames);
	SampleVector	buf;
	for (int i = 0; i < nBufferFrames; i++)
	{
		Sample f = ((double *)inputBuffer)[i];
		buf.push_back(f);
		if (audioinput->get_stereo())
			buf.push_back(f);
	}
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
		printf("%d device: %s input %d output %d\n", col, info.name.c_str(), info.inputChannels, info.outputChannels);

		if (info.name.find(device) != std::string::npos && info.inputChannels > 0)
		{
			if (info.outputChannels < parameters.nChannels)
				parameters.nChannels = info.outputChannels;
			return col;
		}
	}
	std::cout << "No matching device found." << std::endl;
	return 0; // return default device
}

AudioInput::AudioInput(unsigned int pcmrate, bool stereo, DataBuffer<Sample> *AudioBuffer, RtAudio::Api api)
	: RtAudio(api), parameters{}, m_volume{0.5}, asteps{}, tune_tone{audioTone::NoTone}
{
	m_stereo = stereo;
	databuffer = AudioBuffer; 
	parameters.nChannels = 1;
	parameters.firstChannel = 0;
	sampleRate = pcmrate;
	bufferFrames = 2048;
	gaindb = 0;
	digitalmode = false;
	bufferempty = false;
	bufferFramesSend = 0;
}

std::vector<RtAudio::Api> AudioInput::listApis()
{
	std::vector<RtAudio::Api> apis;
	RtAudio ::getCompiledApi(apis);

	std::cout << "\nCompiled APIs:\n";
	for (size_t i = 0; i < apis.size(); i++)
		std::cout << i << ". " << RtAudio::getApiDisplayName(apis[i])
				  << " (" << RtAudio::getApiName(apis[i]) << ")" << std::endl;

	return apis;
}

bool AudioInput::open(std::string device)
{
	RtAudioErrorType err;

	if (this->getDeviceCount() < 1)
	{
		std::cout << "\nNo audio devices found!\n";
		return false;
	}
	if (device != "default")
		parameters.deviceId = getDevices(device);
	else
		parameters.deviceId = getDefaultInputDevice();

	err = openStream(NULL, &parameters, RTAUDIO_FLOAT64, sampleRate, &bufferFrames, &record, (void *)this);
	if (err != RTAUDIO_NO_ERROR)
	{
		printf("Cannot open audio input stream\n");
		return false;
	}
	startStream();
	printf("audio input device = %d %s samplerate %d\n", parameters.deviceId, device.c_str(), sampleRate);
	return true;	
}

bool AudioInput::open(int sampleRate, unsigned int device)
{
	RtAudioErrorType err;

	parameters.deviceId = device;
	parameters.nChannels = 1; //	info.inputChannels;
	err = openStream(NULL, &parameters, RTAUDIO_FLOAT64, sampleRate, &bufferFrames, &record, (void *)this);
	if (err != RTAUDIO_NO_ERROR)
	{
		printf("Cannot open audio input stream\n");
		return false;
	}
	startStream();
	printf("audio input device = %d %s samplerate %d\n", parameters.deviceId, info.name.c_str(), sampleRate);
	return true;
}

void AudioInput::set_volume(int vol)
{
	// log volume
	m_volume = exp(((double)vol * 6.908) / 100.0) / 5.0;
	printf("mic vol %f\n", (float)m_volume);
}

void AudioInput::adjust_gain(SampleVector& samples)
{
	for (unsigned int i = 0, n = samples.size(); i < n; i++) {
		samples[i] *= m_volume * dB2mag(gaindb);
	}
}


bool AudioInput::read(SampleVector& samples)
{
	if (databuffer == nullptr)
		return false;
	if (!isStreamOpen())
		return false;
	samples = databuffer->pull();
	if (samples.empty())
		return false;
	adjust_gain(samples);
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
const double cw_keyer_sidetone_frequency {1500.0};
const double cw_keyer_sidetone_frequency2 {750.0};

double AudioInput::Nexttone()
{
	double angle = (asteps*cw_keyer_sidetone_frequency)*TWOPIOVERSAMPLERATE;
	if (++asteps >= 64) asteps = 0;
	return sin(angle) / 200.0;
}

void AudioInput::ToneBuffer()
{
	SampleVector	buf;
	
	for (int i = 0; i < bufferFrames; i++)
	{
		Sample f;

		if (tune_tone == TwoTone)
		{
			f = (Sample) NextTwotone();
		}
		else
		{
			f = (Sample) Nexttone();	
		}
		buf.push_back(f);
	}
	databuffer->push(move(buf));
}

void AudioInput::StartDigitalMode(vector<float> &signal)
{
	if (digitalmode != false)
	{
		cout << "Digital mode already started \n";
		return;
	}
	digitalmode = true;
	bufferempty = false;
	digitalmodesignal = std::move(signal);
}

bool AudioInput::IsdigitalMode()
{
	return digitalmode;
}

bool AudioInput::IsBufferEmpty()
{
	return bufferempty;
}

void AudioInput::StopDigitalMode()
{
	digitalmode = false;
	bufferempty = false;
	bufferFramesSend = 0;
	digitalmodesignal.clear();
}

void AudioInput::doDigitalMode()
{
	SampleVector buf, buf_out;

	if (digitalmode == false || bufferempty)
		return ;

	for (int i = 0; i < bufferFrames; i++)
	{
		if ((i + bufferFramesSend * bufferFrames) < digitalmodesignal.size())
			buf.push_back((Sample)digitalmodesignal.at(i + bufferFramesSend * bufferFrames));
		else
			buf.push_back(0.0);
		//printf("sample %f \n", (Sample)digitalmodesignal.at(i + bufferFramesSend));
	}
	bufferFramesSend++;
	//cout << "bufferframes send " << bufferFramesSend << endl;
	audio_output->adjust_gain(buf, buf_out);
	audio_output->writeSamples(buf_out);
	databuffer->push(move(buf));
	if ((bufferFramesSend * bufferFrames) >= digitalmodesignal.size())
	{
		cout << "all ft8 audio samples streamed\n";
		bufferFramesSend = 0;
		bufferempty = true;
	}
}

double AudioInput::NextTwotone()
{
	double angle = (asteps*cw_keyer_sidetone_frequency)*TWOPIOVERSAMPLERATE;
	double angle2 = (asteps*cw_keyer_sidetone_frequency2)*TWOPIOVERSAMPLERATE;
	if (++asteps >= 64) asteps = 0;
	return (0.5 * sin(angle) + 0.5 * sin(angle2)) / 400.0;
}

int	 AudioInput::queued_samples()
{
	if (databuffer != nullptr)
		return databuffer->queued_samples();
	return 0;
}