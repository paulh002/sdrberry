
#include <cstdio>
#include <cassert>
#include <cmath>
#include <unistd.h>
#include "FmDecode.h"
#include "DataBuffer.h"
#include "AudioOutput.h"

#include <liquid.h>
#include <complex>
#include <vector>
#include <mutex>
#include "Waterfall.h"
#include "vfo.h"

using namespace std;


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
        SampleVector tmp(move(m_buf_baseband));
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

    if (m_stereo_enabled) {

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
        audio = move(m_buf_mono);

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

FmDecoder_executer Fm_executer;
	
void FmDecoder_executer::init(double ifrate, double tuner_offset, int pcmrate, bool stereo, double bandwidth_pcm, unsigned int downsample, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output)
{
	fm = new FmDecoder(ifrate,             // sample_rate_if
		tuner_offset,                 // tuning_offset
		pcmrate,                           // sample_rate_pcm
		stereo,                            // stereo
		FmDecoder::default_deemphasis,     // deemphasis,
		FmDecoder::default_bandwidth_if,   // bandwidth_if
		FmDecoder::default_freq_dev,       // freq_dev
		bandwidth_pcm,                     // bandwidth_pcm
		downsample);
	
	m_source_buffer = source_buffer;
	m_audio_output = audio_output;
}
void FmDecoder_executer::set_volume(int vol)
{
	volume = ((double)vol) / 100.0;
}

FmDecoder_executer::~FmDecoder_executer()
{
	if (fm != NULL)
	    delete fm;
}	
	
pthread_t fm_thread;

void* rx_fm_thread(void* fm_ptr)
{
	unsigned int            block = 0, fft_block = 0;
	bool                    inbuf_length_warning = false;
	
	unique_lock<mutex> lock_fm(fm_finish); 
	Fft_calc.plan_fft(min(1024, (int)(ifrate / 100.0)));
	while (!stop_flag.load())
	{
		
		if (!inbuf_length_warning && Fm_executer.m_source_buffer->queued_samples() > 10 * 530000) {
			printf("\nWARNING: Input buffer is growing (system too slow) queued samples %u\n", Fm_executer.m_source_buffer->queued_samples());
			inbuf_length_warning = true;
		}
		
		if (Fm_executer.m_source_buffer->queued_samples() == 0)
		{
			usleep(5000);
			continue;
		}
		
		IQSampleVector iqsamples = Fm_executer.m_source_buffer->pull();
		if (iqsamples.empty())
		{
			usleep(5000);
			continue;
		}
		if (iqsamples.size() >= nfft_samples && fft_block == 5)
		{
			fft_block = 0;			
			Fft_calc.process_samples(iqsamples);
			Fft_calc.set_signal_strength(Fm_executer.fm->get_if_level()); 
		}
		fft_block++;			
		
		Fm_executer.fm->process(iqsamples, Fm_executer.m_audiosamples);
		// Measure audio level.
		samples_mean_rms(Fm_executer.m_audiosamples, Fm_executer.m_audio_mean, Fm_executer.m_audio_rms);
		Fm_executer.m_audio_level = 0.95 * Fm_executer.m_audio_level + 0.05 * Fm_executer.m_audio_rms;

		// Set nominal audio volume.
		Fm_executer.m_audio_output->adjust_gain(Fm_executer.m_audiosamples);	
		
		if (block > 0) 
		    {
			    Fm_executer.m_audio_output->write(Fm_executer.m_audiosamples);
			    //printf("write audiosamples %d \n", fm_ex->audiosamples.size());
            }
		
		block++;		
		iqsamples.clear();
		Fm_executer.m_audiosamples.clear();
	}
    pthread_exit(NULL);
}	


void create_fm_thread(double ifrate, double tuner_offset, int pcmrate, bool stereo, double bandwidth_pcm, unsigned int downsample, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output)
{
	
	
	Fm_executer.init(ifrate, tuner_offset, pcmrate, stereo, bandwidth_pcm, downsample, source_buffer, audio_output);
	int rc = pthread_create(&fm_thread, NULL, rx_fm_thread, (void *)&Fm_executer);
	
}

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

void start_fm(double ifrate, int pcmrate, bool stereo, DataBuffer<IQSample> *source_buffer, AudioOutput *audio_output)
{
	double bandwidth_pcm = MIN(15000, 0.45 * pcmrate);
	unsigned int downsample = max(1, int(ifrate / 215.0e3));
		
	printf("baseband downsampling factor %u\n", downsample);
	printf("audio sample rate: %u Hz\n", pcmrate);
	printf("audio bandwidth:   %.3f kHz\n", bandwidth_pcm * 1.0e-3);
	vfo.set_tuner_offset(0.25 * ifrate);
	create_fm_thread(ifrate, 0.25 * ifrate, pcmrate, stereo, bandwidth_pcm, downsample, source_buffer, audio_output);			
}
	/* end */
