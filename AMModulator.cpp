#include "AMModulator.h"
#include "Spectrum.h"
#include "gui_speech.h"
#include "PeakLevelDetector.h"
#include "gui_ft8bar.h"
#include <chrono>
#include <ctime>
#include "IQGenerator.h"
#include "vfo.h"
#include "sdrberry.h"

std::shared_ptr<AMModulator> sp_ammod;

#define DEBUG_VECTOR(v) std::cout << #v << " size: " << v.size() << ", capacity: " << v.capacity() << '\n';

void set_realtime_priority(std::thread& thread, int priority);
void pin_thread_to_core(std::thread& thread, int core_id);

void AMModulator::setsignal(std::vector<float> &signal)
{
	signal = std::move(signal);
}

bool AMModulator::create_modulator(ModulatorParameters param, DataBuffer<IQSample> *source_buffer, AudioInput *audio_input)
{	
	if (sp_ammod != nullptr)
		return false;
	sp_ammod = make_shared<AMModulator>(param, source_buffer, audio_input);
	sp_ammod->ammod_thread = std::thread(&AMModulator::operator(), sp_ammod);
	pin_thread_to_core(sp_ammod->ammod_thread, 2);
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
	float					mod_index = 0.9f; // modulation index (bandwidth)
	float					As = 60.0f; // resampling filter stop-band attenuation [dB]
	int						suppressed_carrier;

	digitalmode = false;
	audio_file_mode = false;
	duplex = param.duplex;
	switch (param.mode)
	{
	case mode_ft4:
	case mode_ft8:
		even = param.even;
		signal = std::move(param.signal);
		digitalmode = true;
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_USB;
		printf("digital tx mode LIQUID_AMPMODEM_USB carrier %d ifrate %f\n", suppressed_carrier, param.ifrate);
		break;

	case mode_usb:
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_USB;
		printf("tx mode LIQUID_AMPMODEM_USB carrier %d\n", suppressed_carrier);
		setBandPassFilter(3000.0f, 100.0f);
		break;
	case mode_lsb:
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_LSB;
		printf("tx mode LIQUID_AMPMODEM_LSB carrier %d\n", suppressed_carrier);
		setBandPassFilter(3000.0f, 100.0f);
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
		setBandPassFilter(3000.0f, 100.0f);
		break;
	case mode_dsb:
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_DSB;
		printf("tx mode LIQUID_AMPMODEM_DSB carrier %d\n", suppressed_carrier);
		setBandPassFilter(3000.0f, 100.0f);
		break;
	default:
		printf("Mode not correct\n");		
		return;
	}
	audio_input->set_tone(param.tone);
	modulatorbuffer.resize(audio_input->getbufferFrames());
	play_prerecorded_file = param.play_prerecorded_file;
	if ((param.ifrate - audio_input->get_samplerate()) > 0.1)
	{
		float sample_ratio;
		
		// only resample and tune if ifrate > pcmrate
		tune_offset(vfo.get_vfo_offset_tx());
		sample_ratio = param.ifrate / (double)audio_input->get_samplerate();
		printf("ifrate %f, audiorate %d  Resample rate %f\n", param.ifrate, audio_input->get_samplerate(), sample_ratio);
		set_resample_rate(sample_ratio); // UP sample to ifrate
		samples_out.resize(sample_ratio * audio_input->getbufferFrames() * 2, 0); //* 2 is savety margin
	}
	else
	{
		printf("No resample \n");
		samples_out.resize( audio_input->getbufferFrames(),0); 
	}
	AMmodulatorHandle = ampmodem_create(mod_index, am_mode, suppressed_carrier); 
	source_buffer->restart_queue();
}

