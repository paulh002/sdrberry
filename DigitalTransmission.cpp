#include "DigitalTransmission.h"
#include "AMModulator.h"
#include "PeakLevelDetector.h"
#include "gui_ft8bar.h"
#include "gui_speech.h"
#include <chrono>
#include "date.h"
#include "sdrstream.h"

extern DataBuffer<IQSample> source_buffer_rx;
extern DataBuffer<IQSample> source_buffer_tx;

static std::atomic<bool> startTX = false;
static ModulatorParameters param;
static shared_ptr<DigitalTransmission> StartDigitaltransmissionthread;

DigitalTransmission::DigitalTransmission(DataBuffer<IQSample> *source_buffer_tx,  DataBuffer<IQSample> *source_buffer_rx, AudioInput *audio_input)
{
	auto startTime = std::chrono::high_resolution_clock::now();
	auto today = date::floor<date::days>(startTime);

	printf("Stop RX stream \n");
	RX_Stream::destroy_rx_streaming_thread();
	if (sp_ammod != nullptr)
		return;
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	cout << "Start TX stream running. Number of samples " << param.signal.size() << date::make_time(now - today) << endl;
	vfo.vfo_rxtx(false, true);
	Source_buffer_rx = source_buffer_rx;
	guift8bar.WaterfallSetMaxMin(500.0, 0);
	sp_ammod = make_shared<AMModulator>(param, source_buffer_tx, audio_input);
	sp_ammod->ammod_thread = std::thread(&AMModulator::operator(), sp_ammod);
	TX_Stream::create_tx_streaming_thread(default_radio, param.txChannel, source_buffer_tx, ifrate_tx);

	now = std::chrono::system_clock::now();
	cout << "Start TX stream running " << date::make_time(now - today) << endl;
	return;
}

void DigitalTransmission::operator()()
{
	auto start = std::chrono::system_clock::now();
	auto today = date::floor<date::days>(start);

	cout << "Wait for TX stream finished running " << date::make_time(start - today) << endl;
	sp_ammod->ammod_thread.join();
	startTX = false;
	sp_ammod.reset();
	TX_Stream::destroy_tx_streaming_thread();
	guift8bar.ClearTransmit();

	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;

	std::cout << "finished sending at " << date::make_time(end - today)
			  << " elapsed time: " << elapsed_seconds.count() << "s"
			  << std::endl;
	RX_Stream::create_rx_streaming_thread(default_radio, param.rxChannel, Source_buffer_rx);
	vfo.vfo_rxtx(true, false);
}

// Called in GUI loop in sdrberry.cpp
void DigitalTransmission::WaitForTimeSlot()
{
	auto startTime = std::chrono::high_resolution_clock::now();
	auto today = date::floor<date::days>(startTime);
	
	if (StartDigitaltransmissionthread != nullptr && !startTX)
	{
		StartDigitaltransmissionthread->DTthread.join();
		StartDigitaltransmissionthread.reset();
		StartDigitaltransmissionthread = nullptr;
	}
	auto millisecondsUTC = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	if (((millisecondsUTC / 100) % param.timeslotTensofSec == 0) && startTX & sp_ammod == nullptr)
	{
		std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
		std::cout << "tx start cycle " << date::make_time(now - today) << "\n";
		if (StartDigitaltransmissionthread != nullptr)
		{
			StartDigitaltransmissionthread->DTthread.join();
			StartDigitaltransmissionthread.reset();
			StartDigitaltransmissionthread = nullptr;
		}
		StartDigitaltransmissionthread = make_shared<DigitalTransmission>(&source_buffer_tx, &source_buffer_rx, audio_input);
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