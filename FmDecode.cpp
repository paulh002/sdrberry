
#include <cstdio>
#include <cassert>
#include <cmath>
#include <unistd.h>
#include "DataBuffer.h"
#include "SdrberryTypeDefs.h"
#include "AudioOutput.h"
#include "Filter.h"
#include "FmDecode.h"
#include <liquid/liquid.h>
#include <complex>
#include <complex.h>
#include <vector>
#include <mutex>
#include "Spectrum.h"
#include "vfo.h"
#include "PeakLevelDetector.h"
#include "Limiter.h"
#include "gui_agc.h"
#include "Agc_class.h"
#include "gui_bar.h"

using namespace std;

static std::thread fmbroadbanddemod_thread;
shared_ptr<FMBroadBandDemodulator> sp_fmbroadbanddemod;

static atomic_bool stop_flag(false);

/*
 *
 * FM Decoder adapted from SoftFM https://github.com/jorisvr/SoftFM
 *
 *
 **/

/** Compute mean and RMS over a sample vector. */
inline void samples_mean_rms(const SampleVector &samples,
							 double &mean, double &rms)
{
	Sample vsum = 0;
	Sample vsumsq = 0;

	unsigned int n = samples.size();
	for (unsigned int i = 0; i < n; i++)
	{
		Sample v = samples[i];
		vsum += v;
		vsumsq += v * v;
	}

	mean = vsum / n;
	rms = sqrt(vsumsq / n);
}

IQSample::value_type rms_level_approx(const IQSampleVector &samples)
{
	unsigned int n = samples.size();
	n = (n + 63) / 64;

	IQSample::value_type level = 0;
	for (unsigned int i = 0; i < n; i++)
	{
		const IQSample &s = samples[i];
		IQSample::value_type re = s.real(), im = s.imag();
		level += re * re + im * im;
	}

	return sqrt(level / n);
}

/** Fast approximation of atan function. */
static inline Sample fast_atan(Sample x)
{
    // http://stackoverflow.com/questions/7378187/approximating-inverse-trigonometric-funcions

    Sample y = 1;
    Sample p = 0;

    if (x < 0) {
        x = -x;
        y = -1;
    }

    if (x > 1) {
        p = y;
        y = -y;
        x = 1 / x;
    }

    const Sample b = 0.596227;
    y *= (b*x + x*x) / (1 + 2*b*x + x*x);

    return (y + p) * Sample(M_PI_2);
}

/* ****************  class PhaseDiscriminator  **************** */

// Construct phase discriminator.
PhaseDiscriminator::PhaseDiscriminator(double max_freq_dev)
    : m_freq_scale_factor(1.0 / (max_freq_dev * 2.0 * M_PI))
{ }


// Process samples.
void PhaseDiscriminator::process(const IQSampleVector& samples_in,
                                 SampleVector& samples_out)
{
    unsigned int n = samples_in.size();
    IQSample s0 = m_last_sample;

    samples_out.resize(n);

    for (unsigned int i = 0; i < n; i++) {
        IQSample s1(samples_in[i]);
        IQSample d(conj(s0) * s1);
// TODO : implement fast approximation of atan2
        Sample w = atan2(d.imag(), d.real());
        samples_out[i] = w * m_freq_scale_factor;
        s0 = s1;
    }

    m_last_sample = s0;
}


/* ****************  class PilotPhaseLock  **************** */

