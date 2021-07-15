#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "wstring.h"
#include "lvgl/lvgl.h"
#include "gui_top_bar.h"

lv_obj_t* bg_top;
lv_obj_t* label_status;
lv_obj_t* bandwidth_dropdown;

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
	lv_label_set_long_mode(label_status, LV_LABEL_LONG_SCROLL);
	lv_obj_set_width(label_status, LV_HOR_RES - 20);
	lv_label_set_text(label_status, "Label");
	lv_obj_align(label_status, LV_ALIGN_CENTER, 20, 0);
	
	bandwidth_dropdown = lv_dropdown_create(bg_top);
	//lv_obj_align(bandwidth_dropdown, LV_ALIGN_TOP_RIGHT, 0, 0);
	lv_obj_set_pos(bandwidth_dropdown, LV_HOR_RES - 200, -15);
}

void set_bandwidth_dropdown(double *bandwidth, int count)
{
	lv_dropdown_clear_options(bandwidth_dropdown);
	for (int i = 0; i < count; i++)
	{
		char buf[10];
		
		sprintf(buf, "%.0f Khz", bandwidth[i] / 1000);
		lv_dropdown_add_option(bandwidth_dropdown, buf, LV_DROPDOWN_POS_LAST);
	}
}
