#include "FT8Processor.h"
#include "ft8.h"
#include "gui_ft8.h"
#include "gui_ft8bar.h"
#include <wsjtx_lib.h>
#include <map>
#include <regex>
#include <string>

std::mutex cycle_mu;
int cycle_count;
time_t saved_cycle_start;
std::map<std::string, bool> cycle_already;
extern unique_ptr<wsjtx_lib> wsjtx;

std::string remove_trailing_whitespace(const std::string &str)
{
	static const std::regex pattern("\\s+$"); // matches one or more whitespace characters at the end of the string
	return std::regex_replace(str, pattern, "");
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
		// 15 seconds has passed
		timeLastPrint = std::chrono::high_resolution_clock::now();
		time_t tt = std::chrono::system_clock::to_time_t(timeLastPrint);
		tm local_tm = *localtime(&tt);
		audiosamples.resize(15 * ft8_rate, 0.0);
		IntWsjTxVector samples;
		for (int i = 0; i < audiosamples.size(); i++)
		{
			samples.push_back((audiosamples[i] * 32768.0f));
		}
		guiQueue.push_back(GuiMessage(GuiMessage::action::clearWsjtx,0));
		wsjtx->decode(FT8, samples, 1000, 4);
		audiosamples.clear();
	}
}

void FT8Processor::AddaudioSample(SampleVector &audiosamples, time_t cycletime)
{
	//cycletimes.push(cycletime);
	samplebuffer.push(move(audiosamples));
}

void FT8Processor::Stop()
{
	stop_flag = true;
	samplebuffer.clear();
	samplebuffer.push_end();
}