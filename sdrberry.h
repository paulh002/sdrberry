#pragma once
#include <complex>
#include <vector>
#include <mutex>
#include <memory>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <liquid.h>
#include <unistd.h>
#include <pthread.h>
#include "RtAudio.h"
#include "DataBuffer.h"
#include "Audiodefs.h"
#include "AudioOutput.h"
#include "AudioInput.h"
#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include "devices.h"
#include "gui_right_pane.h"
#include "gui_top_bar.h"
#include "gui_tx.h"
#include "gui_rx.h"
#include "ble_interface.h"
#include "vfo.h"
#include "sdrstream.h"
#include "gui_vfo.h"
#include "Settings.h"
#include "Gui_band.h"
#include "Keyboard.h"
#include "Filter.h"
#include "FmDecode.h"
#include "Waterfall.h"
#include "AMDemodulator.h"
#include "AMmodulator.h"
#include "MidiControle.h"

const int mode_lsb = 1;
const int mode_usb = 2;
const int mode_am = 3;
const int mode_dsb = 4;
const int mode_cw = 5;
const int mode_ft8 = 6;
const int mode_ft4 = 7;
const int mode_broadband_fm = 8;
const int mode_narrowband_fm = 9;
const int mode_rtty = 10;

extern int		  mode;
extern volatile int		filter;

extern std::mutex	am_finish;
extern std::mutex	am_tx_finish;
extern std::mutex	fm_finish;
extern std::mutex	stream_finish;

extern atomic_bool stop_flag;
extern atomic_bool stop_tx_flag;
extern atomic_bool stop_txmod_flag;


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