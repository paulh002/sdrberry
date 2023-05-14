#include "DigitalTransmission.h"
#include "AMModulator.h"
#include "PeakLevelDetector.h"
#include "gui_ft8bar.h"
#include "gui_speech.h"
#include <chrono>
#include <ctime>


extern DataBuffer<IQSample> source_buffer_rx;
extern DataBuffer<IQSample> source_buffer_tx;

static std::atomic<bool> startTX = false;
static ModulatorParameters param;
static shared_ptr<DigitalTransmission> StartDigitaltransmissionthread;

DigitalTransmission::DigitalTransmission(ModulatorParameters &param, DataBuffer<IQSample> *source_buffer_tx, DataBuffer<IQSample> *source_buffer_rx, AudioInput *audio_input)
{

	printf("Stop RX stream \n");
	RX_Stream::destroy_rx_streaming_thread();
	if (sp_ammod != nullptr)
		return;
	auto start = std::chrono::system_clock::now();
	std::time_t the_time = std::chrono::system_clock::to_time_t(start);
	cout << "Start TX stream running. Number of samples " << param.ft8signal.size() << std::ctime(&the_time) << endl;
	vfo.vfo_rxtx(false, true);
	Source_buffer_rx = source_buffer_rx;
	sp_ammod = make_shared<AMModulator>(param, source_buffer_tx, audio_input);
	sp_ammod->ammod_thread = std::thread(&AMModulator::operator(), sp_ammod);
	TX_Stream::create_tx_streaming_thread(default_radio, default_rx_channel, source_buffer_tx, ifrate_tx);

	start = std::chrono::system_clock::now();
	the_time = std::chrono::system_clock::to_time_t(start);
	cout << "Start TX stream running" << std::ctime(&the_time) << endl;
	return;
}

void DigitalTransmission::operator()()
{
	auto start = std::chrono::system_clock::now();
	std::time_t the_time = std::chrono::system_clock::to_time_t(start);
	cout << "Wait for TX stream finished running " << std::ctime(&the_time) << endl;

	sp_ammod->ammod_thread.join();
	startTX = false;
	sp_ammod.reset();
	TX_Stream::destroy_tx_streaming_thread();

	auto end = std::chrono::system_clock::now();
	std::time_t end_time = std::chrono::system_clock::to_time_t(end);
	std::chrono::duration<double> elapsed_seconds = end - start;

	std::cout << "finished sending at " << std::ctime(&end_time)
			  << "elapsed time: " << elapsed_seconds.count() << "s"
			  << std::endl;
	RX_Stream::create_rx_streaming_thread(default_radio, default_rx_channel, Source_buffer_rx);
	guift8bar.ClearTransmit();
	vfo.vfo_rxtx(true, false);
}

// Called in GUI loop in sdrberry.cpp
void DigitalTransmission::WaitForTimeSlot()
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	time_t tt = std::chrono::system_clock::to_time_t(now);
	int sec = ((long long)tt % 15);
	if (StartDigitaltransmissionthread != nullptr && !startTX)
	{
		StartDigitaltransmissionthread->DTthread.join();
		StartDigitaltransmissionthread.reset();
		StartDigitaltransmissionthread = nullptr;
	}

	if (sec == 0 && startTX & sp_ammod == nullptr)
	{
		now = std::chrono::system_clock::now();
		tt = std::chrono::system_clock::to_time_t(now);
		tm local_tm = *localtime(&tt);
		printf("start tx cycle %2d:%2d:%2d\n", local_tm.tm_hour, local_tm.tm_min, local_tm.tm_sec);
		if (StartDigitaltransmissionthread != nullptr)
		{
			StartDigitaltransmissionthread->DTthread.join();
			StartDigitaltransmissionthread.reset();
			StartDigitaltransmissionthread = nullptr;
		}
		StartDigitaltransmissionthread = make_shared<DigitalTransmission>(param, &source_buffer_tx, &source_buffer_rx, audio_input);
		StartDigitaltransmissionthread->DTthread = std::thread(&DigitalTransmission::operator(), StartDigitaltransmissionthread);
		}
}

void DigitalTransmission::StartDigitalTransmission(ModulatorParameters param)
{
	if (!startTX)
	{
		::param = std::move(param);
		startTX = true;
	}
}

bool DigitalTransmission::CancelDigitalTransmission()
{
	if (startTX && StartDigitaltransmissionthread == nullptr)
	{
		startTX = false;
		return true;
	}
	return false;
}