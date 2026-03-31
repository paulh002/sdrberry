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
	SampleVector audiosamples, audioframes;
	IQSampleVector samples_wav;
	SampleVector audio_wav;

	unique_lock<mutex>
		lock_am(wsprdemod_mutex);
	IQSampleVector iqsamples, iqsamples_out;
	bool capture{false};

	WSPRProcessor::create_modulator(wspr_processor, wsjtxmode);
	cycletime_duration_tensseconds = 1200; // only even
	capture_time_duration_ms = 110600;
	starttime_delay = 1000;

	// Fft_calc.plan_fft(nfft_samples);
	receiveIQBuffer->clear();
	int simulate = Settings_file.get_int("wsjtx", "simulate", 0);
	if (simulate)
	{
		char infile[] = "./150426_0918.wav";
		
		readwavfile(infile, 2, samples_wav, audio_wav);
		printf("wav file iq %ld real %ld\n", samples_wav.size(), audio_wav.size());
	}
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
			if (audiosamples.size() < capture_time_duration_ms * wspr_rate / 1000)
				audiosamples.resize(capture_time_duration_ms * wspr_rate / 1000);
			printf("capatured %ld samples \n", audiosamples.size());
			if (simulate)
				wspr_processor->AddAudioSample(audio_wav);
			else
				wspr_processor->AddAudioSample(audiosamples);
			audiosamples.clear();
		}
		iqsamples.clear();
	}
	WSPRProcessor::destroy_modulator(wspr_processor);
	starttime1 = std::chrono::high_resolution_clock::now();
}

void WSPRDemodulator::process(IQSampleVector &samples_in, SampleVector &audio)
{
	IQSampleVector filter1, filter2;

	// mix to correct frequency
	mix_down(samples_in);
	calc_iq_signalstrength(samples_in);
	Resample(samples_in, filter2);
	lowPassAudioFilter(filter2, filter1);
	guift8bar.Process(filter1);
	for (auto col : filter1)
	{
		float v;

		ampmodem_demodulate(demod, (liquid_float_complex)col, &v);
		audio.push_back(v);
	}
}

void WSPRDemodulator::setLowPassAudioFilterCutOffFrequency(int bandwidth)
{
	if (sp_wsprdemod != nullptr)
		sp_wsprdemod->Demodulator::setLowPassAudioFilterCutOffFrequency(bandwidth);
}

#define PATIENCE FFTW_ESTIMATE
fftwf_plan _PLAN1, _PLAN2, _PLAN3;

unsigned long WSPRDemodulator::readwavfile(char *ptr_to_infile, int ntrmin, IQSampleVector &samples_out, SampleVector &audio)
{
	size_t i, j, npoints, nr;
	int nfft1, nfft2, nh2, i0;
	double df;
	float *idat, *qdat;

	nfft2 = 46080; // this is the number of downsampled points that will be returned
	nh2 = nfft2 / 2;

	if (ntrmin == 2)
	{
		nfft1 = nfft2 * 32; // need to downsample by a factor of 32
		df = 12000.0 / nfft1;
		i0 = 1500.0 / df + 0.5;
		npoints = 114 * 12000;
	}
	else if (ntrmin == 15)
	{
		nfft1 = nfft2 * 8 * 32;
		df = 12000.0 / nfft1;
		i0 = (1500.0 + 112.5) / df + 0.5;
		npoints = 8 * 114 * 12000;
	}
	else
	{
		fprintf(stderr, "This should not happen\n");
		return 1;
	}

	float *realin;
	fftwf_complex *fftin, *fftout;

	FILE *fp;
	short int *buf2;

	fp = fopen(ptr_to_infile, "rb");
	if (fp == NULL)
	{
		fprintf(stderr, "Cannot open data file '%s'\n", ptr_to_infile);
		return 1;
	}

	buf2 = (short int *)calloc(npoints, sizeof(short int));
	nr = fread(buf2, 2, 22, fp);	  // Read and ignore header
	nr = fread(buf2, 2, npoints, fp); // Read raw data
	fclose(fp);
	if (nr == 0)
	{
		fprintf(stderr, "No data in file '%s'\n", ptr_to_infile);
		return 1;
	}

	realin = (float *)fftwf_malloc(sizeof(float) * nfft1);
	fftout = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * (nfft1 / 2 + 1));
	_PLAN1 = fftwf_plan_dft_r2c_1d(nfft1, realin, fftout, PATIENCE);

	for (i = 0; i < npoints; i++)
	{
		realin[i] = buf2[i] / 32768.0;
		audio.push_back(realin[i]);
	}

	for (i = npoints; i < (size_t)nfft1; i++)
	{
		realin[i] = 0.0;
	}
	free(buf2);

	fftwf_execute(_PLAN1);
	fftwf_free(realin);

	fftin = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * nfft2);

	for (i = 0; i < (size_t)nfft2; i++)
	{
		j = i0 + i;
		if (i > (size_t)nh2)
			j = j - nfft2;
		fftin[i][0] = fftout[j][0];
		fftin[i][1] = fftout[j][1];
	}

	fftwf_free(fftout);
	fftout = (fftwf_complex *)fftwf_malloc(sizeof(fftwf_complex) * nfft2);
	_PLAN2 = fftwf_plan_dft_1d(nfft2, fftin, fftout, FFTW_BACKWARD, PATIENCE);
	fftwf_execute(_PLAN2);

	for (i = 0; i < (size_t)nfft2; i++)
	{
		complex<float> dat;
		
		dat.real(fftout[i][0] / 1000.0);
		dat.imag(fftout[i][1] / 1000.0);
		samples_out.push_back(dat);
	}

	fftwf_free(fftin);
	fftwf_free(fftout);
	return nfft2;
}