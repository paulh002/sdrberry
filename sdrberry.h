#pragma once
#include "AMDemodulator.h"
#include "AMModulator.h"
#include "AudioInput.h"
#include "AudioOutput.h"
#include "Catinterface.h"
#include "DataBuffer.h"
#include "Filter.h"
#include "FmDecode.h"
#include "Gui_band.h"
#include "Keyboard.h"
#include "MidiControle.h"
#include "Modes.h"
#include "RtAudio.h"
#include "SdrDevice.h"
#include "Settings.h"
#include "Waterfall.h"
#include "gui_agc.h"
#include "gui_bar.h"
#include "gui_ft8.h"
#include "gui_rx.h"
#include "gui_setup.h"
#include "gui_top_bar.h"
#include "gui_tx.h"
#include "gui_vfo.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_drivers/indev/evdev.h"
#include "lvgl.h"
#include "sdrstream.h"
#include "vfo.h"
#include <cmath>
#include <complex>
#include <cstdio>
#include <cstdlib>
#include <fftw3.h>
#include <liquid/liquid.h>
#include <memory>
#include <mutex>
#include <stdint.h>
#include <sys/time.h>
#include <thread>
#include <time.h>
#include <unistd.h>
#include <vector>

extern const int barHeight;
extern const int MorseHeight;
extern lv_obj_t *bar_view;
extern lv_obj_t *tabview_mid;
extern lv_indev_t *encoder_indev_t;

extern int mode;
extern volatile int filter;

extern std::mutex fm_finish;
extern std::mutex amdemod_mutex;
extern std::mutex rxstream_mutex;
extern mutex gui_mutex;

extern double ifrate;
extern double ifrate_tx;
extern SdrDeviceVector SdrDevices;
extern std::string default_radio;
extern int default_rx_channel;
extern int default_tx_channel;

void select_mode(int s_mode, bool bvfo = true);
void select_mode_tx(int s_mode, int tone = 0);
void destroy_demodulators(bool all = false);
