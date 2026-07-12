#include "AudioInput.h"
#include "AudioOutput.h"

AudioInput *audio_input;

int AudioInput::createAudioInputDevice(int SampleRate, unsigned int bufferFrames, std::string dev, int volume)
{
	RtAudio::Api selectedApi = RtAudio::LINUX_PULSE;
	int deviceID = 0;
	struct DeviceInfo devinfo;
	std::vector<std::string> devices;

	std::cout << "[Audio] Testing API: " << RtAudio::getApiName(RtAudio::LINUX_PULSE) << "\n";
	RtAudio tempAdc(RtAudio::LINUX_PULSE);
	unsigned int tmp_devices = tempAdc.getDeviceCount();
	if (tmp_devices == 0)
	{
		std::cout << "[Audio] API Error: No devices found. Pipewire may be inactive.\n";
		selectedApi = RtAudio::LINUX_ALSA;
	}
	audio_input = new AudioInput(SampleRate, bufferFrames, false, selectedApi);

	if (audio_input)
	{
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
		audio_input->set_volume(volume);
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
	else if (IsdigitalMode())
	{
		doDigitalMode();
		return 0;
	}
	else if (playbackmode)
	{
		doPlayRecording();
		return 0;
	}
	else
	{
		audio_in_buffer.push_block(nBufferFrames, [&](std::span<float> buffer) {
			for (int i = 0; i < nBufferFrames; i++)
			{
				Sample f = ((float *)inputBuffer)[i];
				buffer[i] = f;
			}
			float mic_vol = 0;
			for (auto con : buffer)
			{
				mic_vol += con * con;
			}
			mic_volume.store(mic_vol / buffer.size());
		});
		return 0;
	}
}

float AudioInput::get_mic_vol()
{
	return 20.0 * logf(mic_volume.load());
}

void AudioInput::listDevices(std::vector<std::string> &devices)
{
	struct DeviceInfo	dev;
		
	std::vector<unsigned int> ids = this->getDeviceIds();	
	printf("List devices: \n");
	int default_dev = 1;
	for (auto col : ids)
	{
		dev = getDeviceInfo(col);
		printf("device %s out %d in %d \n", dev.name.c_str(), dev.outputChannels, dev.inputChannels);
		if (!default_dev && (dev.name.find("Default ALSA Device") != std::string::npos || dev.name.find("PulseAudio Sound Server") != std::string::npos))
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
		std::cout << "Audio Input: No devices found." << std::endl;
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
			printf("Found audio input device %s default %d\n", dev.name.c_str(), dev.isDefaultInput);
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
	stereo = false;
	playbackmode = false;
	tone_volume = 1.0;
	digitalvolume = 1.0;
	playback_volume = 1.0;
	audio_in_buffer.reserve(bufferFrames);
	gain_buffer.reserve(bufferFrames);
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
	
	StreamOptions option{{0}, {0}, {0}, {0}};
	//option.flags = RTAUDIO_MINIMIZE_LATENCY;
	option.streamName = "Sdrberry";

	parameters.deviceId = deviceId;
	info = getDeviceInfo(parameters.deviceId);
	if (info.inputChannels > 0)
	{
		err = openStream(NULL, &parameters, RTAUDIO_FLOAT32, sampleRate, &bufferFrames, AudioIn, (void *)this, &option);
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
	volume = expf(((float)vol * 6.908) / 100.0) / 100.0;
	//printf("mic vol %f\n", (float)volume);
}

void AudioInput::set_digital_volume(int vol)
{
	// log volume
	digitalvolume = expf(((float)vol * 6.908) / 100.0) / 1000.0;
}

void AudioInput::set_tone_volume(int vol)
{
	// log volume
	tone_volume = expf(((float)vol * 6.908) / 100.0) / 100.0;

}

void AudioInput::set_playback_volume(int vol)
{
	// log volume
	playback_volume = expf(((float)vol * 6.908) / 100.0) / 1000.0;
}

void AudioInput::adjust_gain(std::span<Sample> samples)
{
	for (auto &con : samples) 
	{
		if (digitalmode)
		{
			con *= digitalvolume;
		}
		else if (get_tone())
		{
			con *= tone_volume;
		}
		else if (playbackmode)
		{
			con *= playback_volume;
		}
		else
		{
			con *= volume;
		}
	}
}

std::span<Sample> AudioInput::read()
{
	std::span<Sample> ret {};
		
	if (!isStreamOpen())
		return ret;
	ret = audio_in_buffer.get_buffer_view(audio_in_buffer.pop_block());
	adjust_gain(ret);
	return ret;
}

void AudioInput::close()
{
	if (isStreamOpen())
	{
		abortStream();
		closeStream();
	}
}

AudioInput::~AudioInput()
{
	close();
}

#define TWOPIOVERSAMPLERATE 0.0001308996938995747;  // 2 Pi / 48000
const float cw_keyer_sidetone_frequency {1500.0};
const float cw_keyer_sidetone_frequency2 {750.0};

float AudioInput::Nexttone()
{
	float angle = (asteps*cw_keyer_sidetone_frequency)*TWOPIOVERSAMPLERATE;
	if (++asteps >= 64) asteps = 0;
	return sin(angle) / 10.0;
}

void AudioInput::ToneBuffer()
{
	audio_in_buffer.push_block(bufferFrames,[&](std::span<float> buffer) {
		for (int i = 0; i < bufferFrames; i++)
		{	Sample f;

			if (tune_tone == TwoTone)
			{
				f = (Sample) NextTwotone();
			}
			else
			{
				f = (Sample) Nexttone();	
			}
			buffer[i] = f;
		}
		float mic_vol = 0;
		for (auto con : buffer)
		{
			mic_vol += con * con;
		}
		mic_volume.store(mic_vol / buffer.size());
	});
}

void AudioInput::StartDigitalMode(std::vector<float> &signal)
{
	if (digitalmode != false)
	{
		std::cout << "Digital mode already started \n";
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

int AudioInput::StartPlayback(std::string filename)
{
	playbackmode = true;
	bufferempty = false;
	if (!wavereader.open(filename))
	{
		printf("Audio file not found\n");
		playbackmode = false;
		bufferempty = true;
		return 1;
	}
	std::cout << "Audio file sample Rate: " << wavereader.getSampleRate() << " Hz\n";
	std::cout << "Audio file total Samples: " << wavereader.getTotalSamples() << "\n";
	return 0;
}

void AudioInput::StopPlayback()
{
	playbackmode = false;
	bufferempty = true;
	wavereader.close();
}

void AudioInput::doDigitalMode()
{
	std::span<float> buf = audio_in_buffer.get_buffer_view(audio_in_buffer.pop_block());

	if (digitalmode == false || bufferempty)
		return ;
	audio_in_buffer.push_block(bufferFrames,[&](std::span<float> buffer) {
		int i = 0;
		for (auto &con : buffer)
		{
			if ((i + bufferFramesSend * bufferFrames) < digitalmodesignal.size())
				con = ((Sample)digitalmodesignal.at(i + bufferFramesSend * bufferFrames));
			else
				con = 0.0;
			i++;
		}
	});	
	bufferFramesSend++;
	//cout << "bufferframes send " << bufferFramesSend << endl;
	audio_output->adjust_gain(buf);
	audio_output->writeSamples(buf);
	if ((bufferFramesSend * bufferFrames) >= digitalmodesignal.size())
	{
		//cout << "all ft8 audio samples streamed\n";
		bufferFramesSend = 0;
		bufferempty = true;
	}
}

void AudioInput::doPlayRecording()
{
	audio_in_buffer.push_block(bufferFrames,[&](std::span<float> buffer) {
		if (!wavereader.readChunk(buffer, getbufferFrames()))
		{
			bufferempty = true;
			playbackmode = false;
		}
		if (wavereader.isEOF())
		{
			bufferempty = true;
			playbackmode = false;
		}
		audio_output->adjust_gain(buffer, gain_buffer);
		audio_output->writeSamples(gain_buffer);	
	});	
}

float AudioInput::NextTwotone()
{
	float angle = (asteps*cw_keyer_sidetone_frequency)*TWOPIOVERSAMPLERATE;
	float angle2 = (asteps*cw_keyer_sidetone_frequency2)*TWOPIOVERSAMPLERATE;
	if (++asteps >= 64) asteps = 0;
	return (sinf(angle) + sinf(angle2)) / 10.0;
}

int	 AudioInput::queued_samples()
{
	return audio_in_buffer.size();
}

void AudioInput::clear()
{
	audio_in_buffer.clear();
}
