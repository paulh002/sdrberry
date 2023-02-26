#include "FT8Processor.h"
#include "ft8.h"
#include "gui_ft8.h"
#include "gui_ft8bar.h"
#include "unpack.h"
#include <map>

std::mutex cycle_mu;
int cycle_count;
time_t saved_cycle_start;
std::map<std::string, bool> cycle_already;

DataQueue<FT8Message> FT8Queue;

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
/*
	printf("%02d%02d%02d %3d %3d %5.2f %6.1f %s\n",
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

bool FT8Processor::create_modulator(std::shared_ptr<FT8Processor> &ft8processor)
{
	if (ft8processor != nullptr)
		return false;
	gft8.reset();
	ft8processor = std::make_shared<FT8Processor>();
	ft8processor->ft8processor_thread = std::thread(&FT8Processor::operator(), ft8processor);
	return true;
}

void FT8Processor::destroy_modulator(std::shared_ptr<FT8Processor> &ft8processor)
{
	if (ft8processor == nullptr)
		return;
	ft8processor->stop_flag = true;
	ft8processor->Stop();
	ft8processor->ft8processor_thread.join();
	ft8processor.reset();
}

FT8Processor::FT8Processor()
{
}

FT8Processor::~FT8Processor()
{
}

void FT8Processor::operator()()
{
	auto startTime = std::chrono::high_resolution_clock::now();
	auto timeLastPrint = std::chrono::high_resolution_clock::now();
	double ttt_start = 0;

	while (!stop_flag.load())
	{
		SampleVector audiosamples = samplebuffer.pull();
		if (!audiosamples.size())
			continue;

		time_t cyclestart = cycletimes.front();
		cycletimes.pop();
		// do something with it

		int hints[2] = {2, 0}; // CQ
		double budget = 5;
		//double ttt_end = ttt_start + audiosamples.size() / ft8_rate;
		long long nominal_start = 0; //audiosamples.size() - ft8_rate * (ttt_end - cycle_start - 0.5);

		// 15 seconds has passed
		timeLastPrint = std::chrono::high_resolution_clock::now();
		time_t tt = std::chrono::system_clock::to_time_t(timeLastPrint);
		tm local_tm = *localtime(&tt);
		printf("end cycle %d:%d:%d Size %ld start %lld\n", local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec, audiosamples.size(), nominal_start);
		audiosamples.resize(15 * ft8_rate, 0.0);

		cycle_mu.lock();
		cycle_count = 0;
		saved_cycle_start = cyclestart; // for hcb() callback
		cycle_already.clear();
		//if (guift8bar.GetFilter().length() > 0)
			gft8.clear();
		cycle_mu.unlock();

		entry(audiosamples.data(), audiosamples.size(), nominal_start, ft8_rate,
			  150,
			  3600, // 2900,
			  hints, hints, budget, budget, hcb,
			  0, (struct cdecode *)0);

		audiosamples.clear();
	}
}

void FT8Processor::AddaudioSample(SampleVector &audiosamples, time_t cycletime)
{
	cycletimes.push(cycletime);
	samplebuffer.push(move(audiosamples));
}

void FT8Processor::Stop()
{
	stop_flag = true;
	samplebuffer.clear();
	samplebuffer.push_end();
}