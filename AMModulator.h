#pragma once
#include "Demodulator.h"
#include "DigitalTransmission.h"

class AMModulator : public Demodulator
{
  public:
	AMModulator(ModulatorParameters &param, DataBuffer<IQSample> *source_buffer, AudioInput *audio_input);
	~AMModulator();

	void process(const SampleVector &samples, IQSampleVector &samples_out);
	void setsignal(vector<float> &signal);

	void operator()();
	static void destroy_modulator();
	static bool create_modulator(ModulatorParameters param, DataBuffer<IQSample> *source_buffer, AudioInput *audio_input);
	static void setLowPassAudioFilterCutOffFrequency(int bandwidth);

	atomic<bool> stop_flag{false};
	std::thread ammod_thread;

  private:
	ampmodem AMmodulatorHandle{nullptr};
	liquid_ampmodem_type am_mode;
	bool even;
	bool digitalmode;
	nco_crcf m_fft{nullptr};
	void fft_offset(long offset);
	void mix_up_fft(const IQSampleVector &filter_in, IQSampleVector &filter_out);
	SampleVector audioframes;
	void audio_feedback(const SampleVector &audiosamples);
	void WaitForTimeSlot();
	vector<float> signal;
};

extern shared_ptr<AMModulator> sp_ammod;