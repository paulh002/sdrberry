#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "lvgl/lvgl.h"
#include "vfo.h"
#include "Gui_band.h"
#include "Settings.h"
#include <string>
#include <algorithm>
#include "Keyboard.h"


LV_FONT_DECLARE(FreeSansOblique42);
LV_FONT_DECLARE(FreeSans42);

Keyboard	keyb;

static void remove_aplha(char *p, char *s)
{
	int loop;
	for (loop = 0; loop < strlen(p); loop++)
	{	char ss[10];
		
		if (p[loop] >= '0' || p[loop] <= '9')
		{
			sprintf(ss, "%c", p[loop]);                  // printing the non numeric characters
			strcat(s, ss);
		}
	}
}


static void ta_event_cb(lv_event_t * e)
{	long long freq;
	
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * ta = (lv_obj_t *)lv_event_get_target(e);
	lv_obj_t * kb = (lv_obj_t *)lv_event_get_user_data(e);
	
	if (code == LV_EVENT_READY) {
		char	str[80];
		char *ptr = (char *)lv_textarea_get_text(ta);
		
		memset(str, 0, 80*sizeof(char));
		if (ptr[0] == '+' || ptr[0] == '-')
		{
			remove_aplha(ptr, str);
			vfo.step_vfo(atol(str), false);
		}
		if (isdigit(ptr[0]))
		{
			remove_aplha(ptr, str);
			freq = atoll(str);	
		}
		if (vfo.set_vfo(freq, false) != 0)
			return ; // error
		//sprintf(str, "%3ld.%03ld,%02ld", (long)(freq / 1000000), (long)((freq / 1000) % 1000), (long)((freq / 10) % 100));
		lv_textarea_set_text(ta, "");
	}
}

static void kb_event_cb(lv_event_t * e)
{
	long long freq;
	
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * kb = (lv_obj_t *)lv_event_get_target(e);
		
	if (code == LV_EVENT_VALUE_CHANGED) {
		uint16_t btn_id   = lv_btnmatrix_get_selected_btn(kb);
		if (btn_id == 15) //'>'
		{
			vfo.step_vfo(1,false);
		}
		
		if (btn_id == 16) //'<'
		{
			vfo.step_vfo(-1, false);
		}
		return;
	}
}
	
void Keyboard::init_keyboard(lv_obj_t *o_tab, lv_coord_t w, lv_coord_t h)
{
	lv_style_init(&text_style);

	/*Set a background color and a radius*/
	lv_style_set_radius(&text_style, 5);
	lv_style_set_bg_opa(&text_style, LV_OPA_COVER);
	lv_style_set_bg_color(&text_style, lv_color_black());
	lv_style_set_text_align(&text_style, LV_ALIGN_CENTER);
	lv_style_set_text_font(&text_style, &FreeSans42);
	
	/*Create a keyboard to use it with an of the text areas*/
	lv_obj_t *kb = lv_keyboard_create(o_tab);
	lv_keyboard_set_mode(kb, LV_KEYBOARD_MODE_NUMBER);
	lv_obj_set_size(kb, w - 40, h - 150);
	lv_obj_align(kb, LV_ALIGN_RIGHT_MID, 0, 0);
	lv_obj_add_event_cb(kb, kb_event_cb, LV_EVENT_ALL, NULL);
	
	/*Create a text area. The keyboard will write here*/
	lv_obj_t * ta;
	ta = lv_textarea_create(o_tab);
	lv_obj_align(ta, LV_ALIGN_TOP_LEFT, 10, 10);
	lv_obj_add_event_cb(ta, ta_event_cb, LV_EVENT_ALL, kb);
	lv_obj_set_size(ta, w-40, 60);
	lv_obj_add_style(ta, &text_style, 0);
	lv_textarea_set_one_line(ta, true);
		
	string s = vfo.get_vfo_str();
	lv_textarea_set_placeholder_text(ta, (char *)s.c_str());
	lv_keyboard_set_textarea(kb, ta);
	lv_obj_clear_flag(ta, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_clear_flag(o_tab, LV_OBJ_FLAG_SCROLLABLE);
}
