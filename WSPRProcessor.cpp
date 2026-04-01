#include "WSPRProcessor.h"
#include "gui_ft8.h"
#include "gui_ft8bar.h"
#include <wsjtx_lib.h>
#include <map>
#include <regex>
#include <string>
#include "tz.h"
#include "vfo.h"

//std::mutex cycle_mu;
//int cycle_count;
//time_t saved_cycle_start;
//std::map<std::string, bool> cycle_already;
extern unique_ptr<wsjtx_lib> wsjtx;
extern std::unique_ptr<FT8UdpClient> ft8udpclient;

DataQueue<std::vector<decoder_results>> WsprMessageQueue;

std::string remove_trailing_whitespace(const std::string &str);

bool WSPRProcessor::create_modulator(std::shared_ptr<WSPRProcessor> &WSPRprocessor, int mode)
{
	if (WSPRprocessor != nullptr)
		return false;
	gft8.reset();
	WSPRprocessor = std::make_shared<WSPRProcessor>(mode);
	WSPRprocessor->WSPRprocessor_thread = std::thread(&WSPRProcessor::operator(), WSPRprocessor);
	//ft8udpclient = std::make_unique<FT8UdpClient>(mode);
	guift8bar.send_status();
	return true;
}

void WSPRProcessor::destroy_modulator(std::shared_ptr<WSPRProcessor> &WSPRprocessor)
{
	if (WSPRprocessor == nullptr)
		return;
	WSPRprocessor->stop_flag = true;
	WSPRprocessor->Stop();
	WSPRprocessor->WSPRprocessor_thread.join();
	WSPRprocessor.reset();
	//ft8udpclient.reset();
}

WSPRProcessor::WSPRProcessor(int wmode)
	: wsjtxmode{wmode}
{
}

WSPRProcessor::~WSPRProcessor()
{
}

void WSPRProcessor::operator()()
{
	double ttt_start = 0, capturesize;
	wsjtxMode decodeMode = wsjtxMode::WSPR;
	const int nothreads = 4;
	decoder_options options;

	options.freq = vfo.get_active_vfo_freq();		  // Dial frequency
	strcpy(options.rcall,"");	  // Callsign of the RX station
	strcpy(options.rloc, "");	  // Locator of the RX station
	options.quickmode = 0;	  // Decoder option & tweak
	options.usehashtable = 0; //  ''
	options.npasses = 3;	  //  ''
	options.subtraction = 1;  //

	while (!stop_flag.load())
	{
		std::vector<decoder_results> results;
		IntWsjTxVector samples;

		SampleVector audiosamples = samplebuffer.pull();
		if (!audiosamples.size())
			continue;
		for (auto con : audiosamples)
		{
			samples.push_back((con * 32768.0f));
		}
		guift8bar.send_status(true);
		gft8.set_decode_start_time(std::chrono::system_clock::now());
		results = wsjtx->wspr_decode(samples, options);
		if (results.size())
			WsprMessageQueue.push(results);
		for (auto con : results)
		{
			printf("call: %s freq %ld %ld message %s\n", con.call, con.freq, vfo.get_active_vfo_freq(), con.message);
		}
		//ft8udpclient->SendHeartBeat();
		samples.clear();
	}
}

void WSPRProcessor::AddAudioSample(SampleVector samples)
{
	samplebuffer.push(std::move(samples));
}

void WSPRProcessor::Stop()
{
	stop_flag = true;
	samplebuffer.clear();
	samplebuffer.push_end();
}
