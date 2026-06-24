#include "FMModulator.h"
#include "Spectrum.h"
#include "vfo.h"
#include "gui_speech.h"

static std::shared_ptr<FMModulator> sp_fmmod;

bool FMModulator::create_modulator(int mode, int duplex, double ifrate, audioTone tone, std::string filename, DataBuffer<IQSample> *source_buffer, AudioInput *audio_input)
{	
	if (sp_fmmod != nullptr)
		return false;
	sp_fmmod = make_shared<FMModulator>(mode, duplex, ifrate, tone, filename, source_buffer, audio_input);
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

FMModulator::FMModulator(int mode, int duplex_, double ifrate, audioTone tone, std::string filename, DataBuffer<IQSample> *source_buffer, AudioInput *audio_input)
	: play_prerecorded_file(filename), Demodulator(ifrate, source_buffer, audio_input) 
{
	float kf          = 0.5f; // modulation factor

	duplex = duplex_;
	audio_file_mode = false;
	audio_input->set_tone(tone);
	setBandPassFilter(3000.0f, 100.0f);
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
	//source_buffer->restart_queue();
}

void FMModulator::operator()()
{
	unsigned int            fft_block = 0;
	bool                    inbuf_length_warning = false;
	SampleVector            audiosamples;

	audio_file_mode = false;
	audioInputBuffer->clear();
	if (play_prerecorded_file.size() > 0)
	{
		if (!audioInputBuffer->StartPlayback(play_prerecorded_file))
			audio_file_mode = true;
	}
	while (!stop_flag.load())
	{
		if (vfo.tune_flag)
		{
			vfo.tune_flag = false;
			tune_offset(vfo.get_vfo_offset());
		}
		
		if (audioInputBuffer->IsBufferEmpty() && audio_file_mode)
		{
			stop_flag = true;
			std::cout << "Stop playback transmit buffer empty\n";
			audioInputBuffer->StopPlayback();
		}
		
		if (audioInputBuffer->read(audiosamples) == false)
		{
			printf("wait for input\n");
			usleep(1000); // wait 1024 audio sample time
			continue;
		}
		calc_af_signalstrength(audiosamples);
		process(audiosamples);
		audiosamples.clear();
	}
	transmitIQBuffer->push_end();
	
	if (audio_file_mode)
	{
		std::cout << "Close playback transmit\n";
		guiQueue.push_back(GuiMessage(GuiMessage::action::receive, 0));
		audioInputBuffer->StopPlayback();
		audio_file_mode = false;
	}
	printf("exit am_mod_thread\n");
}

void FMModulator::process(SampleVector& samples)
{
	IQSampleVector				buf_mod, buf_filter, buf_out;
	unsigned int				num_written;
	
	// Modulate audio to FM
	executeBandpassFilter(samples);
	buf_mod.reserve(samples.size());
	for (auto& col : samples)
	{
		std::complex<float> f;	
		freqmod_modulate(modFM, col, &f);
		//printf("%f;%f;%f \n", col, f.real(), f.imag());
		buf_mod.push_back(f);
	}
	Resample(buf_mod, buf_out);
	buf_filter.clear();
	buf_filter.reserve(buf_out.size());
	mix_up(buf_out, buf_filter); // Mix up to vfo freq
	if (!duplex)
		SpectrumGraph.ProcessWaterfall(buf_filter);
	transmitIQBuffer->push(std::move(buf_filter));
}

void FMModulator::setLowPassAudioFilterCutOffFrequency(int bandwidth)
{
	if (sp_fmmod != nullptr)
		sp_fmmod->Demodulator::setLowPassAudioFilterCutOffFrequency(bandwidth);
}