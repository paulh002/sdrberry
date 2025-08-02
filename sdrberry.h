#pragma once
#include "FT891_CAT.h"
#include "AudioInput.h"
#include "SdrDevice.h"

extern int screenRotate;

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
extern std::mutex gui_mutex;

extern double ifrate;
extern double ifrate_tx;
extern SdrDeviceVector SdrDevices;
extern std::string default_radio;

void select_mode(int s_mode, bool bvfo = true, int channel = 0);
bool select_mode_tx(int s_mode, audioTone tone = audioTone::NoTone, int cattx = TX_MAN, int channel  = 0);
void destroy_demodulators(bool all = false, bool close_stream = false);
void update_filter(int bandwidth);
bool IsDigtalMode(int mode);
int IsScreenRotated();
void set_tx_buttons();
