#include "FMModulator.h"
#include "Waterfall.h"
#include "Audiodefs.h"

static shared_ptr<FMModulator> sp_fmmod;

bool FMModulator::create_modulator(int mode, double ifrate, int pcmrate, int tone, DataBuffer<IQSample16> *source_buffer, AudioInput *audio_input)
{	
	if (sp_fmmod != nullptr)
		return false;
	sp_fmmod = make_shared<FMModulator>(mode, ifrate, pcmrate, tone, source_buffer, audio_input);
	sp_fmmod->fmmod_thread = std::thread(&FMModulator::operator(), sp_fmmod);
	return true;
}

void FMModulator::destroy_modulator()
{
	if (sp_fmmod == nullptr)
		return;
	sp_fmmod->stop_flag = true;
	sp_fmmod->fmmod_thread.join();
	sp_fmmod.reset();
}

FMModulator::~FMModulator()
{
	if (modFM)
		freqmod_destroy(modFM);
}

FMModulator::FMModulator(int mode, double ifrate, int pcmrate, int tone, DataBuffer<IQSample16> *source_buffer, AudioInput *audio_input)
	: Demodulator(ifrate, pcmrate, source_buffer, audio_input)
{
	float kf          = 0.1f; // modulation factor
	
	m_tone = tone;
	m_fcutoff = 5000;
	Demodulator::set_filter(m_pcmrate, m_fcutoff);
	Demodulator::set_resample_rate(ifrate / pcmrate); // UP sample to ifrate
	modFM = freqmod_create(kf); 
	source_buffer->restart_queue();
	audio_input->open();
}

void FMModulator::operator()()
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

void FMModulator::process(const IQSampleVector& samples_in, SampleVector& samples)
{
	IQSampleVector				buf_mod, buf_filter, buf_out;
	IQSampleVector16			buf_out16;
	unsigned int				num_written;
	
	// Modulate audio to USB, LSB or DSB
	buf_mod.clear(); 
	for (auto& col : samples)
	{
		complex<float> f;	
		freqmod_modulate(modFM, col, &f);
		//printf("%f;%f;%f \n", col, f.real(), f.imag());
		buf_mod.push_back(f);
	}
	double if_rms = rms_level_approx(buf_mod);
	m_if_level = 0.95 * m_if_level + 0.05 * if_rms;
	
	// Low pass filter 5 Khz for NB FM
	buf_filter.clear(); 
	filter(buf_mod, buf_filter);	
	buf_out.clear();
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