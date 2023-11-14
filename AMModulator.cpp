#include "AMModulator.h"
#include "Spectrum.h"
#include "gui_speech.h"
#include "PeakLevelDetector.h"
#include "gui_ft8bar.h"
#include <chrono>
#include <ctime>
#include "IQGenerator.h"

shared_ptr<AMModulator> sp_ammod;

void AMModulator::setsignal(vector<float> &signal)
{
	signal = std::move(signal);
}

bool AMModulator::create_modulator(ModulatorParameters param, DataBuffer<IQSample> *source_buffer, AudioInput *audio_input)
{	
	if (sp_ammod != nullptr)
		return false;
	sp_ammod = make_shared<AMModulator>(param, source_buffer, audio_input);
	sp_ammod->ammod_thread = std::thread(&AMModulator::operator(), sp_ammod);
	return true;
}

void AMModulator::destroy_modulator()
{
	if (sp_ammod == nullptr)
		return;
	sp_ammod->stop_flag = true;
	sp_ammod->ammod_thread.join();
	sp_ammod.reset();
}

AMModulator::~AMModulator()
{
	if (AMmodulatorHandle)
		ampmodem_destroy(AMmodulatorHandle);
	if (m_fft != nullptr)
		nco_crcf_destroy(m_fft);
	audio_input->set_tone(audioTone::NoTone);
}

AMModulator::AMModulator(ModulatorParameters &param, DataBuffer<IQSample> *source_buffer, AudioInput *audio_input)
	: Demodulator(param.ifrate, source_buffer, audio_input)
{
	float					mod_index = 0.99f; // modulation index (bandwidth)
	float					As = 60.0f; // resampling filter stop-band attenuation [dB]
	int						suppressed_carrier;
	liquid_ampmodem_type	am_mode;

	digitalmode = false;
	switch (param.mode)
	{
	case mode_ft4:
	case mode_ft8:
		even = param.even;
		signal = std::move(param.signal);
		digitalmode = true;
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_USB;
		printf("digital tx mode LIQUID_AMPMODEM_USB carrier %d\n", suppressed_carrier);
		break;

	case mode_usb:
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_USB;
		printf("tx mode LIQUID_AMPMODEM_USB carrier %d\n", suppressed_carrier);		
		break;
	case mode_lsb:
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_LSB;
		printf("tx mode LIQUID_AMPMODEM_LSB carrier %d\n", suppressed_carrier);		
		break;
	case mode_cw:
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_LSB;
		printf("mode CW LIQUID_AMPMODEM_LSB carrier %d\n", suppressed_carrier);
		break;
	case mode_am:
		suppressed_carrier = 0;
		am_mode = LIQUID_AMPMODEM_DSB;
		printf("tx mode LIQUID_AMPMODEM_DSB carrier %d\n", suppressed_carrier);		
		break;
	case mode_dsb:
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_DSB;
		printf("tx mode LIQUID_AMPMODEM_DSB carrier %d\n", suppressed_carrier);		
		break;
	default:
		printf("Mode not correct\n");		
		return;
	}
	audio_input->set_tone(param.tone);
	printf("tone %d \n", param.tone);
	setLowPassAudioFilterCutOffFrequency(2500);
	if ((param.ifrate - audio_input->get_samplerate()) > 0.1)
	{
		float sample_ratio, sample_ratio1;
		
		// only resample and tune if ifrate > pcmrate
		tune_offset(vfo.get_vfo_offset());

		sample_ratio1 = param.ifrate / (double)audio_input->get_samplerate();
		std::string sampleratio = Settings_file.get_string(default_radio, "resamplerate");
		sscanf(sampleratio.c_str(), "%f", &sample_ratio);
		if (abs(sample_ratio1 - sample_ratio) > 0.1)
			sample_ratio = sample_ratio1;

		set_resample_rate(sample_ratio); // UP sample to ifrate		
	}
	else
	{	// mix the transmid signal to the mid of the fft display
		//fft_offset(ifrate / 4);
	}
	setBandPassFilter(2700.0f, 2000.0f, 500.0f, 150.0f);
	AMmodulatorHandle = ampmodem_create(mod_index, am_mode, suppressed_carrier); 
	source_buffer->restart_queue();
}

