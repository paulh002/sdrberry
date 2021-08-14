#pragma once
#include <complex>
#include <vector>
#include <mutex>
#include "RtAudio.h"
#include "DataBuffer.h"
#include "Audiodefs.h"
#include "AudioOutput.h"
#include "AudioInput.h"

const int mode_broadband_fm = 0;
const int mode_lsb = 1;
const int mode_usb = 2;
const int mode_am = 3;
const int mode_dsb = 4;
const int mode_cw = 5;
const int mode_ft8 = 6;
extern int		  mode;
extern volatile int		filter;

extern std::mutex	am_finish;
extern std::mutex	am_tx_finish;
extern std::mutex	fm_finish;
extern std::mutex	stream_finish;

//double	freq = 89950000;
extern double   ifrate;

/** Compute mean and RMS over a sample vector. */
inline void samples_mean_rms(const SampleVector& samples,
                             double& mean, double& rms)
{
    Sample vsum = 0;
    Sample vsumsq = 0;

    unsigned int n = samples.size();
    for (unsigned int i = 0; i < n; i++) {
        Sample v = samples[i];
        vsum   += v;
        vsumsq += v * v;
    }

    mean = vsum / n;
    rms  = sqrt(vsumsq / n);
}

void select_filter(int ifilter);
void select_mode(int s_mode);
void select_mode_tx(int s_mode);