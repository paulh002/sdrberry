#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctime>
#include <string>
#include "lvgl.h"
#include "gui_top_bar.h"

gui_top_bar GuiTopBar;

void gui_top_bar::setup_top_bar(lv_obj_t* scr)
{	
	lv_style_init(&top_style);
	lv_style_set_radius(&top_style, 0);
	lv_style_set_bg_color(&top_style, lv_palette_main(LV_PALETTE_INDIGO));
		
	bg_top = lv_obj_create(scr);
	lv_obj_add_style(bg_top, &top_style, 0);
	lv_obj_set_size(bg_top, LV_HOR_RES, topHeight);
	lv_obj_clear_flag(bg_top, LV_OBJ_FLAG_SCROLLABLE);

	lv_style_init(&LabelTextStyle);
	lv_style_set_text_color(&LabelTextStyle, lv_color_white());
	
	label_status = lv_label_create(bg_top);
	lv_obj_add_style(label_status, &LabelTextStyle, 0);
	lv_obj_align(label_status, LV_ALIGN_LEFT_MID, 0, 0);

	lv_style_init(&TimeTextStyle);
	lv_style_set_text_color(&TimeTextStyle, lv_color_white());

	label_date_time = lv_label_create(bg_top);
	lv_obj_add_style(label_date_time, &TimeTextStyle, 0);
	lv_obj_align(label_date_time, LV_ALIGN_LEFT_MID, LV_HOR_RES - LV_HOR_RES / 3.5, 0);
}

void gui_top_bar::set_time_label()
{
	std::time_t result = std::time(nullptr);
	std::string s = std::asctime(std::localtime(&result));
	char& back = s.back();
	back = '\0';
	lv_label_set_text(label_date_time, s.c_str());
}

void gui_top_bar::set_label_status(std::string s)
{
	label = s;
	lv_label_set_text(label_status, s.c_str());
}

std::string gui_top_bar::getLabel()
{
	return label;
}