#include "WSPRDemodulator.h"
#include "FreeDVTab.h"
#include "Spectrum.h"
#include "date.h"
#include "gui_ft8bar.h"
#include "gui_bar.h"
#include "gui_rx.h"
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
#include "SignalStrength.h"

static shared_ptr<WSPRDemodulator> sp_wsprdemod;
std::mutex wsprdemod_mutex;
static std::chrono::high_resolution_clock::time_point starttime1{};

bool WSPRDemodulator::create_demodulator(double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output, int mode)
{
	if (sp_wsprdemod != nullptr)
		return false;
	sp_wsprdemod = make_shared<WSPRDemodulator>(ifrate, source_buffer, audio_output, mode);
	sp_wsprdemod->amdemod_thread = std::thread(&WSPRDemodulator::operator(), sp_wsprdemod);
	return true;
}

void WSPRDemodulator::destroy_demodulator()
{
	auto startTime = std::chrono::high_resolution_clock::now();

	if (sp_wsprdemod == nullptr)
		return;
	sp_wsprdemod->stop_flag = true;
	sp_wsprdemod->amdemod_thread.join();
	sp_wsprdemod.reset();

	auto now = std::chrono::high_resolution_clock::now();
	const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
	cout << "Stoptime WSPRDemodulator:" << timePassed.count() << endl;
}

WSPRDemodulator::WSPRDemodulator(double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output, int mode)
	: wsjtxmode{mode}, Demodulator(ifrate, source_buffer, audio_output)
{
	float mod_index = 0.03125f;
	int suppressed_carrier;
	liquid_ampmodem_type am_mode;

	Demodulator::set_resample_rate(wspr_rate / ifrate); // down sample to wspr_rate
	suppressed_carrier = 1;
	am_mode = LIQUID_AMPMODEM_USB;
	printf("mode LIQUID_AMPMODEM_USB carrier %d\n", suppressed_carrier);

	const auto startTime = std::chrono::high_resolution_clock::now();

	bandwidth = Settings_file.get_int("wspr", "bandwidth", 4000);
	gbar.set_filter_dropdown(bandwidth);
	Demodulator::set_filter_offset(0);
	Demodulator::set_filter_type(0);
	Demodulator::set_filter_order(6);
	guirx.enable_filter_settings(false);
	Demodulator::setLowPassAudioFilter(wspr_rate, bandwidth);
	demod = ampmodem_create(mod_index, am_mode, suppressed_carrier);
	auto now = std::chrono::high_resolution_clock::now();
	const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
	cout << "starttime :" << timePassed.count() << endl;
}

WSPRDemodulator::~WSPRDemodulator()
{
	printf("WSPR destructor called \n");
	if (demod != nullptr)
	{
		ampmodem_destroy(demod);
		demod = nullptr;
	}
}

void WSPRDemodulator::operator()()
{
	auto startTime = std::chrono::high_resolution_clock::now();
	auto timeLastPrint = std::chrono::high_resolution_clock::now();
	auto today = date::floor<date::days>(startTime);

	int ifilter{-1}, rcount{0}, dropped_frames{0};
	long span;
	int cycletime_duration_tensseconds{150}, starttime_delay{0};
	int capture_time_duration_ms{150};

	unique_lock<mutex> lock_am(wsprdemod_mutex);
	IQSampleVector iqsamples, iqsamples_out;
	bool capture{false};

	WSPRProcessor::create_modulator(wspr_processor, wsjtxmode);
	cycletime_duration_tensseconds = 1200; // only even
	capture_time_duration_ms = 110600;
	starttime_delay = 1000;

	// Fft_calc.plan_fft(nfft_samples);
	receiveIQBuffer->clear();
	while (!stop_flag.load())
	{
		span = vfo.get_span();
		if (vfo.tune_flag.load())
		{
			vfo.tune_flag = false;
			tune_offset(vfo.get_vfo_offset());
		}

		IQSampleVector iqsamples = receiveIQBuffer->pull();
		if (iqsamples.empty())
		{
			usleep(500);
			continue;
		}
		gain_phasecorrection(iqsamples, gbar.get_if());
		perform_fft(iqsamples);
		process(iqsamples, iqsamples_out);
		auto millisecondsUTC = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		if (((millisecondsUTC / 100) % cycletime_duration_tensseconds == 0) && !capture)
		{
			capture = true;
			std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
			std::cout << "start cycle " << date::make_time(now - today) << "\n";
			iqsamples_out.clear();
			// drop all captured data
			startTime = std::chrono::high_resolution_clock::now();
		}
		auto now = std::chrono::high_resolution_clock::now();
		auto timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
		if ((timePassed.count() > capture_time_duration_ms) && (capture))
		{
			capture = false;
			if (iqsamples_out.size() < capture_time_duration_ms * wspr_rate / 1000)
				iqsamples_out.resize(capture_time_duration_ms * wspr_rate / 1000);
			printf("capatured %ld samples \n", iqsamples_out.size());
			wspr_processor->AddIQSample(iqsamples_out);
			iqsamples_out.clear();
		}
		iqsamples.clear();
	}
	WSPRProcessor::destroy_modulator(wspr_processor);
	starttime1 = std::chrono::high_resolution_clock::now();
}

void WSPRDemodulator::process(IQSampleVector &samples_in, IQSampleVector &samples_out)
{
	IQSampleVector filter;

	// mix to correct frequency
	mix_down(samples_in);
	calc_iq_signalstrength(samples_in);
	Resample(samples_in, filter);
	lowPassAudioFilter(filter, samples_out);
	guift8bar.Process(samples_out);
}

void WSPRDemodulator::setLowPassAudioFilterCutOffFrequency(int bandwidth)
{
	if (sp_wsprdemod != nullptr)
		sp_wsprdemod->Demodulator::setLowPassAudioFilterCutOffFrequency(bandwidth);
}
