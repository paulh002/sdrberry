#include "sdrberry.h"
#include "AMDemodulator.h"
#include <thread>
#include "PeakLevelDetector.h"

static shared_ptr<AMDemodulator> sp_amdemod;
std::mutex amdemod_mutex;

static std::chrono::high_resolution_clock::time_point starttime1 {};

AMDemodulator::AMDemodulator(int mode, double ifrate, int pcmrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output)
	: Demodulator(ifrate, pcmrate, source_buffer, audio_output)
{
	float					mod_index  = 0.03125f; 
	int						suppressed_carrier;
	liquid_ampmodem_type	am_mode;

	sample_ratio = (1.05 * (float)pcmrate) / ifrate;
	Demodulator::set_resample_rate(sample_ratio); // down sample to pcmrate
	switch (mode)
	{
	case mode_usb:
		m_bandwidth = 2500; // SSB
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_USB;
		printf("mode LIQUID_AMPMODEM_USB carrier %d\n", suppressed_carrier);		
		break;
	case mode_cw:
		m_bandwidth = 500; // CW
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_LSB;
		printf("mode CW LIQUID_AMPMODEM_LSB carrier %d\n", suppressed_carrier);
		break;
	case mode_lsb:
		m_bandwidth = 2500; // SSB
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_LSB;
		printf("mode LIQUID_AMPMODEM_LSB carrier %d\n", suppressed_carrier);		
		break;
	case mode_am:
		m_bandwidth = 5000; // SSB
		suppressed_carrier = 0;
		am_mode = LIQUID_AMPMODEM_DSB;
		printf("mode LIQUID_AMPMODEM_DSB carrier %d\n", suppressed_carrier);		
		break;
	case mode_dsb:
		m_bandwidth = 5000; // SSB
		suppressed_carrier = 1;
		am_mode = LIQUID_AMPMODEM_DSB;
		printf("mode LIQUID_AMPMODEM_DSB carrier %d\n", suppressed_carrier);		
		break;
	default:
		printf("Mode not correct\n");		
		return;
	}
	const auto startTime = std::chrono::high_resolution_clock::now();
	m_fcutoff = m_bandwidth;
	Demodulator::set_filter(m_pcmrate, m_bandwidth);
	m_demod = ampmodem_create(mod_index, am_mode, suppressed_carrier);
	gbar.set_filter_slider(m_bandwidth);
	pMDecoder = make_unique<MorseDecoder>(m_pcmrate);
	auto now = std::chrono::high_resolution_clock::now();
	const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
	cout << "starttime :" << timePassed.count() << endl;
	//catinterface.SetSH(m_bandwidth);
}


AMDemodulator::~AMDemodulator()
{
	printf("AM destructor called \n");
	if (m_demod != nullptr)
	{
		ampmodem_destroy(m_demod);
		m_demod = nullptr;	
	}
}
	
