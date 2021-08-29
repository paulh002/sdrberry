#pragma once
#include "AudioInput.h"
#include "lvgl/lvgl.h"

extern const int screenWidth;
extern const int screenHeight;
extern const int bottomHeight;
extern const int topHeight;
extern const int tunerHeight;
extern const int rightWidth;


void gui_tx_init(lv_obj_t* o_tab, lv_coord_t w);
void set_mic_slider(int volume);
void step_mic_slider(int step);
void set_drv_slider(int drive);
void drv_mic_slider(int step);
void set_tx_state(bool state);
void set_drv_range();