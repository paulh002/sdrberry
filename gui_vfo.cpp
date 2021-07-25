#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "wstring.h"
#include "gui_vfo.h"

LV_FONT_DECLARE(FreeSansOblique42);
LV_FONT_DECLARE(FreeSansOblique32);

gui_vfo	gui_vfo_inst;

void gui_vfo::gui_vfo_init(lv_obj_t* scr)
{
			
	lv_style_init(&tuner_style);
	lv_style_set_radius(&tuner_style, 0);
	lv_style_set_bg_color(&tuner_style, lv_color_black());
	
	bg_tuner1 = lv_obj_create(scr);
	lv_obj_add_style(bg_tuner1, &tuner_style, 0);
	lv_obj_set_pos(bg_tuner1, 0, topHeight);
	lv_obj_set_size(bg_tuner1, LV_HOR_RES / 2 - 3, tunerHeight);
	lv_obj_clear_flag(bg_tuner1, LV_OBJ_FLAG_SCROLLABLE);
		
	bg_tuner2 = lv_obj_create(scr);
	lv_obj_add_style(bg_tuner2, &tuner_style, 0);
	lv_obj_set_pos(bg_tuner2, LV_HOR_RES / 2 + 3, topHeight);
	lv_obj_set_size(bg_tuner2, LV_HOR_RES / 2 - 3, tunerHeight);
	lv_obj_clear_flag(bg_tuner2, LV_OBJ_FLAG_SCROLLABLE);
	
	lv_style_init(&text_style);

	/*Set a background color and a radius*/
	lv_style_set_radius(&text_style, 5);
	lv_style_set_bg_opa(&text_style, LV_OPA_COVER);
	lv_style_set_bg_color(&text_style, lv_color_black());
	lv_style_set_text_align(&text_style, LV_ALIGN_CENTER);
	lv_style_set_text_font(&text_style, &FreeSansOblique42);
	
	vfo1_frequency = lv_label_create(bg_tuner1);
	//lv_label_set_long_mode(vfo1_frequency, LV_LABEL_LONG_CLIP);
	lv_obj_add_style(vfo1_frequency, &text_style, 0);
	lv_obj_set_width(vfo1_frequency, LV_HOR_RES - 20);
	lv_label_set_text(vfo1_frequency, "3,500.00 Khz");
	lv_obj_set_height(vfo1_frequency, 40);
	
	vfo2_frequency = lv_label_create(bg_tuner2);
	//lv_label_set_long_mode(vfo2_frequency, LV_LABEL_LONG_CLIP);
	lv_obj_add_style(vfo2_frequency, &text_style, 0);
	lv_obj_set_width(vfo2_frequency, LV_HOR_RES - 20);
	lv_label_set_text(vfo2_frequency, "7,200.00 Khz");
	lv_obj_set_height(vfo2_frequency, 40);
}


void gui_vfo::set_vfo_gui(int vfo,unsigned long freq)
{
	char	str[20];
	
	if (freq > 10000000LU)
	{
		sprintf(str, "%3ld.%03ld,%02ld Khz", freq / 1000000, (freq / 1000) % 1000, (freq / 10) % 100);
	}
	else
	{
		sprintf(str, "%3ld.%03ld,%02ld Khz", freq / 1000000, (freq / 1000) % 1000, (freq / 10) % 100);		
	}
	if (vfo)
		lv_label_set_text(vfo2_frequency, str);
	else
		lv_label_set_text(vfo1_frequency, str);		
	printf(str);
	printf("\n");
}