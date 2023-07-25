#include "FT8Processor.h"
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



bool FT8Processor::create_modulator(std::shared_ptr<FT8Processor> &ft8processor, int mode)
{
	if (ft8processor != nullptr)
		return false;
	gft8.reset();
	ft8processor = std::make_shared<FT8Processor>(mode);
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

FT8Processor::FT8Processor(int wmode)
	:wsjtxmode{wmode}
{
}

FT8Processor::~FT8Processor()
{
}

void FT8Processor::operator()()
{
	double ttt_start = 0, capturesize;
	wsjtxMode decodeMode = wsjtxMode::FT8;
	const int nothreads = 4;

	switch (wsjtxmode)
	{
	case mode_ft8:
		decodeMode = wsjtxMode::FT8;
		capturesize = 15 * ft8_rate;
		break;
	case mode_ft4:
		decodeMode = wsjtxMode::FT4;
		capturesize = 75 * ft8_rate / 10;
		break;
	case mode_wspr:
		decodeMode = wsjtxMode::WSPR;
		break;
	}

	while (!stop_flag.load())
	{
		IntWsjTxVector samples;

		SampleVector audiosamples = samplebuffer.pull();
		if (!audiosamples.size())
			continue;
		for (int i = 0; i < audiosamples.size(); i++)
		{
			samples.push_back((audiosamples[i] * 32768.0f));
		}
		guiQueue.push_back(GuiMessage(GuiMessage::action::clearWsjtx,0));
		wsjtx->decode(decodeMode, samples, 1000, nothreads);
		audiosamples.clear();
		samples.clear();
	}
}

void FT8Processor::AddaudioSample(SampleVector &audiosamples)
{
	samplebuffer.push(move(audiosamples));
}

void FT8Processor::Stop()
{
	stop_flag = true;
	samplebuffer.clear();
	samplebuffer.push_end();
}