// Construct phase-locked loop.
PilotPhaseLock::PilotPhaseLock(double freq, double bandwidth, double minsignal)
{
    /*
     * This is a type-2, 4th order phase-locked loop.
     *
     * Open-loop transfer function:
     *   G(z) = K * (z - q1) / ((z - p1) * (z - p2) * (z - 1) * (z - 1))
     *   K  = 3.788 * (bandwidth * 2 * Pi)**3
     *   q1 = exp(-0.1153 * bandwidth * 2*Pi)
     *   p1 = exp(-1.146 * bandwidth * 2*Pi)
     *   p2 = exp(-5.331 * bandwidth * 2*Pi)
     *
     * I don't understand what I'm doing; hopefully it will work.
     */

    // Set min/max locking frequencies.
    m_minfreq = (freq - bandwidth) * 2.0 * M_PI;
    m_maxfreq = (freq + bandwidth) * 2.0 * M_PI;

    // Set valid signal threshold.
    m_minsignal  = minsignal;
    m_lock_delay = int(20.0 / bandwidth);
    m_lock_cnt   = 0;
    m_pilot_level = 0;

    // Create 2nd order filter for I/Q representation of phase error.
    // Filter has two poles, unit DC gain.
    double p1 = exp(-1.146 * bandwidth * 2.0 * M_PI);
    double p2 = exp(-5.331 * bandwidth * 2.0 * M_PI);
    m_phasor_a1 = - p1 - p2;
    m_phasor_a2 = p1 * p2;
    m_phasor_b0 = 1 + m_phasor_a1 + m_phasor_a2;

    // Create loop filter to stabilize the loop.
    double q1 = exp(-0.1153 * bandwidth * 2.0 * M_PI);
    m_loopfilter_b0 = 0.62 * bandwidth * 2.0 * M_PI;
    m_loopfilter_b1 = - m_loopfilter_b0 * q1;

    // After the loop filter, the phase error is integrated to produce
    // the frequency. Then the frequency is integrated to produce the phase.
    // These integrators form the two remaining poles, both at z = 1.

    // Initialize frequency and phase.
    m_freq  = freq * 2.0 * M_PI;
    m_phase = 0;

    m_phasor_i1 = 0;
    m_phasor_i2 = 0;
    m_phasor_q1 = 0;
    m_phasor_q2 = 0;
    m_loopfilter_x1 = 0;

    // Initialize PPS generator.
    m_pilot_periods = 0;
    m_pps_cnt       = 0;
    m_sample_cnt    = 0;
}


// Process samples.
void PilotPhaseLock::process(const SampleVector& samples_in,
                             SampleVector& samples_out)
{
    unsigned int n = samples_in.size();

    samples_out.resize(n);

    bool was_locked = (m_lock_cnt >= m_lock_delay);
    m_pps_events.clear();

    if (n > 0)
        m_pilot_level = 1000.0;

    for (unsigned int i = 0; i < n; i++) {

        // Generate locked pilot tone.
        Sample psin = sin(m_phase);
        Sample pcos = cos(m_phase);

        // Generate double-frequency output.
        // sin(2*x) = 2 * sin(x) * cos(x)
        samples_out[i] = 2 * psin * pcos;

        // Multiply locked tone with input.
        Sample x = samples_in[i];
        Sample phasor_i = psin * x;
        Sample phasor_q = pcos * x;

        // Run IQ phase error through low-pass filter.
        phasor_i = m_phasor_b0 * phasor_i
                   - m_phasor_a1 * m_phasor_i1
                   - m_phasor_a2 * m_phasor_i2;
        phasor_q = m_phasor_b0 * phasor_q
                   - m_phasor_a1 * m_phasor_q1
                   - m_phasor_a2 * m_phasor_q2;
        m_phasor_i2 = m_phasor_i1;
        m_phasor_i1 = phasor_i;
        m_phasor_q2 = m_phasor_q1;
        m_phasor_q1 = phasor_q;

        // Convert I/Q ratio to estimate of phase error.
        Sample phase_err;
        if (phasor_i > abs(phasor_q)) {
            // We are within +/- 45 degrees from lock.
            // Use simple linear approximation of arctan.
            phase_err = phasor_q / phasor_i;
        } else if (phasor_q > 0) {
            // We are lagging more than 45 degrees behind the input.
            phase_err = 1;
        } else {
            // We are more than 45 degrees ahead of the input.
            phase_err = -1;
        }

        // Detect pilot level (conservative).
        m_pilot_level = min(m_pilot_level, phasor_i);

        // Run phase error through loop filter and update frequency estimate.
        m_freq += m_loopfilter_b0 * phase_err
                  + m_loopfilter_b1 * m_loopfilter_x1;
        m_loopfilter_x1 = phase_err;

        // Limit frequency to allowable range.
        m_freq = max(m_minfreq, min(m_maxfreq, m_freq));

        // Update locked phase.
        m_phase += m_freq;
        if (m_phase > 2.0 * M_PI) {
            m_phase -= 2.0 * M_PI;
            m_pilot_periods++;

            // Generate pulse-per-second.
            if (m_pilot_periods == pilot_frequency) {
                m_pilot_periods = 0;
                if (was_locked) {
                    struct PpsEvent ev;
                    ev.pps_index      = m_pps_cnt;
                    ev.sample_index   = m_sample_cnt + i;
                    ev.block_position = double(i) / double(n);
                    m_pps_events.push_back(ev);
                    m_pps_cnt++;
                }
            }
        }
    }

    // Update lock status.
    if (2 * m_pilot_level > m_minsignal) {
        if (m_lock_cnt < m_lock_delay)
            m_lock_cnt += n;
    } else {
        m_lock_cnt = 0;
    }

    // Drop PPS events when pilot not locked.
    if (m_lock_cnt < m_lock_delay) {
        m_pilot_periods = 0;
        m_pps_cnt = 0;
        m_pps_events.clear();
    }

    // Update sample counter.
    m_sample_cnt += n;
}


