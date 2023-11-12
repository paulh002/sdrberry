#include <thread>
#include "AMDemodulator.h"
#include "gui_bar.h"
#include "gui_agc.h"
#include "PeakLevelDetector.h"
#include "Limiter.h"
#include "SharedQueue.h"

static shared_ptr<AMDemodulator> sp_amdemod;
std::mutex amdemod_mutex;

static std::chrono::high_resolution_clock::time_point starttime1 {};

AMDemodulator::AMDemodulator(int mode, double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audioOutputBuffer)
	: Demodulator(ifrate, source_buffer, audioOutputBuffer), receiverMode(mode) 
{
	float					modulationIndex  = 0.03125f; 
	int						suppressed_carrier;
	liquid_ampmodem_type	am_mode;
	float bandwidth{2500}; // SSB

	float sample_ratio = (1.05 * (float)audio_output->get_samplerate()) / ifrate;
	Demodulator::set_resample_rate(sample_ratio); // down sample to pcmrate
	
	switch (mode)
	{
	case mode_usb:
		bandwidth = 2500; // SSB
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_USB;
		printf("mode LIQUID_AMPMODEM_USB carrier %d\n", suppressed_carrier);		
		break;
	case mode_cw:
		bandwidth = 500; // CW
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_LSB;
		printf("mode CW LIQUID_AMPMODEM_LSB carrier %d\n", suppressed_carrier);
		break;
	case mode_lsb:
		bandwidth = 2500; // SSB
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_LSB;
		printf("mode LIQUID_AMPMODEM_LSB carrier %d\n", suppressed_carrier);		
		break;
	case mode_am:
		bandwidth = 5000; // SSB
		suppressed_carrier = 0;
		am_mode = LIQUID_AMPMODEM_DSB;
		printf("mode LIQUID_AMPMODEM_DSB carrier %d\n", suppressed_carrier);		
		break;
	case mode_dsb:
		bandwidth = 5000; // SSB
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_DSB;
		printf("mode LIQUID_AMPMODEM_DSB carrier %d\n", suppressed_carrier);		
		break;
	default:
		printf("Mode not correct\n");		
		return;
	}
	const auto startTime = std::chrono::high_resolution_clock::now();
	Demodulator::setLowPassAudioFilterCutOffFrequency(bandwidth);
	Demodulator::setLowPassAudioFilter(audioSampleRate, bandwidth);
	demodulatorHandle = ampmodem_create(modulationIndex, am_mode, suppressed_carrier);
	gbar.set_filter_slider(bandwidth);
	pMDecoder = make_unique<MorseDecoder>(audioSampleRate);
	auto now = std::chrono::high_resolution_clock::now();
	const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
	cout << "starttime :" << timePassed.count() << endl;
	//catinterface.SetSH(m_bandwidth);
}


AMDemodulator::~AMDemodulator()
{
	printf("AM destructor called \n");
	if (demodulatorHandle != nullptr)
	{
		ampmodem_destroy(demodulatorHandle);
		demodulatorHandle = nullptr;	
	}
}
	
