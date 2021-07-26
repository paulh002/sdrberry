#pragma once

#include "lvgl/lvgl.h"

extern const int screenWidth;
extern const int screenHeight;
extern const int bottomHeight;
extern const int topHeight;
extern const int tunerHeight;
extern const int rightWidth;

class gui_vfo
{
public:
	void gui_vfo_init(lv_obj_t* scr);
	void set_vfo_gui(int vfo, long long freq);
	
private:
	lv_style_t	text_style;
	lv_style_t	tuner_style;
	lv_obj_t*	vfo1_frequency;
	lv_obj_t*	vfo2_frequency;
	lv_obj_t*	bg_tuner1;
	lv_obj_t*	bg_tuner2;
};

extern gui_vfo	gui_vfo_inst;

