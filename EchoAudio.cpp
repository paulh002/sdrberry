#include <stdio.h>
#include <math.h>
#include <complex.h>

#include "liquid.h"
#include "EchoAudio.h"
#include "Waterfall.h"
#include "Audiodefs.h"
#include "gui_speech.h"
#include "PeakLevelDetector.h"

static shared_ptr<EchoAudio> sp_echo;

#define dB2mag(x) pow(10.0, (x) / 20.0)

bool EchoAudio::create_modulator(int pcmrate, AudioOutput *audio_ouput, AudioInput *audio_input)
{
	if (sp_echo != nullptr)
		return false;
	sp_echo = make_shared<EchoAudio>(pcmrate, audio_ouput, audio_input);
	sp_echo->echo_thread = std::thread(&EchoAudio::operator(), sp_echo);
	return true;
}

void EchoAudio::destroy_modulator()
{
	if (sp_echo == nullptr)
		return;
	sp_echo->stop_flag = true;
	sp_echo->echo_thread.join();
	sp_echo.reset();
}

EchoAudio::EchoAudio(int pcmrate, AudioOutput *audio_ouput, AudioInput *audio_input)
	: Demodulator(pcmrate, audio_ouput, audio_input)
{
	
}

EchoAudio::~EchoAudio()
{

}

void EchoAudio::process(const IQSampleVector &samples_in, SampleVector &audio)
{
	
}

void EchoAudio::operator()()
{
	const auto startTime = std::chrono::high_resolution_clock::now();
	auto timeLastPrint = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point now, start1;
	
	SampleVector audiosamples, audioframes,filter;
	IQSampleVector buf_mod, buf_filter;
	AudioProcessor Speech;

	liquid_ampmodem_type am_mode{LIQUID_AMPMODEM_USB};

	Speech.prepareToPlay(audio_output->get_samplerate());
	Speech.setThresholdDB(gspeech.get_threshold());
	Speech.setRatio(gspeech.get_ratio());
	Fft_calc.plan_fft(nfft_samples);
	audioInputBuffer->clear();
	if (gspeech.get_speech_mode())
		audioInputBuffer->set_gain(20);

	float mod_index = 0.99f; // modulation index (bandwidth)
	ampmodem modAM = ampmodem_create(mod_index, am_mode, 1);
	ampmodem demod = ampmodem_create(mod_index, am_mode, 1);

	setBandPassFilter(2700.0f, 2000.0f, 500.0f, 150.0f);

	while (!stop_flag.load())
	{
		if (!audioInputBuffer->read(audiosamples))
			continue;

		if (gspeech.get_speech_mode())
		{
			audioInputBuffer->set_gain(20);
			Speech.setRelease(gspeech.get_release());
			Speech.setRatio(gspeech.get_ratio());
			Speech.setAtack(gspeech.get_atack());
			Speech.setThresholdDB(gspeech.get_threshold());
			Speech.processBlock(audiosamples);
		}
		else
			audioInputBuffer->set_gain(0);

		for (auto &col : audiosamples)
		{
			complex<float> f;
			ampmodem_modulate(modAM, col, &f);
			//printf("%f;%f;%f \n", col, f.real(), f.imag());
			buf_mod.push_back(f);
		}
		audiosamples.clear();
		executeBandpassFilter(buf_mod, buf_filter);
		buf_mod.clear();
		for (auto col : buf_filter)
		{
			float v;
			ampmodem_demodulate(demod, (liquid_float_complex)col, &v);
			audiosamples.push_back(v);
		}
		buf_filter.clear();

		calc_af_level(audiosamples);
		Fft_calc.set_signal_strength(get_af_level());
		audio_output->adjust_gain(audiosamples);
		for (auto &col : audiosamples)
		{
			// split the stream in blocks of samples of the size framesize
			audioframes.insert(audioframes.end(), col);
			if (audioframes.size() == audio_output->get_framesize())
			{
				if ((audio_output->queued_samples() / 2) < 4096)
				{
					SampleVector audio_stereo;

					mono_to_left_right(audioframes, audio_stereo);
					audio_output->write(audio_stereo);
					audioframes.clear();
				}
				else
					audioframes.clear();
			}
		}
		audiosamples.clear();
		now = std::chrono::high_resolution_clock::now();
		auto process_time1 = std::chrono::duration_cast<std::chrono::microseconds>(now - start1);
		if (timeLastPrint + std::chrono::seconds(10) < now)
		{
			timeLastPrint = now;
			const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
			printf("peak %f db gain %f db threshold %f ratio %f atack %f release %f\n", Speech.getPeak(), Speech.getGain(), Speech.getThreshold(), Speech.getRatio(), Speech.getAtack(), Speech.getRelease());
			printf("rms %f \n", get_if_level());
		}
	}

	ampmodem_destroy(modAM);
	ampmodem_destroy(demod);
	printf("exit EchoAudio\n");
}