/* ****************  class FmDecoder  **************** */

FmDecoder::FmDecoder(double sample_rate_if,
                     double tuning_offset,
                     double sample_rate_pcm,
                     bool   stereo,
                     double deemphasis,
                     double bandwidth_if,
                     double freq_dev,
                     double bandwidth_pcm,
                     unsigned int downsample)

    // Initialize member fields
    : m_sample_rate_if(sample_rate_if)
    , m_sample_rate_baseband(sample_rate_if / downsample)
    , m_tuning_table_size(64)
    , m_tuning_shift(lrint(-64.0 * tuning_offset / sample_rate_if))
    , m_freq_dev(freq_dev)
    , m_downsample(downsample)
    , m_stereo_enabled(stereo)
    , m_stereo_detected(false)
    , m_if_level(0)
    , m_baseband_mean(0)
    , m_baseband_level(0)

    // Construct FineTuner
    , m_finetuner(m_tuning_table_size, m_tuning_shift)

    // Construct LowPassFilterFirIQ
    , m_iffilter(10, bandwidth_if / sample_rate_if)

    // Construct PhaseDiscriminator
    , m_phasedisc(freq_dev / sample_rate_if)

    // Construct DownsampleFilter for baseband
    , m_resample_baseband(8 * downsample, 0.4 / downsample, downsample, true)

    // Construct PilotPhaseLock
    , m_pilotpll(pilot_freq / m_sample_rate_baseband,       // freq
                 50 / m_sample_rate_baseband,               // bandwidth
                 0.04)                                      // minsignal

    // Construct DownsampleFilter for mono channel
    , m_resample_mono(
        int(m_sample_rate_baseband / 1000.0),               // filter_order
        bandwidth_pcm / m_sample_rate_baseband,             // cutoff
        m_sample_rate_baseband / sample_rate_pcm,           // downsample
        false)                                              // integer_factor

    // Construct DownsampleFilter for stereo channel
    , m_resample_stereo(
        int(m_sample_rate_baseband / 1000.0),               // filter_order
        bandwidth_pcm / m_sample_rate_baseband,             // cutoff
        m_sample_rate_baseband / sample_rate_pcm,           // downsample
        false)                                              // integer_factor

    // Construct HighPassFilterIir
    , m_dcblock_mono(30.0 / sample_rate_pcm)
    , m_dcblock_stereo(30.0 / sample_rate_pcm)

    // Construct LowPassFilterRC
    , m_deemph_mono(
        (deemphasis == 0) ? 1.0 : (deemphasis * sample_rate_pcm * 1.0e-6))
    , m_deemph_stereo(
        (deemphasis == 0) ? 1.0 : (deemphasis * sample_rate_pcm * 1.0e-6))

{
    // nothing more to do
}


void FmDecoder::process(const IQSampleVector& samples_in,
                        SampleVector& audio)
{
    // Fine tuning.
    m_finetuner.process(samples_in, m_buf_iftuned);

    // Low pass filter to isolate station.
    m_iffilter.process(m_buf_iftuned, m_buf_iffiltered);

    // Measure IF level.
    double if_rms = rms_level_approx(m_buf_iffiltered);
    m_if_level = 0.95 * m_if_level + 0.05 * if_rms;
		
    // Extract carrier frequency.
    m_phasedisc.process(m_buf_iffiltered, m_buf_baseband);

    // Downsample baseband signal to reduce processing.
    if (m_downsample > 1) {
		SampleVector tmp(std::move(m_buf_baseband));
        m_resample_baseband.process(tmp, m_buf_baseband);
    }

    // Measure baseband level.
    double baseband_mean, baseband_rms;
    samples_mean_rms(m_buf_baseband, baseband_mean, baseband_rms);
    m_baseband_mean  = 0.95 * m_baseband_mean + 0.05 * baseband_mean;
    m_baseband_level = 0.95 * m_baseband_level + 0.05 * baseband_rms;

    // Extract mono audio signal.
    m_resample_mono.process(m_buf_baseband, m_buf_mono);

    // DC blocking and de-emphasis.
    m_dcblock_mono.process_inplace(m_buf_mono);
    m_deemph_mono.process_inplace(m_buf_mono);

	if (m_stereo_enabled && audio_output->get_channels() > 1) {

        // Lock on stereo pilot.
        m_pilotpll.process(m_buf_baseband, m_buf_rawstereo);
        m_stereo_detected = m_pilotpll.locked();

        // Demodulate stereo signal.
        demod_stereo(m_buf_baseband, m_buf_rawstereo);

        // Extract audio and downsample.
        // NOTE: This MUST be done even if no stereo signal is detected yet,
        // because the downsamplers for mono and stereo signal must be
        // kept in sync.
        m_resample_stereo.process(m_buf_rawstereo, m_buf_stereo);

        // DC blocking and de-emphasis.
        m_dcblock_stereo.process_inplace(m_buf_stereo);
        m_deemph_stereo.process_inplace(m_buf_stereo);

        if (m_stereo_detected) {

            // Extract left/right channels from mono/stereo signals.
            stereo_to_left_right(m_buf_mono, m_buf_stereo, audio);

        } else {

            // Duplicate mono signal in left/right channels.
            mono_to_left_right(m_buf_mono, audio);

        }

    } else {

        // Just return mono channel.
		audio = std::move(m_buf_mono);

    }
}


