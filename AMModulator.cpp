#include "AMModulator.h"
#include "Waterfall.h"
#include "Audiodefs.h"
#include "gui_speech.h"
#include "PeakLevelDetector.h"

static shared_ptr<AMModulator> sp_ammod;

bool AMModulator::create_modulator(int mode, double ifrate, int pcmrate, int tone, DataBuffer<IQSample> *source_buffer, AudioInput *audio_input)
{	
	if (sp_ammod != nullptr)
		return false;
	sp_ammod = make_shared<AMModulator>(mode, ifrate, pcmrate, tone, source_buffer, audio_input);
	sp_ammod->ammod_thread = std::thread(&AMModulator::operator(), sp_ammod);
	return true;
}

void AMModulator::destroy_modulator()
{
	if (sp_ammod == nullptr)
		return;
	sp_ammod->stop_flag = true;
	sp_ammod->ammod_thread.join();
	sp_ammod.reset();
}

AMModulator::~AMModulator()
{
	if (modAM)
		ampmodem_destroy(modAM);
	if (m_fft != nullptr)
		nco_crcf_destroy(m_fft);
	audio_input->set_tone(0);
}

AMModulator::AMModulator(int mode, double ifrate, int pcmrate, int tone, DataBuffer<IQSample> *source_buffer, AudioInput *audio_input)
	: Demodulator(ifrate, pcmrate, source_buffer, audio_input)
{
	float					mod_index = 0.99f; // modulation index (bandwidth)
	float					As = 60.0f; // resampling filter stop-band attenuation [dB]
	int						suppressed_carrier;
	liquid_ampmodem_type	am_mode;
		
	switch (mode)
	{
	case mode_usb:
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_USB;
		printf("tx mode LIQUID_AMPMODEM_USB carrier %d\n", suppressed_carrier);		
		break;
	case mode_lsb:
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_LSB;
		printf("tx mode LIQUID_AMPMODEM_LSB carrier %d\n", suppressed_carrier);		
		break;
	case mode_am:
		suppressed_carrier = 0;
		am_mode = LIQUID_AMPMODEM_DSB;
		printf("tx mode LIQUID_AMPMODEM_DSB carrier %d\n", suppressed_carrier);		
		break;
	case mode_dsb:
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_DSB;
		printf("tx mode LIQUID_AMPMODEM_DSB carrier %d\n", suppressed_carrier);		
		break;
	default:
		printf("Mode not correct\n");		
		return;
	}
	audio_input->set_tone(tone);
	m_fcutoff = 2500;
	if ((ifrate - pcmrate) > 0.1)
	{
		// only resample and tune if ifrate > pcmrate
		tune_offset(vfo.get_vfo_offset());
		set_resample_rate(ifrate / pcmrate); // UP sample to ifrate		
	}
	else
	{	// mix the transmid signal to the mid of the fft display
		//fft_offset(ifrate / 4);
	}
	set_bandpass_filter(2700.0f, 2000.0f, 500.0f, 150.0f);
	modAM = ampmodem_create(mod_index, am_mode, suppressed_carrier); 
	source_buffer->restart_queue();
}

void AMModulator::operator()()
{
	const auto startTime = std::chrono::high_resolution_clock::now();
	auto timeLastPrint = std::chrono::high_resolution_clock::now();
	
	unsigned int            fft_block = 0;
	bool                    inbuf_length_warning = false;
	SampleVector            audiosamples;
	IQSampleVector			samples_out;
	AudioProcessor			Speech;

	Speech.prepareToPlay(audio_output->get_samplerate());
	Speech.setThresholdDB(gspeech.get_threshold());
	Speech.setRatio(gspeech.get_ratio());
	Fft_calc.plan_fft(nfft_samples);
	m_audio_input->clear();
	if (gspeech.get_speech_mode())
		m_audio_input->set_gain(0);
	while (!stop_flag.load())
	{
		if (vfo.tune_flag.load())
		{
			vfo.tune_flag = false;
			tune_offset(vfo.get_vfo_offset());
		}
		if (!m_audio_input->read(audiosamples))
			continue;
		if (gspeech.get_speech_mode())
		{
			Speech.setRelease(gspeech.get_release());
			Speech.setRatio(gspeech.get_ratio());
			Speech.setAtack(gspeech.get_atack());
			Speech.setThresholdDB(gspeech.get_threshold());
			Speech.processBlock(audiosamples);
		}
		else
			m_audio_input->set_gain(0);

		calc_af_level(audiosamples);
		Fft_calc.set_signal_strength(get_af_level());
		process_tx(audiosamples, samples_out);
		m_transmit_buffer->push(move(samples_out));
		audiosamples.clear();
		
		const auto now = std::chrono::high_resolution_clock::now();
		if (timeLastPrint + std::chrono::seconds(5) < now)
		{
			timeLastPrint = now;
			const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);			
			printf("Queued transmitbuffer Samples %d\n", m_transmit_buffer->queued_samples());
			printf("peak %f db gain %f db threshold %f ratio %f atack %f release %f\n", Speech.getPeak(), Speech.getGain(), Speech.getThreshold(), Speech.getRatio(), Speech.getAtack(), Speech.getRelease());
		}
	}
	m_transmit_buffer->clear();
	m_transmit_buffer->push_end();
	printf("exit am_mod_thread\n");
}

void AMModulator::process(const IQSampleVector& samples_out, SampleVector& samples)
{
}

void AMModulator::process_tx(const SampleVector &samples, IQSampleVector &samples_out)
{
	IQSampleVector buf_mod, buf_filter, buf_out;
	unsigned int num_written;

	// Modulate audio to USB, LSB or DSB;
	for (auto &col : samples)
	{
		complex<float> f;
		ampmodem_modulate(modAM, col, &f);
		//printf("audio %f;I %f;Q %f \n", col, f.real(), f.imag());
		buf_mod.push_back(f);
	}
	double if_rms = rms_level_approx(buf_mod);
	m_if_level = 0.95 * m_if_level + 0.05 * if_rms;

	exec_bandpass_filter(buf_mod, buf_filter);
	buf_mod.clear();
	Resample(buf_filter, buf_out);
	buf_filter.clear();
	mix_up(buf_out, samples_out); // Mix up to vfo freq
	mix_up_fft(samples_out, buf_mod);
	Fft_calc.process_samples(buf_mod);
}

void AMModulator::mix_up_fft(const IQSampleVector& filter_in,
	IQSampleVector& filter_out)
{	
	if (m_fft)
	{
		for (auto& col : filter_in)
		{
			complex<float> v;
		
			nco_crcf_step(m_fft);
			nco_crcf_mix_up(m_fft, col, &v);
			filter_out.push_back(v);
		}
	}
	else
	{
		filter_out = filter_in;
	}
}

void AMModulator::fft_offset(long offset)
{
	if (m_fft != nullptr)
		nco_crcf_destroy(m_fft);
	float rad_per_sample   = ((2.0f * (float)M_PI * (float)(offset)) / (float)m_ifrate);
	m_fft = nco_crcf_create(LIQUID_NCO);
	nco_crcf_set_phase(m_fft, 0.0f);
	nco_crcf_set_frequency(m_fft, rad_per_sample);
}

void AMModulator::audio_feedback(const SampleVector &audiosamples)
{
	
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
}