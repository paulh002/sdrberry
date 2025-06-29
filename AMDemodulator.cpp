#include <thread>
#include "AMDemodulator.h"
#include "PeakLevelDetector.h"
#include "Limiter.h"
#include "SharedQueue.h"
#include "gui_cal.h"
#include "vfo.h"
#include "gui_bar.h"
#include "gui_rx.h"
#include <complex>
#include <complex.h>

static shared_ptr<AMDemodulator> sp_amdemod;
std::mutex amdemod_mutex;

static std::chrono::high_resolution_clock::time_point starttime1 {};

extern double get_rxsamplerate();

AMDemodulator::AMDemodulator(int mode, double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audioOutputBuffer)
	: Demodulator(ifrate, source_buffer, audioOutputBuffer), receiverMode(mode)
{
	float					modulationIndex  = 0.03125f; 
	int						suppressed_carrier;
	liquid_ampmodem_type	am_mode;
	float bandwidth{2500}; // SSB
	float sample_ratio{0.0};
	std::vector<std::string> resamplerate_setting;

	// 1.05
	sample_ratio = ((float)audio_output->get_samplerate()) / ifrate;
	resamplerate_setting = Settings_file.get_array_string(default_radio, "resample_rate");
	if (resamplerate_setting.size() == 3)
	{
		if (std::stoi(resamplerate_setting.at(1)) == (int)round(ifrate / 1000.0))
		{
			int decimate = Settings_file.get_int(default_radio, "decimate",0);
			if (std::stoi(resamplerate_setting.at(2)) == decimate)
			{
				sscanf(resamplerate_setting.at(0).c_str(), "%f", &sample_ratio);
				printf("Sampleratio %f\n", sample_ratio);
			}
		}
	}
	Demodulator::set_resample_rate(sample_ratio); // down sample to pcmrate
	Demodulator::set_filter_offset(Settings_file.get_int("Radio", "filter_offset"));
	Demodulator::set_filter_type(Settings_file.get_int("Radio", "filter_type"));
	guirx.enable_filter_settings(true);
	bandwidth = gbar.get_filter_frequency(mode);
	switch (mode)
	{
	case mode_usb:
		if (bandwidth == 0)
			bandwidth = 2500; // SSB
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_USB;
		printf("mode LIQUID_AMPMODEM_USB carrier %d\n", suppressed_carrier);		
		break;
	case mode_cw:
		if (bandwidth == 0)
			bandwidth = 500; // CW
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_LSB;
		printf("mode CW LIQUID_AMPMODEM_LSB carrier %d\n", suppressed_carrier);
		break;
	case mode_lsb:
		if (bandwidth == 0)
			bandwidth = 2500; // SSB
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_LSB;
		printf("mode LIQUID_AMPMODEM_LSB carrier %d\n", suppressed_carrier);		
		break;
	case mode_am:
		if (bandwidth == 0)
			bandwidth = 5000; // SSB
		suppressed_carrier = 0;
		am_mode = LIQUID_AMPMODEM_DSB;
		printf("mode LIQUID_AMPMODEM_DSB carrier %d\n", suppressed_carrier);		
		break;
	case mode_dsb:
		if (bandwidth == 0)
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
	gbar.set_filter_slider(bandwidth);
	Demodulator::setLowPassAudioFilter(audioSampleRate, bandwidth);
	demodulatorHandle = ampmodem_create(modulationIndex, am_mode, suppressed_carrier);
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
	auto timeLastPrintIQ = std::chrono::high_resolution_clock::now();
	auto DownSampleT = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point  now, start1, start2;
	std::chrono::microseconds phasetime;
	long SampleT;
	
	int lowPassAudioFilterCutOffFrequency {-1}, droppedFrames {0};
	SampleVector            audioSamples, audioFrames;
	unique_lock<mutex>		lock_am(amdemod_mutex);
	IQSampleVector			dc_iqsamples, iqsamples;
	long long pr_time{0};
	long noRfSamples{0}, noAfSamples{0};
	int vsize, passes{0};
	int phase = 0, pcount = 0;
	float resamplerate = 0.0, sample_ratio= 0.0;

	int limiterAtack = Settings_file.get_int(Limiter::getsetting(), "limiterAtack", 10);
	int limiterDecay = Settings_file.get_int(Limiter::getsetting(), "limiterDecay", 500);
	Limiter limiter(limiterAtack, limiterDecay, ifSampleRate);
	int thresholdDroppedFrames = Settings_file.get_int(default_radio, "thresholdDroppedFrames", 2);
	int thresholdUnderrun = Settings_file.get_int(default_radio, "thresholdUnderrun", 1);

	resamplerate = sample_ratio = ((float)audio_output->get_samplerate()) / ifrate;
	pNoisesp = make_unique<SpectralNoiseReduction>(audioSampleRate, tuple<float,float>(0, 2500));
	//pLMS = make_unique<LMSNoisereducer>(); switched off memory leak in library
	pXanr = make_unique<Xanr>();
	receiveIQBuffer->clear();
	audioOutputBuffer->CopyUnderrunSamples(true);
	audioOutputBuffer->clear_underrun();
	lowPassAudioFilterCutOffFrequency = get_lowPassAudioFilterCutOffFrequency();
	while (!stop_flag.load())
	{
		start1 = std::chrono::high_resolution_clock::now();
		if (vfo.tune_flag.load())
		{
			vfo.tune_flag = false;
			tune_offset(vfo.get_vfo_offset(true));
		}

		if (lowPassAudioFilterCutOffFrequency != get_lowPassAudioFilterCutOffFrequency())
		{
			lowPassAudioFilterCutOffFrequency = get_lowPassAudioFilterCutOffFrequency();
			printf("set filter %d\n", lowPassAudioFilterCutOffFrequency);
			setLowPassAudioFilter(audioSampleRate, lowPassAudioFilterCutOffFrequency);
		}

		iqsamples = receiveIQBuffer->pull();
		if (iqsamples.empty())
			continue;
		dc_filter(iqsamples);
		int nosamples = iqsamples.size();
		noRfSamples += nosamples;
		passes++;
		calc_if_level(iqsamples);
		gain_phasecorrection(iqsamples, gbar.get_if());
		limiter.Process(iqsamples);
		perform_fft(iqsamples);
		process(iqsamples, audioSamples);
		set_signal_strength();
		// Set nominal audio volume.
		audioOutputBuffer->adjust_gain(audioSamples);
		int noaudiosamples = audioSamples.size();
		noAfSamples += noaudiosamples;
		for (auto &col : audioSamples)
		{
			// split the stream in blocks of samples of the size framesize
			audioFrames.insert(audioFrames.end(), col);
			if (audioFrames.size() == audioOutputBuffer->get_framesize())
			{
				if ((audioOutputBuffer->queued_samples() / 2) < get_audioBufferSize())
				{
					SampleVector		audioStereoSamples, audioNoiseSamples;

					switch (get_noise())
					{
					case 1:
						pXanr->Process(audioFrames, audioNoiseSamples);
						mono_to_left_right(audioNoiseSamples, audioStereoSamples);
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
					audioFrames.reserve(audioOutputBuffer->get_framesize());
				}
				else
				{
					droppedFrames++;
					audioFrames.clear();
					audioFrames.reserve(audioOutputBuffer->get_framesize());
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

		FlashGainSlider(limiter.getEnvelope());
		correlationMeasurement = get_if_CorrelationNorm();
		errorMeasurement = get_if_levelI() * 10000.0 - get_if_levelQ() * 10000.0;
		
/*		if (timeLastPrintIQ + std::chrono::seconds(1) < now)
		{
			timeLastPrintIQ = now;
			double error = get_if_levelI() * 10000.0 - get_if_levelQ() * 10000.0;
			float phase = (float)gcal.getRxPhase();
			float gain = (float)gcal.getRxGain();
			printf("IQ Balance I %f Q %f correlation %f error %f gain %f phase %f\n", get_if_levelI() * 10000.0, get_if_levelQ() * 10000.0, get_if_CorrelationNorm(), error, gain, phase);
		}
*/

		if (timeLastPrint + std::chrono::seconds(10) < now)
		{
			timeLastPrint = now;
			const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
			printf("Buffer queue %d Radio samples %d Audio Samples %d Passes %d Queued Audio Samples %d droppedframes %d underrun %d\n", receiveIQBuffer->size(), nosamples, noaudiosamples, passes, audioOutputBuffer->queued_samples() / 2, droppedFrames, audioOutputBuffer->get_underrun());
			SquelchPrint();
			printf("rms %f db %f envelope %f suppression %f db\n", get_if_level(), 20 * log10(get_if_level()), limiter.getEnvelope(), getSuppression());
			printf("RF samples %ld Af samples %ld ratio %f \n", noRfSamples, noAfSamples, (float)noAfSamples / (float)noRfSamples);
			//printf("Process time %lld Samples %d\n", pr_time, nosamples);
			noRfSamples = noAfSamples = 0L;
			//printf("IQ Balance I %f Q %f Phase %f\n", get_if_levelI() * 10000.0, get_if_levelQ() * 10000.0, get_if_Phase());
			//std::cout << " sample rate " << get_rxsamplerate() << "\n";
			pr_time = 0;
			passes = 0;
			
			if (droppedFrames > thresholdDroppedFrames && audioOutputBuffer->get_underrun() == 0)
			{
				float resamplerate = Demodulator::adjust_resample_rate(-0.0005 * droppedFrames); //-0.002
				std::string str1 = std::to_string(resamplerate);
				printf("dropframes resamplerate %s %f\n", str1.c_str(), -0.0005 * droppedFrames);
			}
			if ((audioOutputBuffer->get_underrun() > thresholdUnderrun) && droppedFrames == 0)
			{
				float resamplerate = Demodulator::adjust_resample_rate(0.0005 * audioOutputBuffer->get_underrun());
				std::string str1 = std::to_string(resamplerate);
				printf("underrun resamplerate %s %f\n", str1.c_str(), -0.0005 * audioOutputBuffer->get_underrun());
			}
			
			audioOutputBuffer->clear_underrun();
			droppedFrames = 0;	
		}
	}
	audioOutputBuffer->CopyUnderrunSamples(false);
	starttime1 = std::chrono::high_resolution_clock::now();
}

void AMDemodulator::process(IQSampleVector&	samples_in, SampleVector& audio)
{
	IQSampleVector filter1,filter2, filter3;
	
	// mix to correct frequency
	mix_down(samples_in);
	Resample(samples_in, filter1);
	SquelchProcess(filter1);
	if (get_noise())
	{
		lowPassAudioFilter(filter1);
		calc_signal_level(filter1);
		NoiseFilterProcess(filter1, filter2);
		filter1 = filter2;
	}
	else
	{
		lowPassAudioFilter(filter1);
		calc_signal_level(filter1);
	}
	if (guirx.get_cw())
		pMDecoder->decode(filter1);
	for (auto col : filter1)
	{
		float v;

		ampmodem_demodulate(demodulatorHandle, (liquid_float_complex)col, &v);
		if (Squelch())
			audio.push_back(0.0);
		else
			audio.push_back(v);
	}
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

void AMDemodulator::setLowPassAudioFilterCutOffFrequency(int bandwidth)
{
	if (sp_amdemod != nullptr)
		sp_amdemod->Demodulator::setLowPassAudioFilterCutOffFrequency(bandwidth);
}