void AMDemodulator::operator()()
{	
	const auto startTime = std::chrono::high_resolution_clock::now();
	auto timeLastPrint = std::chrono::high_resolution_clock::now();
	auto timeLastFlashGainSlider = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point now, start1, start2;
	
	AudioProcessor Agc;
	
	int lowPassAudioFilterCutOffFrequency {-1}, span, droppedFrames {0};
	SampleVector            audioSamples, audioFrames;
	unique_lock<mutex>		lock_am(amdemod_mutex);
	IQSampleVector			dc_iqsamples, iqsamples;
	long long pr_time{0};
	int vsize, passes{0};

	int limiterAtack = Settings_file.get_int(Limiter::getsetting(), "limiterAtack", 10);
	int limiterDecay = Settings_file.get_int(Limiter::getsetting(), "limiterDecay", 500);
	Limiter limiter(limiterAtack, limiterDecay, ifSampleRate);
	int thresholdDroppedFrames = Settings_file.get_int(default_radio, "thresholdDroppedFrames", 15);
	int thresholdUnderrun = Settings_file.get_int(default_radio, "thresholdUnderrun", 5);

	pNoisesp = make_unique<SpectralNoiseReduction>(audioSampleRate, tuple<float,float>(0, 2500));
	//pLMS = make_unique<LMSNoisereducer>(); switched off memory leak in library
	pXanr = make_unique<Xanr>();
	Agc.prepareToPlay(audioOutputBuffer->get_samplerate());
	Agc.setThresholdDB(gagc.get_threshold());
	Agc.setRatio(10);
	set_span(gsetup.get_span());
	receiveIQBuffer->clear();
	audioOutputBuffer->clear_underrun();
	while (!stop_flag.load())
	{
		start1 = std::chrono::high_resolution_clock::now();
		span = gsetup.get_span();
		if (vfo.tune_flag.load() || m_span != span)
		{
			vfo.tune_flag = false;
			tune_offset(vfo.get_vfo_offset());
			set_span(span);
		}

		if (lowPassAudioFilterCutOffFrequency != get_lowPassAudioFilterCutOffFrequency())
		{
			lowPassAudioFilterCutOffFrequency = get_lowPassAudioFilterCutOffFrequency();
			printf("set filter %d\n", lowPassAudioFilterCutOffFrequency);
			setLowPassAudioFilter(audioSampleRate, lowPassAudioFilterCutOffFrequency);
		}

		dc_iqsamples = receiveIQBuffer->pull();
		if (dc_iqsamples.empty())
		{
			//printf("No samples queued 2\n");
			usleep(5000);
			continue;
		}
		dc_filter(dc_iqsamples,iqsamples);
		int nosamples = iqsamples.size();
		passes++;
		adjust_gain_phasecorrection(iqsamples, gbar.get_if());;
		limiter.Process(iqsamples);
		perform_fft(iqsamples);
		process(iqsamples, audioSamples);
		set_signal_strength();
		if (gagc.get_agc_mode())
		{
			Agc.setRelease(gagc.get_release());
			Agc.setRatio(gagc.get_ratio());
			Agc.setAtack(gagc.get_atack());
			Agc.setThresholdDB(gagc.get_threshold());
			Agc.processBlock(audioSamples);
		}
		// Set nominal audio volume.
		audioOutputBuffer->adjust_gain(audioSamples);
		int noaudiosamples = audioSamples.size();
		for (auto &col : audioSamples)
		{
			// split the stream in blocks of samples of the size framesize
			audioFrames.insert(audioFrames.end(), col);
			if (audioFrames.size() == audioOutputBuffer->get_framesize())
			{
				if ((audioOutputBuffer->queued_samples() / 2) < get_audioBufferSize())
				{
					SampleVector		audioStereoSamples, audioNoiseSamples;

					switch (gbar.get_noise())
					{
					case 1:
						pXanr->Process(audioFrames, audioNoiseSamples);
						break;
					case 2:
						pNoisesp->Process(audioFrames, audioNoiseSamples);
						mono_to_left_right(audioNoiseSamples, audioStereoSamples);
						break;
					case 3:
						pNoisesp->Process_Kim1_NR(audioFrames, audioNoiseSamples);
						mono_to_left_right(audioNoiseSamples, audioStereoSamples);
						break;
					default:
						mono_to_left_right(audioFrames, audioStereoSamples);
						break;
					}
					audioOutputBuffer->write(audioStereoSamples);
					audioFrames.clear();
				}
				else
				{
					droppedFrames++;
					audioFrames.clear();
				}
			}
		}
		dc_iqsamples.clear();
		iqsamples.clear();
		audioSamples.clear();
		now = std::chrono::high_resolution_clock::now();
		auto process_time1 = std::chrono::duration_cast<std::chrono::microseconds>(now - start1);
		if (pr_time < process_time1.count())
			pr_time = process_time1.count();

		if (timeLastFlashGainSlider + std::chrono::milliseconds(500) < now)
		{// toggle collor of gain slider when signal is limitted
			if (limiter.getEnvelope() > 0.99)
				guiQueue.push_back(GuiMessage(GuiMessage::action::blink, 1));
			else
				guiQueue.push_back(GuiMessage(GuiMessage::action::blink, 0));
			timeLastFlashGainSlider = now;
		}
		
		if (timeLastPrint + std::chrono::seconds(10) < now)
		{
			timeLastPrint = now;
			const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
			printf("Buffer queue %d Radio samples %d Audio Samples %d Passes %d Queued Audio Samples %d droppedframes %d underrun %d\n", receiveIQBuffer->size(), nosamples, noaudiosamples, passes, audioOutputBuffer->queued_samples() / 2, droppedFrames, audioOutputBuffer->get_underrun());
			printf("peak %f db gain %f db threshold %f ratio %f atack %f release %f\n", Agc.getPeak(), Agc.getGain(), Agc.getThreshold(), Agc.getRatio(), Agc.getAtack(),Agc.getRelease());
			printf("rms %f db %f envelope %f\n", get_if_level(), 20 * log10(get_if_level()), limiter.getEnvelope());
			printf("IQ Balance I %f Q %f\n", get_if_levelI(), get_if_levelQ());
			//std::cout << "SoapySDR samples " << gettxNoSamples() <<" sample rate " << get_rxsamplerate() << " ratio " << (double)audioSampleRate / get_rxsamplerate() << "\n";
			pr_time = 0;
			passes = 0;
			
			if (droppedFrames > thresholdDroppedFrames && audioOutputBuffer->get_underrun() == 0)
			{
				float resamplerate = Demodulator::adjust_resample_rate(-0.0005 * droppedFrames); //-0.002
				std::string str1 = std::to_string(resamplerate);
				Settings_file.save_string(default_radio, "resamplerate", str1);
				Settings_file.write_settings();
			}
			if ((audioOutputBuffer->get_underrun() > thresholdUnderrun) && droppedFrames == 0)
			{
				float resamplerate = Demodulator::adjust_resample_rate(0.0005 * audioOutputBuffer->get_underrun());
				std::string str1 = std::to_string(resamplerate);
				Settings_file.save_string(default_radio, "resamplerate", str1);
				Settings_file.write_settings();
			}
			audioOutputBuffer->clear_underrun();
			droppedFrames = 0;
		}
	}
	starttime1 = std::chrono::high_resolution_clock::now();
}

