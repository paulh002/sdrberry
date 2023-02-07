#include "DigitalTransmission.h"
#include "AMModulator.h"
#include "PeakLevelDetector.h"
#include "Waterfall.h"
#include "gui_ft8bar.h"
#include "gui_speech.h"
#include <chrono>
#include <ctime>


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