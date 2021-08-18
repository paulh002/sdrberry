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
#include "lvgl/lvgl.h"
#include "gui_top_bar.h"

lv_obj_t* bg_top;
lv_obj_t* label_status;
lv_obj_t* label_date_time;

void	setup_top_bar(lv_obj_t* scr)
{
	static lv_style_t top_style;
		
	lv_style_init(&top_style);
	lv_style_set_radius(&top_style, 0);
	lv_style_set_bg_color(&top_style, lv_palette_main(LV_PALETTE_INDIGO));
		
	bg_top = lv_obj_create(scr);
	lv_obj_add_style(bg_top, &top_style, 0);
	lv_obj_set_size(bg_top, LV_HOR_RES, topHeight);
	lv_obj_clear_flag(bg_top, LV_OBJ_FLAG_SCROLLABLE);
	
	label_status = lv_label_create(bg_top);
	//lv_label_set_long_mode(label_status, LV_LABEL_LONG_SCROLL);
	//lv_obj_set_width(label_status, LV_HOR_RES - 20);
	//lv_label_set_text(label_status, "Labelx");
	lv_obj_align(label_status, LV_ALIGN_CENTER, -220, 0);

	label_date_time = lv_label_create(bg_top);
	//lv_label_set_long_mode(label_date_time, LV_LABEL_LONG_SCROLL);
	//lv_obj_set_width(label_date_time, LV_HOR_RES - 20);
	//lv_label_set_text(label_date_time, "Label");
	lv_obj_align(label_date_time, LV_ALIGN_CENTER, 270, 8);

}

void	set_time_label()
{
	std::time_t result = std::time(nullptr);
	std::string s = std::asctime(std::localtime(&result));
	lv_label_set_text(label_date_time, s.c_str());
}