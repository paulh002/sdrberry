#include "FMDemodulator.h"
#include "PeakLevelDetector.h"
#include "Limiter.h"
#include "gui_agc.h"
#include "gui_bar.h"
#include "Agc_class.h"
#include "sdrberry.h"
#include <thread>

FMDemodulator::FMDemodulator(double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output)
	: Demodulator(ifrate, source_buffer, audio_output)
{
	gbar.set_filter_slider(3500);
	Demodulator::setLowPassAudioFilter(audioSampleRate, 3500);
	int lowPassAudioFilterCutOffFrequency = get_lowPassAudioFilterCutOffFrequency();
	Demodulator::set_resample_rate(audio_output->get_samplerate() / ifrate); // down sample to pcmrate
	Demodulator::setLowPassAudioFilter(audio_output->get_samplerate(), lowPassAudioFilterCutOffFrequency);
	demodFM = freqdem_create(0.5);
}
	
void FMDemodulator::operator()()
{	
	const auto startTime = std::chrono::high_resolution_clock::now();
	auto timeLastPrint = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point now, start1, start2;

	int ifilter{-1}, lowPassAudioFilterCutOffFrequency{-1};
	int droppedFrames{0};
	long span;
	SampleVector audiosamples, audioframes;
	long long pr_time{0};
	int vsize, passes{0};

	int thresholdDroppedFrames = Settings_file.get_int(default_radio, "thresholdDroppedFrames", 2);
	int thresholdUnderrun = Settings_file.get_int(default_radio, "thresholdUnderrun", 1);
	int limiterAtack = Settings_file.get_int(Limiter::getsetting(), "limiterAtack", 10);
	int limiterDecay = Settings_file.get_int(Limiter::getsetting(), "limiterDecay", 500);
	Limiter limiter(limiterAtack, limiterDecay, ifSampleRate);
	AudioProcessor Agc;

	Agc.prepareToPlay(audioOutputBuffer->get_samplerate());
	Agc.setThresholdDB(gagc.get_threshold());
	Agc.setRatio(10);
	receiveIQBuffer->clear();
	audioOutputBuffer->CopyUnderrunSamples(true);
	audioOutputBuffer->clear_underrun();
	while (!stop_flag.load())
	{
		span = vfo.get_span();
		if (vfo.tune_flag.load())
		{
			vfo.tune_flag = false;
			tune_offset(vfo.get_vfo_offset());
		}
		
		if (lowPassAudioFilterCutOffFrequency != get_lowPassAudioFilterCutOffFrequency())
		{
			lowPassAudioFilterCutOffFrequency = get_lowPassAudioFilterCutOffFrequency();
			printf("set filter %d\n", lowPassAudioFilterCutOffFrequency);
			setLowPassAudioFilter(audioSampleRate, lowPassAudioFilterCutOffFrequency);
		}
		IQSampleVector iqsamples;
		IQSampleVector dc_iqsamples = receiveIQBuffer->pull();
		if (dc_iqsamples.empty())
		{
			usleep(500);
			continue;
		}
		dc_filter(dc_iqsamples, iqsamples);
		int nosamples = iqsamples.size();
		calc_if_level(iqsamples);
		gain_phasecorrection(iqsamples, gbar.get_if());
		limiter.Process(iqsamples);
		perform_fft(iqsamples);
		set_signal_strength();
		process(iqsamples, audiosamples);
		if (gagc.get_agc_mode())
		{
			Agc.setRelease(gagc.get_release());
			Agc.setRatio(gagc.get_ratio());
			Agc.setAtack(gagc.get_atack());
			Agc.setThresholdDB(gagc.get_threshold());
			Agc.processBlock(audiosamples);
		}
		// Set nominal audio volume.
		audio_output->adjust_gain(audiosamples);
		int noaudiosamples = audiosamples.size();
		for (auto &col : audiosamples)
		{
			// split the stream in blocks of samples of the size framesize
			audioframes.insert(audioframes.end(), col);
			if (audioframes.size() == audio_output->get_framesize())
			{
				if ((audioOutputBuffer->queued_samples() / 2) < get_audioBufferSize())
				{

					SampleVector audio_stereo;

					mono_to_left_right(audioframes, audio_stereo);
					audio_output->write(audio_stereo);
					audioframes.clear();
				}
				else
				{
					droppedFrames++;
					audioframes.clear();
				}
			}
		}
		iqsamples.clear();
		audiosamples.clear();

		now = std::chrono::high_resolution_clock::now();
		auto process_time1 = std::chrono::duration_cast<std::chrono::microseconds>(now - start1);
		if (pr_time < process_time1.count())
			pr_time = process_time1.count();
		FlashGainSlider(limiter.getEnvelope());
		if (timeLastPrint + std::chrono::seconds(10) < now)
		{
			timeLastPrint = now;
			const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
			printf("Buffer queue %d Radio samples %d Audio Samples %d Passes %d Queued Audio Samples %d droppedframes %d underrun %d\n", receiveIQBuffer->size(), nosamples, noaudiosamples, passes, audioOutputBuffer->queued_samples() / 2, droppedFrames, audioOutputBuffer->get_underrun());
			printf("peak %f db gain %f db threshold %f ratio %f atack %f release %f\n", Agc.getPeak(), Agc.getGain(), Agc.getThreshold(), Agc.getRatio(), Agc.getAtack(), Agc.getRelease());
			printf("rms %f db %f envelope %f\n", get_if_level(), 20 * log10(get_if_level()), limiter.getEnvelope());
			//printf("IQ Balance I %f Q %f Phase %f\n", get_if_levelI() * 10000.0, get_if_levelQ() * 10000.0, get_if_Phase());
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
	audioOutputBuffer->CopyUnderrunSamples(false);
}

void FMDemodulator::process(const IQSampleVector&	samples_in, SampleVector& audio)
{
	IQSampleVector		filter1, filter2;
		
	// mix to correct frequency
	mix_down(samples_in, filter1);
	Resample(filter1, filter2);
	filter1.clear();
	lowPassAudioFilter(filter2, filter1);
	filter2.clear();
	calc_signal_level(filter1);
	for (auto col : filter1)
	{
		float v;
		
		freqdem_demodulate(demodFM, col, &v);
		audio.push_back(v);
	}	
	filter1.clear();
}

FMDemodulator::~FMDemodulator()
{
	if (demodFM != nullptr)
	{
		freqdem_destroy(demodFM);
		demodFM = nullptr;		
	}
}

static	std::thread				fmdemod_thread;
shared_ptr<FMDemodulator>		sp_fmdemod;

bool FMDemodulator::create_demodulator(double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output)
{	
	if (sp_fmdemod != nullptr)
		return false;
	sp_fmdemod = make_shared<FMDemodulator>(ifrate, source_buffer, audio_output);
	fmdemod_thread = std::thread(&FMDemodulator::operator(), sp_fmdemod);
	return true;
}

void FMDemodulator::destroy_demodulator()
{
	if (sp_fmdemod == nullptr)
		return;
	sp_fmdemod->stop_flag = true; 
	fmdemod_thread.join();
	sp_fmdemod.reset();
}

void FMDemodulator::setLowPassAudioFilterCutOffFrequency(int bandwidth)
{
	if (sp_fmdemod != nullptr)
		sp_fmdemod->Demodulator::setLowPassAudioFilterCutOffFrequency(bandwidth);
}