// Demodulate stereo L-R signal.
void FmDecoder::demod_stereo(const SampleVector& samples_baseband,
                             SampleVector& samples_rawstereo)
{
    // Just multiply the baseband signal with the double-frequency pilot.
    // And multiply by two to get the full amplitude.
    // That's all.

    unsigned int n = samples_baseband.size();
    assert(n == samples_rawstereo.size());

    for (unsigned int i = 0; i < n; i++) {
        samples_rawstereo[i] *= 2 * samples_baseband[i];
    }
}


// Duplicate mono signal in left/right channels.
void FmDecoder::mono_to_left_right(const SampleVector& samples_mono,
                                   SampleVector& audio)
{
    unsigned int n = samples_mono.size();

    audio.resize(2*n);
    for (unsigned int i = 0; i < n; i++) {
        Sample m = samples_mono[i];
        audio[2*i]   = m;
        audio[2*i+1] = m;
    }
}


// Extract left/right channels from mono/stereo signals.
void FmDecoder::stereo_to_left_right(const SampleVector& samples_mono,
                                     const SampleVector& samples_stereo,
                                     SampleVector& audio)
{
    unsigned int n = samples_mono.size();
    assert(n == samples_stereo.size());

    audio.resize(2*n);
    for (unsigned int i = 0; i < n; i++) {
        Sample m = samples_mono[i];
        Sample s = samples_stereo[i];
        audio[2*i]   = m + s;
        audio[2*i+1] = m - s;
    }
}

