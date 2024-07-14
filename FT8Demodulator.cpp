#include "FT8Demodulator.h"
#include "FreeDVTab.h"
#include "Spectrum.h"
#include "date.h"
#include "gui_ft8bar.h"
#include "gui_bar.h"
#include "sdrberry.h"
#include <assert.h>
#include <chrono>
#include <ctime>
#include <map>
#include <mutex>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <vector>

static shared_ptr<FT8Demodulator> sp_ft8demod;
std::mutex ft8demod_mutex;
static std::chrono::high_resolution_clock::time_point starttime1{};

bool FT8Demodulator::create_demodulator(double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output, int mode)
{
	if (sp_ft8demod != nullptr)
		return false;
	sp_ft8demod = make_shared<FT8Demodulator>(ifrate, source_buffer, audio_output, mode);
	sp_ft8demod->amdemod_thread = std::thread(&FT8Demodulator::operator(), sp_ft8demod);
	return true;
}

void FT8Demodulator::destroy_demodulator()
{
	auto startTime = std::chrono::high_resolution_clock::now();

	if (sp_ft8demod == nullptr)
		return;
	sp_ft8demod->stop_flag = true;
	sp_ft8demod->amdemod_thread.join();
	sp_ft8demod.reset();

	auto now = std::chrono::high_resolution_clock::now();
	const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
	cout << "Stoptime FT8Demodulator:" << timePassed.count() << endl;
}

FT8Demodulator::FT8Demodulator(double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output, int mode)
	: wsjtxmode{mode}, Demodulator(ifrate, source_buffer, audio_output)
{
	float mod_index = 0.03125f;
	int suppressed_carrier;
	liquid_ampmodem_type am_mode;

	Demodulator::set_resample_rate(ft8_rate / ifrate); // down sample to ft8_rate
	suppressed_carrier = 1;
	am_mode = LIQUID_AMPMODEM_USB;
	printf("mode LIQUID_AMPMODEM_USB carrier %d\n", suppressed_carrier);

	const auto startTime = std::chrono::high_resolution_clock::now();

	m_bandwidth = Settings_file.get_int("ft8", "bandwidth", 4000);
	gbar.set_filter_slider(m_bandwidth);
	Demodulator::setLowPassAudioFilter(ft8_rate, m_bandwidth);
	m_demod = ampmodem_create(mod_index, am_mode, suppressed_carrier);
	auto now = std::chrono::high_resolution_clock::now();
	const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
	cout << "starttime :" << timePassed.count() << endl;
}

FT8Demodulator::~FT8Demodulator()
{
	printf("FT8 destructor called \n");
	if (m_demod != nullptr)
	{
		ampmodem_destroy(m_demod);
		m_demod = nullptr;
	}
}

void FT8Demodulator::operator()()
{
	auto startTime = std::chrono::high_resolution_clock::now();
	auto timeLastPrint = std::chrono::high_resolution_clock::now();
	auto today = date::floor<date::days>(startTime);

	int ifilter{-1}, rcount{0}, dropped_frames{0};
	long span;
	int cycletime_duration_tensseconds{150}, starttime_delay{0};
	int capture_time_duration_ms{150};
	SampleVector audiosamples, audioframes;
	unique_lock<mutex> lock_am(ft8demod_mutex);
	IQSampleVector iqsamples;
	bool capture{false};

	FT8Processor::create_modulator(ft8processor, wsjtxmode);
	switch (wsjtxmode)
	{
	case mode_ft8:
		cycletime_duration_tensseconds = 150;
		capture_time_duration_ms = 15000;
		starttime_delay = 0;
		break;
	case mode_ft4:
		cycletime_duration_tensseconds = 75;
		capture_time_duration_ms = 7500;
		starttime_delay = 0;
		break;
	case mode_wspr:
		cycletime_duration_tensseconds = 1200; // only even
		capture_time_duration_ms = 110600;
		starttime_delay = 1000;
		break;
	}

	//Fft_calc.plan_fft(nfft_samples);
	receiveIQBuffer->clear();
	while (!stop_flag.load())
	{
		span = vfo.get_span();
		if (vfo.tune_flag.load())
		{
			vfo.tune_flag = false;
			tune_offset(vfo.get_vfo_offset());
		}

		if (ifilter != get_lowPassAudioFilterCutOffFrequency())
		{
			ifilter = get_lowPassAudioFilterCutOffFrequency();
			printf("set filter %d\n", ifilter);
			setLowPassAudioFilter(ft8_rate, ifilter);
		}

		IQSampleVector iqsamples = receiveIQBuffer->pull();
		if (iqsamples.empty())
		{
			usleep(500);
			continue;
		}
		calc_if_level(iqsamples);
		gain_phasecorrection(iqsamples, gbar.get_if());
		perform_fft(iqsamples);
		set_signal_strength();
		process(iqsamples, audiosamples);
		auto millisecondsUTC = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		if (((millisecondsUTC / 100) % cycletime_duration_tensseconds == 0) && !capture)
		{
			capture = true;
			std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
			std::cout << "start cycle " << date::make_time(now - today) << "\n";
			audiosamples.clear();
			// drop all captured data
			startTime = std::chrono::high_resolution_clock::now();
		}
		auto now = std::chrono::high_resolution_clock::now();
		auto timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
		if ((timePassed.count() > capture_time_duration_ms) && (capture))
		{
			capture = false;
			if (audiosamples.size() < capture_time_duration_ms * ft8_rate / 1000)
				audiosamples.resize(capture_time_duration_ms * ft8_rate / 1000);
			printf("cpatured %ld samples \n", audiosamples.size());
			ft8processor->AddaudioSample(audiosamples);
			audiosamples.clear();
		}
		iqsamples.clear();
	}
	FT8Processor::destroy_modulator(ft8processor);
	starttime1 = std::chrono::high_resolution_clock::now();
}

void FT8Demodulator::process(const IQSampleVector &samples_in, SampleVector &audio)
{
	IQSampleVector filter1, filter2;
	SampleVector audio_mono;

	// mix to correct frequency
	mix_down(samples_in, filter1);
	Resample(filter1, filter2);
	filter1.clear();
	lowPassAudioFilter(filter2, filter1);
	calc_signal_level(filter1);
	guift8bar.Process(filter1);
	for (auto col : filter1)
	{
		float v;

		ampmodem_demodulate(m_demod, (liquid_float_complex)col, &v);
		audio.push_back(v);
	}
}