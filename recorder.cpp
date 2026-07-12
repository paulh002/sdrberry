#include "recorder.h"
#include "FastFourier.h"
#include "PeakLevelDetector.h"
#include "Spectrum.h"
#include "gui_speech.h"
#include <complex.h>
#include <liquid/liquid.h>
#include <math.h>
#include <stdio.h>
#include <string>
#include "Catinterface.h"
#include "CatTcpServer.h"
#include "gui_bar.h"

static std::shared_ptr<recorder> sp_recorder;

#define dB2mag(x) pow(10.0, (x) / 20.0)

bool recorder::create_recorder(AudioOutput *audio_output, AudioInput *audio_input)
{
	if (sp_recorder != nullptr)
		return false;
	gbar.enable_digital_mode(true);
	sp_recorder = std::make_shared<recorder>(audio_output, audio_input);
	sp_recorder->recorder_thread = std::thread(&recorder::operator(), sp_recorder);
	return true;
}

void recorder::destroy_recorder()
{
	if (sp_recorder == nullptr)
		return;
	sp_recorder->stop_flag = true;
	sp_recorder->recorder_thread.join();
	sp_recorder.reset();
	gbar.enable_digital_mode(false);
}

recorder::recorder(AudioOutput *audio_ouput, AudioInput *audio_input)
	: Demodulator(audio_ouput, audio_input)
{
}

recorder::~recorder()
{
}

void recorder::operator()()
{
	const auto startTime = std::chrono::high_resolution_clock::now();
	auto timeLastPrint = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point now, start1;

	std::span<Sample> audiosamples;
	AudioProcessor Speech;
	WavWriter WavWriterOut;

	catinterface.Pause_Cat(true);
	cattcpserver.Pause_Cat(true);
	
	Speech.prepareToPlay(audio_output->get_samplerate());
	Speech.setThresholdDB(gspeech.get_threshold());
	Speech.setRatio(gspeech.get_ratio());
	audioInputBuffer->clear();

	WavWriterOut.create("output.wav", audio_output->get_samplerate(), 1);
	start1 = std::chrono::high_resolution_clock::now();
	while (!stop_flag.load())
	{
		audiosamples = audioInputBuffer->read();
		if (gspeech.get_speech_mode())
		{
			Speech.setRelease(gspeech.get_release());
			Speech.setRatio(gspeech.get_ratio());
			Speech.setAtack(gspeech.get_atack());
			Speech.setThresholdDB(gspeech.get_threshold());
			Speech.processBlock(audiosamples);
		}
		executeBandpassFilter(audiosamples);
		WavWriterOut.addSamples(audiosamples);
		now = std::chrono::high_resolution_clock::now();
		auto process_time1 = std::chrono::duration_cast<std::chrono::seconds>(now - start1);
		//if (process_time1.count() > 120)
		//	break;
		if (timeLastPrint + std::chrono::seconds(10) < now)
		{
			timeLastPrint = now;
			const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
			printf("peak %f db gain %f db threshold %f ratio %f atack %f release %f\n", Speech.getPeak(), Speech.getGain(), Speech.getThreshold(), Speech.getRatio(), Speech.getAtack(), Speech.getRelease());
		}
	}
	WavWriterOut.close();
	catinterface.Pause_Cat(false);
	cattcpserver.Pause_Cat(false);
	printf("exit recorder\n");
}