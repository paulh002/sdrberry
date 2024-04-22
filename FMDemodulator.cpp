#include "sdrberry.h"
#include "FMDemodulator.h"
#include <thread>

FMDemodulator::FMDemodulator(double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output)
	: Demodulator(ifrate, source_buffer, audio_output)
{
	m_bandwidth = 12500; // Narrowband FM
	Demodulator::set_resample_rate(audio_output->get_samplerate() / ifrate); // down sample to pcmrate
	Demodulator::setLowPassAudioFilter(audio_output->get_samplerate(), m_bandwidth);
	demodFM = freqdem_create(0.5);
}
	
void FMDemodulator::operator()()
{	
	const auto startTime = std::chrono::high_resolution_clock::now();
	auto timeLastPrint = std::chrono::high_resolution_clock::now();

	int ifilter{-1};
	long span;
	SampleVector audiosamples, audioframes;
	
	while (!stop_flag.load())
	{
		span = vfo.get_span();
		if (vfo.tune_flag.load())
		{
			vfo.tune_flag = false;
			tune_offset(vfo.get_vfo_offset());
		}

		IQSampleVector iqsamples = receiveIQBuffer->pull();
		if (iqsamples.empty())
		{
			usleep(500);
			continue;
		}
		calc_if_level(iqsamples);
		adjust_gain_phasecorrection(iqsamples, gbar.get_if());
		perform_fft(iqsamples);
		set_signal_strength();
		process(iqsamples, audiosamples);
		// Set nominal audio volume.
		audio_output->adjust_gain(audiosamples);
		for (auto &col : audiosamples)
		{
			// split the stream in blocks of samples of the size framesize
			audioframes.insert(audioframes.end(), col);
			if (audioframes.size() == audio_output->get_framesize())
			{
				SampleVector audio_stereo;

				mono_to_left_right(audioframes, audio_stereo);
				audio_output->write(audio_stereo);
				audioframes.clear();
			}
		}
		iqsamples.clear();
		audiosamples.clear();
		const auto now = std::chrono::high_resolution_clock::now();
		if (timeLastPrint + std::chrono::seconds(10) < now)
		{
			timeLastPrint = now;
			const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
			printf("Queued Audio Samples %d underrun %d\n", audio_output->queued_samples() / 2, audio_output->get_underrun());
		}
	}
}

void FMDemodulator::process(const IQSampleVector&	samples_in, SampleVector& audio)
{
	IQSampleVector		filter1, filter2;
		
	// mix to correct frequency
	mix_down(samples_in, filter1);
	Resample(filter1, filter2);
	filter1.clear();
	lowPassAudioFilter(filter2, filter1);
	filter2.clear();
	calc_signal_level(filter1);
	for (auto col : filter1)
	{
		float v;
		
		freqdem_demodulate(demodFM, col, &v);
		audio.push_back(v);
	}	
	filter1.clear();
}

FMDemodulator::~FMDemodulator()
{
	if (demodFM != nullptr)
	{
		freqdem_destroy(demodFM);
		demodFM = nullptr;		
	}
}

static	std::thread				fmdemod_thread;
shared_ptr<FMDemodulator>		sp_fmdemod;

bool FMDemodulator::create_demodulator(double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output)
{	
	if (sp_fmdemod != nullptr)
		return false;
	sp_fmdemod = make_shared<FMDemodulator>(ifrate, source_buffer, audio_output);
	fmdemod_thread = std::thread(&FMDemodulator::operator(), sp_fmdemod);
	return true;
}

void FMDemodulator::destroy_demodulator()
{
	if (sp_fmdemod == nullptr)
		return;
	sp_fmdemod->stop_flag = true; 
	fmdemod_thread.join();
	sp_fmdemod.reset();
}