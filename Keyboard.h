#pragma once
#include "lvgl/lvgl.h"

class Keyboard
{
public:
	void init_keyboard(lv_obj_t *o_tab, lv_coord_t w, lv_coord_t h);

private:
	lv_style_t	text_style; 
};

extern Keyboard	keyb;