void AMDemodulator::process(const IQSampleVector&	samples_in, SampleVector& audio)
{
	IQSampleVector		filter1, filter2;
		
	// mix to correct frequency
	mix_down(samples_in, filter1);
	Resample(filter1, filter2);
	filter1.clear();
	lowPassAudioFilter(filter2, filter1);
	filter2.clear();
	calc_if_level(filter1);
	if (guirx.get_cw())
		pMDecoder->decode(filter1);
	for (auto col : filter1)
	{
		float v;

		ampmodem_demodulate(demodulatorHandle, (liquid_float_complex)col, &v);
		audio.push_back(v);
	}	
	filter1.clear();
	filter2.clear();
}
	
bool AMDemodulator::create_demodulator(int mode, double ifrate,  DataBuffer<IQSample> *source_buffer, AudioOutput *audioOutputBuffer)
{	
	if (sp_amdemod != nullptr)
		return false;
	sp_amdemod = make_shared<AMDemodulator>(mode, ifrate, source_buffer, audioOutputBuffer);
	sp_amdemod->amdemod_thread = std::thread(&AMDemodulator::operator(), sp_amdemod);
	return true;
}

void AMDemodulator::destroy_demodulator()
{
	auto startTime = std::chrono::high_resolution_clock::now();

	if (sp_amdemod == nullptr)
		return;
	sp_amdemod->stop_flag = true;
	sp_amdemod->amdemod_thread.join();
	sp_amdemod.reset();
	sp_amdemod = nullptr;
	auto now = std::chrono::high_resolution_clock::now();
	const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
	cout << "Stoptime AMDemodulator:" << timePassed.count() << endl;
}