void AMModulator::operator()()
{
	const auto startTime = std::chrono::high_resolution_clock::now();
	auto timeLastPrint = std::chrono::high_resolution_clock::now();
	
	unsigned int            fft_block = 0;
	bool                    inbuf_length_warning = false;
	SampleVector            audiosamples;
	IQSampleVector			samples_out;
	AudioProcessor			Speech;
	IQGenerator IqGenerator(ifrate, audioInputBuffer);

	Speech.prepareToPlay(audio_output->get_samplerate());
	Speech.setThresholdDB(gspeech.get_threshold());
	Speech.setRatio(gspeech.get_ratio());
	tune_offset(vfo.get_vfo_offset());
	audioInputBuffer->clear();
	if (gspeech.get_speech_mode())
		audioInputBuffer->set_gain(0);
	if (digitalmode)
	{
		//cout << "Wait for Timeslot \n";
		//WaitForTimeSlot();
		audioInputBuffer->clear();
		audioInputBuffer->StartDigitalMode(signal);
		cout << "Start digital transmit \n";
	}
	while (!stop_flag.load())
	{
		if (vfo.tune_flag.load())
		{
			vfo.tune_flag = false;
			tune_offset(vfo.get_vfo_offset());
		}

		if (audioInputBuffer->IsBufferEmpty() && digitalmode)
		{
			stop_flag = true;
			cout << "Stop digital transmit \n";
			audioInputBuffer->StopDigitalMode();
		}

		if (!audioInputBuffer->read(audiosamples))
			continue;
		
		if (gspeech.get_speech_mode() && !digitalmode)
		{
			Speech.setRelease(gspeech.get_release());
			Speech.setRatio(gspeech.get_ratio());
			Speech.setAtack(gspeech.get_atack());
			Speech.setThresholdDB(gspeech.get_threshold());
			Speech.processBlock(audiosamples);
		}
		else
			audioInputBuffer->set_gain(0);

		calc_af_level(audiosamples);
		set_signal_strength();
		if (audioInputBuffer->get_tone() != audioTone::FourTone)
			process(audiosamples, samples_out);
		else
		{
			samples_out = IqGenerator.generateIQVectors(4, 45.0f, 48000);
			SpectrumGraph.ProcessWaterfall(samples_out);
		}

		adjust_calibration(samples_out);
		transmitIQBuffer->push(move(samples_out));
		audiosamples.clear();
		
		const auto now = std::chrono::high_resolution_clock::now();
		if (timeLastPrint + std::chrono::seconds(5) < now)
		{
			timeLastPrint = now;
			const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);			
			printf("Queued transmitbuffer Samples %lu\n", transmitIQBuffer->queued_samples());
			printf("peak %f db gain %f db threshold %f ratio %f atack %f release %f\n", Speech.getPeak(), Speech.getGain(), Speech.getThreshold(), Speech.getRatio(), Speech.getAtack(), Speech.getRelease());
		}
	}
	transmitIQBuffer->clear();
	transmitIQBuffer->push_end();
	
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	time_t tt = std::chrono::system_clock::to_time_t(now);
	tm local_tm = *localtime(&tt);
	printf("exit am_mod_thread %2d:%2d:%2d\n", local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec);
}

void AMModulator::process(const SampleVector &samples, IQSampleVector &samples_out)
{
	IQSampleVector buf_mod, buf_filter, buf_out;
	unsigned int num_written;

	// Modulate audio to USB, LSB or DSB;
	for (auto &col : samples)
	{
		complex<float> f;
		ampmodem_modulate(AMmodulatorHandle, col, &f);
		//printf("audio %f;I %f;Q %f \n", col, f.real(), f.imag());
		buf_mod.push_back(f); 
	}
	if (digitalmode)
		guift8bar.Process(buf_mod);
	executeBandpassFilter(buf_mod, buf_filter);
	Resample(buf_filter, buf_out);
	mix_up(buf_out, samples_out); // Mix up to vfo freq
	SpectrumGraph.ProcessWaterfall(samples_out);
}

void AMModulator::mix_up_fft(const IQSampleVector& filter_in,
	IQSampleVector& filter_out)
{	
	if (m_fft)
	{
		for (auto& col : filter_in)
		{
			complex<float> v;
		
			nco_crcf_step(m_fft);
			nco_crcf_mix_up(m_fft, col, &v);
			filter_out.push_back(v);
		}
	}
	else
	{
		filter_out = filter_in;
	}
}

void AMModulator::fft_offset(long offset)
{
	if (m_fft != nullptr)
		nco_crcf_destroy(m_fft);
	float rad_per_sample   = ((2.0f * (float)M_PI * (float)(offset)) / (float)ifSampleRate);
	m_fft = nco_crcf_create(LIQUID_NCO);
	nco_crcf_set_phase(m_fft, 0.0f);
	nco_crcf_set_frequency(m_fft, rad_per_sample);
}

void AMModulator::audio_feedback(const SampleVector &audiosamples)
{
	
	for (auto &col : audiosamples)
	{
		// split the stream in blocks of samples of the size framesize
		audioframes.insert(audioframes.end(), col);
		if (audioframes.size() == audio_output->get_framesize())
		{
			SampleVector audio_stereo;
			
			mono_to_left_right(audioframes, audio_stereo);
			audio_output->write(audio_stereo);
			audioframes.clear();
		}
	}
}

void AMModulator::WaitForTimeSlot()
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	time_t tt = std::chrono::system_clock::to_time_t(now);
	int sec = ((long long)tt % 15);
	while (sec != 0)
	{
		usleep(1000);
		now = std::chrono::system_clock::now();
		tt = std::chrono::system_clock::to_time_t(now);
		sec = ((long long)tt % 15);
	}

	now = std::chrono::system_clock::now();
	tt = std::chrono::system_clock::to_time_t(now);
	tm local_tm = *localtime(&tt);
	printf("start tx cycle %2d:%2d:%2d\n", local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec);
}