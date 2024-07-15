#include "FMModulator.h"
#include "Spectrum.h"
#include "vfo.h"

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
	float kf          = 0.1f; // modulation factor
	
	audio_input->set_tone(tone);
	Demodulator::setLowPassAudioFilter(audioSampleRate, 5000);
	Demodulator::set_resample_rate(ifrate / audio_input->get_samplerate()); // UP sample to ifrate
	modFM = freqmod_create(kf); 
	source_buffer->restart_queue();
}

void FMModulator::operator()()
{
	unsigned int            fft_block = 0;
	bool                    inbuf_length_warning = false;
	SampleVector            audiosamples;
	IQSampleVector			dummy;
		
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
	buf_mod.clear(); 
	for (auto& col : samples)
	{
		complex<float> f;	
		freqmod_modulate(modFM, col, &f);
		//printf("%f;%f;%f \n", col, f.real(), f.imag());
		buf_mod.push_back(f);
	}
	// Low pass filter 5 Khz for NB FM
	buf_filter.clear();
	lowPassAudioFilter(buf_mod, buf_filter);	
	buf_out.clear();
	Resample(buf_filter, buf_out);
	buf_filter.clear();
	mix_up(buf_out, buf_filter); // Mix up to vfo freq
	SpectrumGraph.ProcessWaterfall(buf_filter);
	transmitIQBuffer->push(move(buf_filter));
	buf_mod.clear();
	buf_out.clear();
	buf_filter.clear();
}

void FMModulator::setLowPassAudioFilterCutOffFrequency(int bandwidth)
{
	if (sp_fmmod != nullptr)
		sp_fmmod->Demodulator::setLowPassAudioFilterCutOffFrequency(bandwidth);
}