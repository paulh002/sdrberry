#include "AudioInput.h"
#include "AudioOutput.h"

AudioInput *audio_input;

int AudioInput::createAudioInputDevice(int SampleRate, unsigned int bufferFrames)
{
	auto RtApi = RtAudio::LINUX_ALSA;
	int deviceID = 0;
	struct DeviceInfo devinfo;
	std::vector<std::string> devices;

	audio_input = new AudioInput(SampleRate, bufferFrames, false, RtApi);
	if (audio_input)
	{
		std::string dev = Settings_file.find_audio("device");
		deviceID = audio_input->getAudioDevice(dev);
		if (!deviceID)
		{
			audio_input->listDevices(devices);
			if (devices.size() == 0)
				return 0;
			dev = devices.at(0);
			deviceID = audio_input->getAudioDevice(dev);
		}
		audio_input->open(deviceID);
		audio_input->set_volume(Settings_file.get_int("Radio", "micgain", 85));
		return deviceID;
	}
	fprintf(stderr, "ERROR: Cannot create AudioInputDevice\n");
	return 0; // 0 is no device found
}

int AudioInput::AudioIn_class(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames, double streamTime, RtAudioStreamStatus status)
{
	if (status)
		std::cout << "Stream overflow detected!" << std::endl;
	if (get_tone())
	{
		ToneBuffer();
		return 0;
	}
	if (IsdigitalMode())
	{
		doDigitalMode();
		return 0;
	}
	// Do something with the data in the "inputBuffer" buffer.
	//printf("frames %u \n", nBufferFrames);
	SampleVector	buf;
	for (int i = 0; i < nBufferFrames; i++)
	{
		Sample f = ((double *)inputBuffer)[i];
		buf.push_back(f);
		if (get_stereo())
			buf.push_back(f);
	}
	databuffer.clear();
	databuffer.push(std::move(buf));
	return 0;
}


void AudioInput::listDevices(std::vector<std::string> &devices)
{
	struct DeviceInfo	dev;
		
	std::vector<unsigned int> ids = this->getDeviceIds();	
	printf("List devices: \n");
	for (auto col : ids)
	{
		dev = getDeviceInfo(col);
		printf("device %s out %d in %d \n", dev.name.c_str(), dev.outputChannels, dev.inputChannels);
		if (dev.name.find("Default ALSA Device") != std::string::npos || dev.name.find("PulseAudio Sound Server") != std::string::npos)
		{
			printf("skip %d device: %s \n", col, dev.name.c_str());
			continue;
		}
		
		if (dev.outputChannels > 0 || dev.inputChannels > 0)
			devices.push_back(dev.name);
	}
}

int AudioInput::getAudioDevice(std::string device)
{
	int devno = 0;
	std::vector<unsigned int> ids = getDeviceIds();
	if (ids.size() == 0)
	{
		std::cout << "No devices found." << std::endl;
		return 0;
	}

	RtAudio::DeviceInfo dev;
	for (auto col : ids)
	{
		dev = getDeviceInfo(col);
		if (dev.name == device && dev.inputChannels > 0)
		{
			if (dev.outputChannels < parameters.nChannels)
				parameters.nChannels = dev.outputChannels;
			return col;
		}
	}
	std::cout << "No matching device found." << std::endl;
	return devno; // return default device
}


AudioInput::AudioInput(unsigned int pcmrate, unsigned int bufferFrames_, bool stereo_, RtAudio::Api api)
	: RtAudio(api), parameters{}, sampleRate{pcmrate}, bufferFrames{bufferFrames_}, volume{0.5}, asteps{}, tune_tone{audioTone::NoTone}, stereo{stereo_}
{
	parameters.nChannels = 1;
	parameters.firstChannel = 0;
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

bool AudioInput::open(int deviceId)
{
	RtAudioErrorType err;

	parameters.deviceId = deviceId;
	info = getDeviceInfo(parameters.deviceId);
	if (info.inputChannels > 0)
	{
		err = openStream(NULL, &parameters, RTAUDIO_FLOAT64, sampleRate, &bufferFrames, AudioIn, (void *)this);
		if (err != RTAUDIO_NO_ERROR)
		{
			printf("Cannot open audio input stream\n");
			return false;
		}
		int streamerror = startStream();
		printf("audio input device = %d %s samplerate %d channels %d streamerror %d\n", parameters.deviceId, info.name.c_str(), sampleRate, parameters.nChannels, streamerror);
		if (streamerror)
		{
			exit(0);
		}
		return true;
	}
	printf("No audio input device \n");
	return false;
}

void AudioInput::set_volume(int vol)
{
	// log volume
	volume = exp(((double)vol * 6.908) / 100.0) / 5.0;
	//printf("mic vol %f\n", (float)volume);
}

void AudioInput::set_digital_volume(int vol)
{
	// log volume
	digitalvolume = exp(((double)vol * 6.908) / 100.0) / 1000.0;
}

void AudioInput::adjust_gain(SampleVector& samples)
{
	for (unsigned int i = 0, n = samples.size(); i < n; i++) {
		if (digitalmode)
		{
			samples[i] *= digitalvolume;
		}
		else
		{
			samples[i] *= volume;
		}
	}
}

bool AudioInput::read(SampleVector& samples)
{
	if (!isStreamOpen())
		return false;
	samples = databuffer.pull();
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
	return sin(angle) / 210.0;
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
	databuffer.push(std::move(buf));
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
	databuffer.push(move(buf));
	if ((bufferFramesSend * bufferFrames) >= digitalmodesignal.size())
	{
		//cout << "all ft8 audio samples streamed\n";
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
	return databuffer.queued_samples();
}