void FMBroadBandDemodulator::operator()()
{
	const auto startTime = std::chrono::high_resolution_clock::now();
	auto timeLastPrint = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point now, start1, start2, SampleTimeStart, SampleTimeNow;

	int ifilter{-1}, lowPassAudioFilterCutOffFrequency{200000};
	int droppedFrames{0};
	long span;
	SampleVector audiosamples, audioframes;
	long long pr_time{0};
	int vsize, passes{0};
	int phase = 0;

	int thresholdDroppedFrames = Settings_file.get_int(default_radio, "thresholdDroppedFrames", 2);
	int thresholdUnderrun = Settings_file.get_int(default_radio, "thresholdUnderrun", 1);
	int limiterAtack = Settings_file.get_int(Limiter::getsetting(), "limiterAtack", 10);
	int limiterDecay = Settings_file.get_int(Limiter::getsetting(), "limiterDecay", 500);
	Limiter limiter(limiterAtack, limiterDecay, ifSampleRate);
	AudioProcessor Agc;

	Agc.prepareToPlay(audioOutputBuffer->get_samplerate());
	Agc.setThresholdDB(gagc.get_threshold());
	Agc.setRatio(10);
	receiveIQBuffer->clear();
	audioOutputBuffer->CopyUnderrunSamples(true);
	audioOutputBuffer->clear_underrun();
	setLowPassAudioFilter(ifrate, lowPassAudioFilterCutOffFrequency);
	SampleTimeStart = SampleTimeNow = std::chrono::high_resolution_clock::now();
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
		// Measure time between samples
		SampleTimeNow = std::chrono::high_resolution_clock::now();
		auto SampleTime = std::chrono::duration_cast<std::chrono::microseconds>(SampleTimeNow - SampleTimeStart);
		SampleTimeStart = std::chrono::high_resolution_clock::now();
		
		dc_filter(iqsamples);
		int nosamples = iqsamples.size();
		calc_if_level(iqsamples);
		limiter.Process(iqsamples);
		gain_phasecorrection(iqsamples, gbar.get_if());
		perform_fft(iqsamples);
		set_signal_strength();
		process(iqsamples, audiosamples);
		// Set nominal audio volume.
		audio_output->adjust_gain(audiosamples);
		int noaudiosamples = audiosamples.size();
		for (auto &col : audiosamples)
		{
			// split the stream in blocks of samples of the size framesize
			audioframes.insert(audioframes.end(), col);
			if (audioframes.size() == (2 * audio_output->get_framesize()))
			{
				phase++;
				if (phase > 100)
					phase = 0;
				if ((audioOutputBuffer->queued_samples()) < 100 * get_audioBufferSize())
				{
					audio_output->write(audioframes);
					audioframes.clear();
				}
				else
				{
					droppedFrames++;
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

		FlashGainSlider(limiter.getEnvelope());

		if (timeLastPrint + std::chrono::seconds(10) < now)
		{
			timeLastPrint = now;
			const auto timePassed = std::chrono::duration_cast<std::chrono::microseconds>(now - startTime);
			printf("Buffer queue %d Radio samples %d Audio Samples %d Passes %d Queued Audio Samples %d droppedframes %d underrun %d\n", receiveIQBuffer->size(), nosamples, noaudiosamples, passes, audioOutputBuffer->queued_samples() / 2, droppedFrames, audioOutputBuffer->get_underrun());
			printf("peak %f db gain %f db threshold %f ratio %f atack %f release %f\n", Agc.getPeak(), Agc.getGain(), Agc.getThreshold(), Agc.getRatio(), Agc.getAtack(), Agc.getRelease());
			printf("rms %f db %f envelope %f\n", get_if_level(), 20 * log10(get_if_level()), limiter.getEnvelope());
			//printf("IQ Balance I %f Q %f Phase %f\n", get_if_levelI() * 10000.0, get_if_levelQ() * 10000.0, get_if_Phase());
			//std::cout << "SoapySDR samples " << gettxNoSamples() <<" sample rate " << get_rxsamplerate() << " ratio " << (double)audioSampleRate / get_rxsamplerate() << "\n";
			pr_time = 0;
			passes = 0;
			audioOutputBuffer->clear_underrun();
			droppedFrames = 0;
		}
	}
	audioOutputBuffer->CopyUnderrunSamples(false);
}

void FMBroadBandDemodulator::process(IQSampleVector &samples_in, SampleVector &audio)
{
	// mix to correct frequency
	mix_down(samples_in);
	calc_signal_level(samples_in);
	pfm->process(samples_in, audio);
}


#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

bool FMBroadBandDemodulator::create_demodulator(double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output, int stereo)
{
	if (sp_fmbroadbanddemod != nullptr)
		return false;
	sp_fmbroadbanddemod = make_shared<FMBroadBandDemodulator>(ifrate, source_buffer, audio_output, stereo);
	fmbroadbanddemod_thread = std::thread(&FMBroadBandDemodulator::operator(), sp_fmbroadbanddemod);
	return true;
}

void FMBroadBandDemodulator::destroy_demodulator()
{
	if (sp_fmbroadbanddemod == nullptr)
		return;
	sp_fmbroadbanddemod->stop_flag = true;
	fmbroadbanddemod_thread.join();
	sp_fmbroadbanddemod.reset();
}

FMBroadBandDemodulator::FMBroadBandDemodulator(double ifrate, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output, int stereo)
	: Demodulator(ifrate, source_buffer, audio_output)
{
	int pcmrate = audio_output->get_samplerate();
	pfm = make_unique<FmDecoder>(ifrate,							// sample_rate_if
								 ifrate,							// tuning_offset
								 pcmrate,							// sample_rate_pcm
								 stereo,							// stereo
								 FmDecoder::default_deemphasis,		// deemphasis,
								 FmDecoder::default_bandwidth_if,	// bandwidth_if
								 FmDecoder::default_freq_dev,		// freq_dev
								 MIN(15000, 0.45 * pcmrate),		// bandwidth_pcm
								 MAX(1, int(ifrate / 215.0e3)));    // downsample
}

FMBroadBandDemodulator::~FMBroadBandDemodulator()
{

}