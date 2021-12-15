#include "AMModulator.h"
#include "Waterfall.h"
#include "Audiodefs.h"

static shared_ptr<AMModulator> sp_ammod;

bool AMModulator::create_modulator(int mode, double ifrate, int pcmrate, int tone, DataBuffer<IQSample16> *source_buffer, AudioInput *audio_input)
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
}

AMModulator::AMModulator(int mode, double ifrate, int pcmrate, int tone, DataBuffer<IQSample16> *source_buffer, AudioInput *audio_input)
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
	if (tone == 0)
		audio_input->open();
	
	m_tone = tone; 
	m_fcutoff = 2500;
	if ((ifrate - pcmrate) > 0.1)
	{	// only resample and tune if ifrate > pcmrate
		tune_offset(vfo.get_vfo_offset());
		set_reample_rate(ifrate / pcmrate); // UP sample to ifrate		
	}
	set_filter(m_pcmrate, m_fcutoff);
	
	modAM = ampmodem_create(mod_index, am_mode, suppressed_carrier); 
	source_buffer->restart_queue();
	if (tone == 0)
		audio_input->open();
}

void AMModulator::operator()()
{
	unsigned int            fft_block = 0;
	bool                    inbuf_length_warning = false;
	SampleVector            audiosamples;
	IQSampleVector			dummy;
		
	Fft_calc.plan_fft(nfft_samples * 10); 
	while (!stop_flag.load())
	{
		if (vfo.tune_flag)
		{
			vfo.tune_flag = false;
			tune_offset(vfo.get_vfo_offset());
		}
		
		if (m_tone)
		{
			m_audio_input->ToneBuffer(m_tone);
			m_transmit_buffer->wait_queue_empty(2);
		}
	
		if (m_audio_input->read(audiosamples) == false)
		{
			printf("wait for input\n");
			usleep(1000); // wait 1024 audio sample time
			continue;
		}
		Fft_calc.set_signal_strength(m_audio_input->get_rms_level());
		process(dummy, audiosamples);
		audiosamples.clear();
	}
	m_transmit_buffer->push_end();
	printf("exit am_mod_thread\n");
}

void AMModulator::process(const IQSampleVector& samples_in, SampleVector& samples)
{
	IQSampleVector				buf_mod, buf_filter, buf_out;
	IQSampleVector16			buf_out16;
	unsigned int				num_written;
	
	// Modulate audio to USB, LSB or DSB
	buf_mod.clear(); 
	for (auto& col : samples)
	{
		complex<float> f;	
		ampmodem_modulate(modAM, col, &f);
		//printf("%f;%f;%f \n", col, f.real(), f.imag());
		buf_mod.push_back(f);
	}
	double if_rms = rms_level_approx(buf_mod);
	m_if_level = 0.95 * m_if_level + 0.05 * if_rms;
	
	// Low pass filter 5 Khz for NB FM
	filter(buf_mod, buf_filter);	
	Resample(buf_filter, buf_out);
	buf_filter.clear();
	mix_up(buf_out, buf_filter); // Mix up to vfo freq	
	buf_out16.clear();
	for (auto& col : buf_filter)
	{
		complex<float> f;
		int16_t i, q;

		i = (int16_t)round(col.real() * 16384.0f);
		q = (int16_t)round(col.imag() * 16384.0f);
		IQSample16 s16 {i, q};
		buf_out16.push_back(s16);
	}
	Fft_calc.process_samples(buf_filter);
	m_transmit_buffer->push(move(buf_out16));

	buf_mod.clear();
	buf_out.clear();
	buf_filter.clear();
}