#include "DigitalTransmission.h"
#include "AMModulator.h"
#include "PeakLevelDetector.h"
#include "gui_ft8bar.h"
#include "gui_speech.h"
#include "gui_setup.h"
#include "gui_sdr.h"
#include <chrono>
#include "sdrstream.h"
#include "sdrberry.h"
#include "i2coutput.h"
#include "Catinterface.h"
#include "CatTcpServer.h"

extern DataBuffer<IQSample> source_buffer_rx;
extern DataBuffer<IQSample> source_buffer_tx;

static std::atomic<bool> startTX = false;
static ModulatorParameters param;
static std::shared_ptr<DigitalTransmission> StartDigitaltransmissionthread;

DigitalTransmission::DigitalTransmission(DataBuffer<IQSample> *source_buffer_tx,  DataBuffer<IQSample> *source_buffer_rx, AudioInput *audio_input)
{
	auto startTime = std::chrono::high_resolution_clock::now();
	auto today = std::chrono::floor<std::chrono::days>(startTime);

	printf("Stop RX stream \n");
	RX_Stream::destroy_rx_streaming_thread();
	if (sp_ammod != nullptr)
		return;
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::cout << "Start TX stream running. Number of samples " << param.signal.size() << std::chrono::hh_mm_ss(now - today) << std::endl;
	vfo.vfo_rxtx(false, true);
	vfo.pause_step(false);
	vfo.set_vfo(0, vfo_activevfo::None);
	vfo.pause_step(true);
	catinterface->SetTX(TX_MAN);
	cattcpserver->SetTX(TX_MAN);
	i2c_output.set_rxtx(true);
	Source_buffer_rx = source_buffer_rx;
	guift8bar.WaterfallSetMaxMin(500.0, 0);
	sp_ammod = make_shared<AMModulator>(param, source_buffer_tx, audio_input);
	sp_ammod->ammod_thread = std::thread(&AMModulator::operator(), sp_ammod);
	bool restart = (bool)Settings_file.get_int(default_radio, "restart_tx", 1);
	TX_Stream::create_tx_streaming_thread(ifrate, default_radio, param.txChannel, source_buffer_tx, ifrate_tx, guisdr.get_decimation(), restart);

	now = std::chrono::system_clock::now();
	std::cout << "Start TX stream running " << std::chrono::hh_mm_ss(now - today) << std::endl;
	return;
}

void DigitalTransmission::operator()()
{
	auto start = std::chrono::system_clock::now();
	auto today = std::chrono::floor<std::chrono::days>(start);

	std::cout << "Wait for TX stream finished running " << std::chrono::hh_mm_ss(start - today) << std::endl;
	sp_ammod->ammod_thread.join();
	startTX = false;
	sp_ammod.reset();
	TX_Stream::destroy_tx_streaming_thread();
	{
		std::unique_lock<std::mutex> gui_lock(gui_mutex);
		guift8bar.ClearTransmit();
	}
	auto end = std::chrono::system_clock::now();
	std::chrono::duration<double> elapsed_seconds = end - start;

	std::cout << "finished sending at " << std::chrono::hh_mm_ss(end - today)
			  << " elapsed time: " << elapsed_seconds.count() << "s"
			  << std::endl;
	RX_Stream::create_rx_streaming_thread(ifrate, default_radio, param.rxChannel, Source_buffer_rx, guisdr.get_decimation());
	vfo.vfo_rxtx(true, false);
	catinterface->SetTX(TX_OFF);
	cattcpserver->SetTX(TX_OFF);
	i2c_output.set_rxtx(false);
}

// Called in GUI loop in sdrberry.cpp
void DigitalTransmission::WaitForTimeSlot()
{
	auto startTime = std::chrono::high_resolution_clock::now();
	auto today = std::chrono::floor<std::chrono::days>(startTime);
	
	if (StartDigitaltransmissionthread != nullptr && !startTX)
	{
		StartDigitaltransmissionthread->DTthread.join();
		StartDigitaltransmissionthread.reset();
		StartDigitaltransmissionthread = nullptr;
	}
	auto millisecondsUTC = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	if (((millisecondsUTC / 100) % param.timeslotTensofSec == 0) && startTX & sp_ammod == nullptr)
	{
		auto now = std::chrono::system_clock::now();
		auto now_in_seconds = std::chrono::floor<std::chrono::seconds>(now);
		auto seconds_since_epoch = now_in_seconds.time_since_epoch().count();
		auto start_of_today = std::chrono::floor<std::chrono::days>(now);
		auto time_of_day = now - start_of_today;
		auto time_of_day_sec = std::chrono::floor<std::chrono::seconds>(time_of_day);
		std::chrono::hh_mm_ss hms{time_of_day_sec};
		
		auto current_seconds = hms.seconds().count();
		int qsotime = std::stoi(param.QsoTime);
//		if (param.cq || ((qsotime == 0 || qsotime == 30) && (current_seconds == 15 || current_seconds == 45)) ||
//			((qsotime == 15 || qsotime == 45) && (current_seconds == 0 || current_seconds == 30)))
		if (param.cq || ((qsotime >= 0 && qsotime <= 30) && (current_seconds == 0 || current_seconds == 30)) ||
			((qsotime >= 15 && qsotime <= 45) && (current_seconds == 15 || current_seconds == 45)))
		{	
			now = std::chrono::system_clock::now();
			std::cout << "tx start cycle " << std::chrono::hh_mm_ss(now - today) << "\n";
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