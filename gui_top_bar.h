#pragma once
#include "lvgl/lvgl.h"

extern const int screenWidth;
extern const int screenHeight;
extern const int bottomHeight;
extern const int topHeight;
extern const int tunerHeight;
extern const int rightWidth;

extern lv_obj_t* bg_top;
extern lv_obj_t* label_status;

void	setup_top_bar(lv_obj_t* scr);
void	set_time_label();