void AMModulator::operator()()
{
	const auto startTime = std::chrono::high_resolution_clock::now();
	auto timeLastPrint = std::chrono::high_resolution_clock::now();
	auto timeLastMeasure = std::chrono::high_resolution_clock::now();
	
	unsigned int            fft_block = 0;
	bool                    inbuf_length_warning = false;
	SampleVector            buf_out;
	std::span<Sample>		audiosamples;
	IQSampleVector			samples_in, samples_out2;
	AudioProcessor			Speech;
	//IQGenerator IqGenerator(48000, audioInputBuffer);

	Speech.prepareToPlay(audio_output->get_samplerate());
	Speech.setThresholdDB(gspeech.get_threshold());
	Speech.setRatio(gspeech.get_ratio());
	tune_offset(vfo.get_vfo_offset_tx());
	audioInputBuffer->clear();
	audio_file_mode = false;
	if (play_prerecorded_file.size() > 0)
	{
		if (!audioInputBuffer->StartPlayback(play_prerecorded_file))
			audio_file_mode = true;
	}
	
	if (digitalmode)
	{
		audioInputBuffer->clear();
		audioInputBuffer->StartDigitalMode(signal);
		std::cout << "Start digital transmit \n";
	}

	while (!stop_flag.load())
	{
		if (vfo.tune_flag.load())
		{
			vfo.tune_flag = false;
			tune_offset(vfo.get_vfo_offset_tx());
		}

		if (audioInputBuffer->IsBufferEmpty() && digitalmode)
		{
			stop_flag = true;
			std::cout << "Stop digital transmit \n";
			audioInputBuffer->StopDigitalMode();
		}

		if (audioInputBuffer->IsBufferEmpty() && audio_file_mode)
		{
			stop_flag = true;
			std::cout << "Stop playback transmit \n";
			audioInputBuffer->StopPlayback();
		}
		
		audiosamples = audioInputBuffer->read();
		if (gspeech.get_speech_mode() && !digitalmode)
		{
			Speech.setRelease(gspeech.get_release());
			Speech.setRatio(gspeech.get_ratio());
			Speech.setAtack(gspeech.get_atack());
			Speech.setThresholdDB(gspeech.get_threshold());
			Speech.processBlock(audiosamples);
		}
		calc_af_signalstrength(audiosamples);
		process(audiosamples, samples_out);
		//samples_out = IqGenerator.generateTwoToneIQVectors(1, 0, 1500, 750);
		adjust_calibration(samples_out);
		int number_of_samples = samples_out.size();
		int number_of_audio_samples = audiosamples.size();
		
		samples_out2 = samples_out;
		transmitIQBuffer->push(std::move(samples_out2));
		const auto now = std::chrono::high_resolution_clock::now();
		if (timeLastPrint + std::chrono::seconds(10) < now)
		{
			timeLastPrint = now;
			const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
			printf("Queued transmitbuffer Samples %lu number of audio samples %d number of samples %d\n", transmitIQBuffer->queued_samples(), number_of_audio_samples, number_of_samples);
			printf("peak %f db gain %f db threshold %f ratio %f atack %f release %f\n", Speech.getPeak(), Speech.getGain(), Speech.getThreshold(), Speech.getRatio(), Speech.getAtack(), Speech.getRelease());
		}
	}
	transmitIQBuffer->clear();
	transmitIQBuffer->push_end();
	if (audio_file_mode)
	{
		std::cout << "Close playback transmit\n";
		guiQueue.push_back(GuiMessage(GuiMessage::action::receive, 0));
		audioInputBuffer->StopPlayback();
		audio_file_mode = false;
	}
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	time_t tt = std::chrono::system_clock::to_time_t(now);
	tm local_tm = *localtime(&tt);
	printf("exit am_mod_thread %2d:%2d:%2d\n", local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec);
}

void AMModulator::process(std::span<Sample>samples, IQSampleVector &samples_out)
{
	unsigned int num_written;
	
	// Modulate audio to USB, LSB or DSB;
	if (!digitalmode && !audio_input->get_tone())
		executeBandpassFilter(samples);
	int i = 0;
	for (auto &col : samples)
	{
		std::complex<float> f;
		ampmodem_modulate(AMmodulatorHandle, col, &f);
		modulatorbuffer.at(i++) = f; 
		//printf("I %f;Q %f \n", f.real(), f.imag());
	}
	if (digitalmode)
		guift8bar.Process(modulatorbuffer);
	Resample_new(modulatorbuffer, samples_out);
	mix_up(samples_out); // Mix up to vfo freq
	if (!duplex)
		SpectrumGraph.ProcessWaterfall(samples_out);
}

void AMModulator::mix_up_fft(const IQSampleVector& filter_in,
	IQSampleVector& filter_out)
{	
	if (m_fft)
	{
		for (auto& col : filter_in)
		{
			std::complex<float> v;
		
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

void AMModulator::setLowPassAudioFilterCutOffFrequency(int bandwidth)
{
	if (sp_ammod != nullptr)
		sp_ammod->Demodulator::setLowPassAudioFilterCutOffFrequency(bandwidth);
}
