#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <vector>
#include <ctime>
#include <time.h>
#include <string.h>
#include <mutex>
#include <map>

#include "sdrberry.h"
#include "FT8Demodulator.h"
#include "unpack.h"
#include "ft8.h"

static shared_ptr<FT8Demodulator>	sp_ft8demod;
std::mutex							ft8demod_mutex;
static std::chrono::high_resolution_clock::time_point starttime1{};
DataQueue<FT8Message> FT8Queue;

std::mutex cycle_mu;
volatile int cycle_count;
time_t saved_cycle_start;

std::map<std::string, bool> cycle_already;

int hcb(int *a91, double hz0, double hz1, double off,
		const char *comment, double snr, int pass,
		int correct_bits)
{
	std::string msg = unpack(a91);

	cycle_mu.lock();

	if (cycle_already.count(msg) > 0)
	{
		// already decoded this message on this cycle
		cycle_mu.unlock();
		return 1; // 1 => already seen, don't subtract.
	}

	cycle_already[msg] = true;
	cycle_count += 1;

	cycle_mu.unlock();

	struct tm result;
	gmtime_r(&saved_cycle_start, &result);

	/*printf("%02d%02d%02d %3d %3d %5.2f %6.1f %s\n",
		   result.tm_hour,
		   result.tm_min,
		   result.tm_sec,
		   (int)snr,
		   correct_bits,
		   off - 0.5,
		   hz0,
		   msg.c_str());
	fflush(stdout);
	*/
	
	/* gft8.add_line(result.tm_hour,
				  result.tm_min,
				  result.tm_sec,
				  (int)snr,
				  correct_bits,
				  off - 0.5,
				  hz0,
				  msg.c_str());
	*/

	FT8Queue.push(FT8Message(result.tm_hour,
							 result.tm_min,
							 result.tm_sec,
							 (int)snr,
							 correct_bits,
							 off - 0.5,
							 hz0,
							 msg));

	return 2; // 2 => new decode, do subtract.
}

void FT8Message::display()
{
	gft8.add_line(hh,
				  min,
				  sec,
				  snr,
				  correct_bits,
				  off - 0.5,
				  hz0,
				  msg.c_str());
}

bool FT8Demodulator::create_demodulator(double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output)
{
	if (sp_ft8demod != nullptr)
		return false;
	sp_ft8demod = make_shared<FT8Demodulator>(ifrate, source_buffer, audio_output);
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

FT8Demodulator::FT8Demodulator(double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output)
	: Demodulator(ifrate, source_buffer, audio_output)
{
	float mod_index = 0.03125f;
	int suppressed_carrier;
	liquid_ampmodem_type am_mode;

	Demodulator::set_resample_rate(ft8_rate / ifrate); // down sample to ft8_rate
	m_bandwidth = 2500; // SSB
	suppressed_carrier = 1;
	am_mode = LIQUID_AMPMODEM_USB;
	printf("mode LIQUID_AMPMODEM_USB carrier %d\n", suppressed_carrier);

	const auto startTime = std::chrono::high_resolution_clock::now();
	setLowPassAudioFilterCutOffFrequency(m_bandwidth);
	Demodulator::setLowPassAudioFilter(audioSampleRate, m_bandwidth);
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

	int ifilter{-1}, span, rcount{0}, dropped_frames{0};
	SampleVector audiosamples, audioframes;
	unique_lock<mutex> lock_am(ft8demod_mutex);
	IQSampleVector iqsamples;
	bool capture{false};
	double ttt_start;

	Fft_calc.plan_fft(nfft_samples);
	receiveIQBuffer->clear();
	while (!stop_flag.load())
	{
		span = gsetup.get_span();
		if (vfo.tune_flag.load() || m_span != span)
		{
			vfo.tune_flag = false;
			tune_offset(vfo.get_vfo_offset());
			set_span(span);
		}

		if (ifilter != get_lowPassAudioFilterCutOffFrequency())
		{
			ifilter = get_lowPassAudioFilterCutOffFrequency();
			printf("set filter %d\n", ifilter);
			setLowPassAudioFilter(audioSampleRate, ifilter);
		}

		IQSampleVector iqsamples = receiveIQBuffer->pull();
		if (iqsamples.empty())
		{
			usleep(500);
			continue;
		}
		adjust_gain(iqsamples, gbar.get_if());
		perform_fft(iqsamples);
		Fft_calc.set_signal_strength(get_if_level());
		process(iqsamples, audiosamples);
		// Check for 15 seconds
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		time_t tt = std::chrono::system_clock::to_time_t(now);
		long long cycle_start = tt - ((long long)tt % 15);
		if (tt - cycle_start >= 14 && !capture)
		{
			capture = true;
			tm local_tm = *localtime(&tt);
			printf("start cycle %d:%d:%d\n", local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec);
			audiosamples.clear();
			// drop all captured data
			startTime = std::chrono::high_resolution_clock::now();
			ttt_start = tt;
		}
		now = std::chrono::high_resolution_clock::now();
		auto timePassed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime);
		if ((timePassed.count() > 15000) && (capture))
		{
			int hints[2] = {2, 0}; // CQ
			double budget = 5;
			double ttt_end = ttt_start + audiosamples.size() / ft8_rate;
			long long nominal_start = audiosamples.size() - ft8_rate * (ttt_end - cycle_start - 0.5);
			
			// 15 seconds has passed
			timeLastPrint = std::chrono::high_resolution_clock::now();
			tm local_tm = *localtime(&tt);
			printf("end cycle %d:%d:%d Size %d start %lld\n", local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec, audiosamples.size(), nominal_start);
			printf("Buffer queue %d Audio Samples %ld \n", receiveIQBuffer->size(), audiosamples.size());

			capture = false;

			audiosamples.resize(15 * ft8_rate, 0.0);

			cycle_mu.lock();
			cycle_count = 0;
			saved_cycle_start = cycle_start; // for hcb() callback
			cycle_already.clear();
			gft8.clear();
			cycle_mu.unlock();

			entry(audiosamples.data(), audiosamples.size(), nominal_start, ft8_rate,
				  150,
				  3600, // 2900,
				  hints, hints, budget, budget, hcb,
				  0, (struct cdecode *)0);
			
			audiosamples.clear();
		}
		iqsamples.clear();
	}
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
	filter2.clear();
	calc_if_level(filter1);
	for (auto col : filter1)
	{
		float v;

		ampmodem_demodulate(m_demod, (liquid_float_complex)col, &v);
		audio.push_back(v);
	}
	filter1.clear();
	filter2.clear();
}