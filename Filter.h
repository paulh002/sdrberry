#ifndef SOFTFM_FILTER_H
#define SOFTFM_FILTER_H

#include <vector>
#include "SoftFM.h"


/** Fine tuner which shifts the frequency of an IQ signal by a fixed offset. */
class FineTuner
{
public:

    /**
     * Construct fine tuner.
     *
     * table_size :: Size of internal sin/cos tables, determines the resolution
     *               of the frequency shift.
     *
     * freq_shift :: Frequency shift. Signal frequency will be shifted by
     *               (sample_rate * freq_shift / table_size).
     */
    FineTuner(unsigned int table_size, int freq_shift);

    /** Process samples. */
    void process(const IQSampleVector& samples_in, IQSampleVector& samples_out);

private:
    unsigned int    m_index;
    IQSampleVector  m_table;
};


/** Low-pass filter for IQ samples, based on Lanczos FIR filter. */
class LowPassFilterFirIQ
{
public:

    /**
     * Construct low-pass filter.
     *
     * filter_order :: FIR filter order.
     * cutoff       :: Cutoff frequency relative to the full sample rate
     *                 (valid range 0.0 ... 0.5).
     */
    LowPassFilterFirIQ(unsigned int filter_order, double cutoff);

    /** Process samples. */
    void process(const IQSampleVector& samples_in, IQSampleVector& samples_out);

private:
    std::vector<IQSample::value_type> m_coeff;
    IQSampleVector  m_state;
};


/**
 *  Downsampler with low-pass FIR filter for real-valued signals.
 *
 *  Step 1: Low-pass filter based on Lanczos FIR filter
 *  Step 2: (optional) Decimation by an arbitrary factor (integer or float)
 */
class DownsampleFilter
{
public:

    /**
     * Construct low-pass filter with optional downsampling.
     *
     * filter_order :: FIR filter order
     * cutoff       :: Cutoff frequency relative to the full input sample rate
     *                 (valid range 0.0 .. 0.5)
     * downsample   :: Decimation factor (>= 1) or 1 to disable
     * integer_factor :: Enables a faster and more precise algorithm that
     *                   only works for integer downsample factors.
     *
     * The output sample rate is (input_sample_rate / downsample)
     */
    DownsampleFilter(unsigned int filter_order, double cutoff,
                     double downsample=1, bool integer_factor=true);

    /** Process samples. */
    void process(const SampleVector& samples_in, SampleVector& samples_out);

private:
    double          m_downsample;
    unsigned int    m_downsample_int;
    unsigned int    m_pos_int;
    Sample          m_pos_frac;
    SampleVector    m_coeff;
    SampleVector    m_state;
};


/** First order low-pass IIR filter for real-valued signals. */
class LowPassFilterRC
{
public:

    /**
     * Construct 1st order low-pass IIR filter.
     *
     * timeconst :: RC time constant in seconds (1 / (2 * PI * cutoff_freq)
     */
    LowPassFilterRC(double timeconst);

    /** Process samples. */
    void process(const SampleVector& samples_in, SampleVector& samples_out);

    /** Process samples in-place. */
    void process_inplace(SampleVector& samples);

private:
    double  m_timeconst;
    Sample  m_y1;
};


/** Low-pass filter for real-valued signals based on Butterworth IIR filter. */
class LowPassFilterIir
{
public:

    /**
     * Construct 4th order low-pass IIR filter.
     *
     * cutoff   :: Low-pass cutoff relative to the sample frequency
     *             (valid range 0.0 .. 0.5, 0.5 = Nyquist)
     */
    LowPassFilterIir(double cutoff);

    /** Process samples. */
    void process(const SampleVector& samples_in, SampleVector& samples_out);

private:
    Sample  b0, a1, a2, a3, a4;
    Sample  y1, y2, y3, y4;
};


/** High-pass filter for real-valued signals based on Butterworth IIR filter. */
class HighPassFilterIir
{
public:

    /**
     * Construct 2nd order high-pass IIR filter.
     *
     * cutoff   :: High-pass cutoff relative to the sample frequency
     *             (valid range 0.0 .. 0.5, 0.5 = Nyquist)
     */
    HighPassFilterIir(double cutoff);

    /** Process samples. */
    void process(const SampleVector& samples_in, SampleVector& samples_out);

    /** Process samples in-place. */
    void process_inplace(SampleVector& samples);

private:
    Sample b0, b1, b2, a1, a2;
    Sample x1, x2, y1, y2;
};

#endif