void AMDemodulator::operator()()
{	
	const auto startTime = std::chrono::high_resolution_clock::now();
	auto timeLastPrint = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point now, start1;

	AudioProcessor Agc;

	int						ifilter {-1}, span, rcount {0}, dropped_frames {0};
	SampleVector            audiosamples, audioframes;
	unique_lock<mutex>		lock_am(amdemod_mutex);
	IQSampleVector			iqsamples;
	long long pr_time{0};
	int vsize, passes{0};

	Agc.prepareToPlay(audio_output->get_samplerate());
	Agc.setThresholdDB(gagc.get_threshold());
	Agc.setRatio(10);
	Fft_calc.plan_fft(nfft_samples);
	m_source_buffer->clear();
	while (!stop_flag.load())
	{
		start1 = std::chrono::high_resolution_clock::now();
		span = gsetup.get_span();
		if (vfo.tune_flag.load() || m_span != span)
		{
			vfo.tune_flag = false;
			tune_offset(vfo.get_vfo_offset());
			set_span(span);
		}
		
		if (ifilter != m_fcutoff.load())
		{
			ifilter = m_fcutoff.load();
			printf("set filter %d\n", ifilter);
			set_filter(m_pcmrate, ifilter);
		}
						
		IQSampleVector iqsamples = m_source_buffer->pull();	
		if (iqsamples.empty())
		{
			printf("No samples queued 2\n");
			usleep(500);
			continue;
		}
		int nosamples = iqsamples.size();
		passes++;
		adjust_gain(iqsamples, gbar.get_if());
		perform_fft(iqsamples);
		process(iqsamples, audiosamples);
		Fft_calc.set_signal_strength(get_if_level());
		samples_mean_rms(audiosamples, m_audio_mean, m_audio_rms);
		m_audio_level = 0.95 * m_audio_level + 0.05 * m_audio_rms;
		if (gagc.get_agc_mode())
		{
			Agc.setRelease(gagc.get_release());
			Agc.setRatio(gagc.get_ratio());
			Agc.setAtack(gagc.get_atack());
			Agc.setThresholdDB(gagc.get_threshold());
			Agc.processBlock(audiosamples);
		}
		// Set nominal audio volume.
		audio_output->adjust_gain(audiosamples);
		int noaudiosamples = audiosamples.size();
		for (auto& col : audiosamples)
		{
			// split the stream in blocks of samples of the size framesize 
			audioframes.insert(audioframes.end(), col);
			if (audioframes.size() == audio_output->get_framesize())
			{
				if ((audio_output->queued_samples() / 2) < 4096)
				{
					SampleVector		audio_stereo;

					mono_to_left_right(audioframes, audio_stereo);
					audio_output->write(audio_stereo);
					audioframes.clear();
				}
				else
				{
					//printf("drop frames\n");
					dropped_frames++;
					audioframes.clear();
				}
			}
		}
		iqsamples.clear();
		audiosamples.clear();
		now = std::chrono::high_resolution_clock::now();
		auto process_time1 = std::chrono::duration_cast<std::chrono::microseconds>(now - start1);
		if (pr_time < process_time1.count())
			pr_time = process_time1.count();
		if (timeLastPrint + std::chrono::seconds(10) < now)
		{
			timeLastPrint = now;
			const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
			printf("Radio samples %d Audio Samples %d Passes %d Queued Audio Samples %d droppedframes %d underrun %d\n", nosamples, noaudiosamples, passes, audio_output->queued_samples() / 2, dropped_frames, audio_output->get_underrun());
			printf("peak %f db gain %f db threshold %f ratio %f atack %f release %f\n", Agc.getPeak(), Agc.getGain(), Agc.getThreshold(), Agc.getRatio(), Agc.getAtack(),Agc.getRelease());
			printf("mean %f rms %f \n", m_audio_mean, m_audio_rms);
			pr_time = 0;
			passes = 0;
			if (rcount > 10 && audio_output->get_underrun() == 0 &&  dropped_frames > 15) 
			{
				sample_ratio = sample_ratio / 1.01 ;
				Demodulator::set_resample_rate(sample_ratio);
				rcount = 0;
			}
			if (rcount > 5 && audio_output->get_underrun() > 0 && dropped_frames == 0)
			{
				sample_ratio = 1.01 * sample_ratio;
				Demodulator::set_resample_rate(sample_ratio); // down sample to pcmrate
				rcount = 0;
			}
			rcount++;
			if (rcount > 11)
				rcount = 0;
			audio_output->clear_underrun();
			dropped_frames = 0;
		}
	}
	starttime1 = std::chrono::high_resolution_clock::now();
}

void AMDemodulator::process(const IQSampleVector&	samples_in, SampleVector& audio)
{
	IQSampleVector		filter1, filter2;
		
	// mix to correct frequency
	mix_down(samples_in, filter1);
	Resample(filter1, filter2);
	filter1.clear();
	filter(filter2, filter1);
	filter2.clear();
	calc_if_level(filter1);
	if (gsetup.get_cw())
		pMDecoder->decode(filter1);
	for (auto col : filter1)
	{
		float v;
		
		ampmodem_demodulate(m_demod, (liquid_float_complex)col, &v);
		audio.push_back(v);
	}	
	filter1.clear();
	filter2.clear();
}
	
bool AMDemodulator::create_demodulator(int mode, double ifrate, int pcmrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output)
{	
	if (sp_amdemod != nullptr)
		return false;
	sp_amdemod = make_shared<AMDemodulator>(mode, ifrate, pcmrate, source_buffer, audio_output);
	sp_amdemod->amdemod_thread = std::thread(&AMDemodulator::operator(), sp_amdemod);
	return true;
}

void AMDemodulator::destroy_demodulator()
{
	auto startTime = std::chrono::high_resolution_clock::now();
	
	if (sp_amdemod == nullptr)
		return;
	sp_amdemod->stop_flag = true;
	sp_amdemod->amdemod_thread.join();
	sp_amdemod.reset();

	auto now = std::chrono::high_resolution_clock::now();
	const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
	cout << "Stoptime AMDemodulator:" << timePassed.count() << endl;
}

void select_filter(int ifilter)
{
	if (sp_amdemod)
		sp_amdemod->set_filter(ifilter);
}