#include "FMModulator.h"
#include "Spectrum.h"
#include "vfo.h"
#include "gui_speech.h"

static shared_ptr<FMModulator> sp_fmmod;

bool FMModulator::create_modulator(int mode, double ifrate, audioTone tone, DataBuffer<IQSample> *source_buffer, AudioInput *audio_input)
{	
	if (sp_fmmod != nullptr)
		return false;
	sp_fmmod = make_shared<FMModulator>(mode, ifrate, tone, source_buffer, audio_input);
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

FMModulator::FMModulator(int mode, double ifrate, audioTone tone, DataBuffer<IQSample> *source_buffer, AudioInput *audio_input)
	: Demodulator(ifrate, source_buffer, audio_input)
{
	float kf          = 0.5f; // modulation factor
	
	audio_input->set_tone(tone);
	Demodulator::setLowPassAudioFilter(audioSampleRate, 5000);
	if ((ifrate - audio_input->get_samplerate()) > 0.1)
	{
		float sample_ratio;

		// only resample and tune if ifrate > pcmrate
		tune_offset(vfo.get_vfo_offset_tx());
		sample_ratio = ifrate / (double)audio_input->get_samplerate();
		printf("ifrate %f, audiorate %d  Resample rate %f\n", ifrate, audio_input->get_samplerate(), sample_ratio);
		set_resample_rate(sample_ratio); // UP sample to ifrate
	}
	else
	{
		printf("No resample \n");
	}
	modFM = freqmod_create(kf);
	freqmod_print(modFM);
	source_buffer->restart_queue();
}

void FMModulator::operator()()
{
	unsigned int            fft_block = 0;
	bool                    inbuf_length_warning = false;
	SampleVector            audiosamples;
	IQSampleVector			dummy;

	audioInputBuffer->clear();
	while (!stop_flag.load())
	{
		if (vfo.tune_flag)
		{
			vfo.tune_flag = false;
			tune_offset(vfo.get_vfo_offset());
		}
		
		if (audioInputBuffer->read(audiosamples) == false)
		{
			printf("wait for input\n");
			usleep(1000); // wait 1024 audio sample time
			continue;
		}
		calc_af_level(audiosamples);
		set_signal_strength();
		process(dummy, audiosamples);
		audiosamples.clear();
	}
	transmitIQBuffer->push_end();
	printf("exit am_mod_thread\n");
}

void FMModulator::process(const IQSampleVector& samples_in, SampleVector& samples)
{
	IQSampleVector				buf_mod, buf_filter, buf_out;
	unsigned int				num_written;
	
	// Modulate audio to USB, LSB or DSB
	buf_mod.reserve(samples.size());
	for (auto& col : samples)
	{
		complex<float> f;	
		freqmod_modulate(modFM, col, &f);
		//printf("%f;%f;%f \n", col, f.real(), f.imag());
		buf_mod.push_back(f);
	}
	// Low pass filter 5 Khz for NB FM
	//buf_filter.reserve(buf_mod.size());
	//lowPassAudioFilter(buf_mod, buf_filter);
	Resample(buf_mod, buf_out);
	buf_filter.clear();
	buf_filter.reserve(buf_out.size());
	mix_up(buf_out, buf_filter); // Mix up to vfo freq
	SpectrumGraph.ProcessWaterfall(buf_filter);
	transmitIQBuffer->push(std::move(buf_filter));
}

void FMModulator::setLowPassAudioFilterCutOffFrequency(int bandwidth)
{
	if (sp_fmmod != nullptr)
		sp_fmmod->Demodulator::setLowPassAudioFilterCutOffFrequency(bandwidth);
}