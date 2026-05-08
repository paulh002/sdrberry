#include "gui_vfo.h"
#include <math.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "meter.h"
#include "Modes.h"
#include "screen.h"
#include "Settings.h"
#include "VfoKeyPad.h"

LV_FONT_DECLARE(FreeSansOblique42);
LV_FONT_DECLARE(FreeSansOblique58);
LV_FONT_DECLARE(FreeSansOblique72);

gui_vfo gui_vfo_inst;

void gui_vfo::gui_vfo_init(lv_obj_t *scr, int x, int y, int w, int h, lv_group_t *keyboard_group)
{
	keyboardgroup = keyboard_group;
	lv_style_init(&tuner_style);
	lv_style_set_radius(&tuner_style, 0);
	lv_style_set_bg_color(&tuner_style, lv_color_black());
	lv_style_set_bg_opa(&tuner_style, LV_OPA_100);

	int bg_tuner_size = 2 * (w / 6);
	bg_tuner1 = lv_obj_create(scr);
	lv_obj_add_style(bg_tuner1, &tuner_style, 0);
	lv_obj_set_pos(bg_tuner1, x, y);
	lv_obj_set_size(bg_tuner1, bg_tuner_size - 3, h);
	lv_obj_clear_flag(bg_tuner1, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_event_cb(bg_tuner1, bg_tuner1_clickevent_cb, LV_EVENT_CLICKED, (void *)this);

	bg_tuner2 = lv_obj_create(scr);
	lv_obj_add_style(bg_tuner2, &tuner_style, 0);
	lv_obj_set_pos(bg_tuner2, bg_tuner_size , y);
	lv_obj_set_size(bg_tuner2, bg_tuner_size - 3, h);
	lv_obj_clear_flag(bg_tuner2, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_event_cb(bg_tuner2, bg_tuner1_clickevent_cb, LV_EVENT_CLICKED, (void *)this);

	lv_obj_t *bg_tuner3 = lv_obj_create(scr);
	lv_obj_add_style(bg_tuner3, &tuner_style, 0);
	lv_obj_set_pos(bg_tuner3, 2 * bg_tuner_size , y);
	lv_obj_set_size(bg_tuner3, bg_tuner_size , h);
	lv_obj_clear_flag(bg_tuner3, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_pad_hor(bg_tuner3, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(bg_tuner3, 0, LV_PART_MAIN);
	smeter.init(bg_tuner3, 0, 0, bg_tuner_size - 3, h);

	/*Set a background color and a radius*/
	int32_t line_height;
	vfo1_frequency = lv_label_create(bg_tuner1);
	if (screenWidth > screenfontthresshold_2)
	{
		line_height = lv_font_get_line_height(&FreeSansOblique72) + 4;
		lv_obj_set_style_text_font(vfo1_frequency, &FreeSansOblique72, 0);
	}
	else if (screenWidth > screenfontthresshold_1)
	{
		line_height = lv_font_get_line_height(&FreeSansOblique58) + 4;
		lv_obj_set_style_text_font(vfo1_frequency, &FreeSansOblique58, 0);
	}
	else
	{
		line_height = lv_font_get_line_height(&FreeSansOblique42) + 4;
		lv_obj_set_style_text_font(vfo1_frequency, &FreeSansOblique42, 0);
	}

		// lv_obj_set_width(vfo1_frequency, w - 20);
		lv_obj_set_height(vfo1_frequency, line_height);
	lv_label_set_text(vfo1_frequency, "3.500.00");
	lv_obj_set_style_text_opa(vfo1_frequency, LV_OPA_COVER, 0);
	lv_obj_set_style_text_color(vfo1_frequency, lv_color_white(), 0);

	int pad_top = lv_obj_get_style_pad_top(bg_tuner1, LV_PART_MAIN);
	int pad_bottom = lv_obj_get_style_pad_bottom(bg_tuner1, LV_PART_MAIN);
	lv_obj_set_pos(vfo1_frequency, 0, (10 * (h - pad_top - pad_bottom) / 25) - (line_height / 2));
	
	int32_t label_height = lv_font_get_line_height(LV_FONT_DEFAULT) + 4;
	band_label = lv_label_create(bg_tuner1);
	lv_label_set_text(band_label, "XXXXXX");
	lv_obj_set_height(band_label, label_height);
	lv_obj_set_pos(band_label, (1 * bg_tuner_size) / 100, h - (label_height * 2));
	lv_obj_set_style_text_opa(band_label, LV_OPA_COVER, 0);
	lv_obj_set_style_text_align(band_label, LV_TEXT_ALIGN_CENTER, 0);

	mode_label = lv_label_create(bg_tuner1);
	lv_label_set_text(mode_label, "XXXXXX");
	lv_obj_set_height(mode_label, label_height);
	lv_obj_align_to(mode_label, band_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
	lv_obj_set_style_text_opa(mode_label, LV_OPA_COVER, 0);
	lv_obj_set_style_text_align(mode_label, LV_TEXT_ALIGN_CENTER, 0);

	span_label = lv_label_create(bg_tuner1);
	lv_label_set_text(span_label, "XXXXXXXXXXXX");
	lv_obj_set_height(span_label, label_height);
	lv_obj_align_to(span_label, mode_label, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
	lv_obj_set_style_text_opa(span_label, LV_OPA_COVER, 0);
	lv_obj_set_style_text_align(span_label, LV_TEXT_ALIGN_CENTER, 0);

	rxtx_label = lv_label_create(bg_tuner1);
	lv_label_set_text(rxtx_label, "RX");
	lv_obj_set_height(rxtx_label, label_height);
	lv_obj_set_style_text_align(rxtx_label, LV_TEXT_ALIGN_RIGHT, 0);
	lv_obj_set_pos(rxtx_label, (80 * bg_tuner_size) / 100, h - (label_height * 2));
	lv_label_set_recolor(rxtx_label, true);
	lv_obj_set_style_text_color(rxtx_label, lv_palette_main(LV_PALETTE_BLUE), 0);
	lv_obj_set_style_text_opa(rxtx_label, LV_OPA_COVER, 0);
	lv_obj_set_style_text_align(rxtx_label, LV_TEXT_ALIGN_CENTER, 0);

	vfo2_frequency = lv_label_create(bg_tuner2);
	if (screenWidth > screenfontthresshold_2)
	{
		line_height = lv_font_get_line_height(&FreeSansOblique72) + 4;
		lv_obj_set_style_text_font(vfo2_frequency, &FreeSansOblique72, 0);
	}
	else if (screenWidth > screenfontthresshold_1)
	{
		line_height = lv_font_get_line_height(&FreeSansOblique58) + 4;
		lv_obj_set_style_text_font(vfo2_frequency, &FreeSansOblique58, 0);
	}
	else
	{
		line_height = lv_font_get_line_height(&FreeSansOblique42) + 4;
		lv_obj_set_style_text_font(vfo2_frequency, &FreeSansOblique42, 0);
	}
	lv_obj_set_width(vfo2_frequency, w - 20);
	lv_label_set_text(vfo2_frequency, "7.200.00");
	lv_obj_set_height(vfo2_frequency, line_height);
	lv_obj_set_style_text_opa(vfo2_frequency, LV_OPA_COVER, 0);
	lv_obj_set_style_text_color(vfo2_frequency, lv_color_hex(0x90A4AE), 0);
	lv_obj_set_pos(vfo2_frequency, 0, (10 * (h - pad_top - pad_bottom) / 25) - (line_height / 2));

	band_label2 = lv_label_create(bg_tuner2);
	lv_label_set_text(band_label2, "XXXXXX");
	lv_obj_set_height(band_label2, label_height);
	lv_obj_set_pos(band_label2, (1 * bg_tuner_size) / 100, h - (label_height * 2));
	lv_obj_set_style_text_opa(band_label2, LV_OPA_COVER, 0);
	lv_obj_set_style_text_align(band_label2, LV_TEXT_ALIGN_CENTER, 0);

	mode_label2 = lv_label_create(bg_tuner2);
	lv_label_set_text(mode_label2, "XXXXXX");
	lv_obj_set_height(mode_label2, label_height);
	lv_obj_align_to(mode_label2, band_label2, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
	lv_obj_set_style_text_opa(mode_label2, LV_OPA_COVER, 0);
	lv_obj_set_style_text_align(mode_label2, LV_TEXT_ALIGN_CENTER, 0);

	mode_split2 = lv_label_create(bg_tuner2);
	lv_label_set_text(mode_split2, "#00ff00 Split#");
	lv_obj_set_height(mode_split2, label_height);
	lv_obj_align_to(mode_split2, mode_label2, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
	lv_obj_set_style_text_opa(mode_split2, LV_OPA_COVER, 0);
	lv_obj_set_style_text_align(mode_split2, LV_TEXT_ALIGN_CENTER, 0);
	lv_label_set_recolor(mode_split2, true);

	rxtx_label2 = lv_label_create(bg_tuner2);
	lv_label_set_text(rxtx_label2, "RX");
	lv_obj_set_height(rxtx_label2, label_height);
	lv_obj_set_style_text_align(rxtx_label2, LV_TEXT_ALIGN_RIGHT, 0);
	lv_obj_set_pos(rxtx_label2, (80 * bg_tuner_size) / 100, h - (label_height * 2));
	lv_obj_set_style_text_opa(rxtx_label2, LV_OPA_COVER, 0);
	lv_obj_set_style_text_align(rxtx_label2, LV_TEXT_ALIGN_CENTER, 0);
	lv_label_set_recolor(rxtx_label2, true);
	lv_obj_set_style_text_color(rxtx_label2, lv_palette_main(LV_PALETTE_BLUE), 0);

	smeter_delay = Settings_file.get_int("Radio", "s-meter-delay", 25);
	smeter_filter = std::make_unique<SMeterFilter>(20.0f, 25.0f, 280.0f); // SSB defaults
}

void gui_vfo::set_smeter_delay(int delay)
{
	switch (delay)
	{
	case 0:
		smeter_filter->configure(30.0f, 12.5f, 140.0f);
		break;
	case 1:
		smeter_filter->configure(30.0f, 25.0f, 280.0f);
		break;
	case 2:
		smeter_filter->configure(30.0f, 37.5f, 420.0f);
		break;
	case 3:
		smeter_filter->configure(30.0f, 50.0f, 560.0f);
		break;
	case 4:
		smeter_filter->configure(30.0f, 75.0f, 840.0f);
		break;
	}
}

void gui_vfo::set_vfo_freq(std::string freq_str, int selected_vfo)
{
	if (selected_vfo)
	{
		lv_label_set_text(vfo2_frequency, freq_str.c_str());
		//lv_obj_set_style_text_color(vfo2_frequency, lv_color_white(), 0);
		//lv_obj_set_style_text_color(vfo1_frequency, lv_color_hex(0x90A4AE), 0);
	}
	else
	{
		lv_label_set_text(vfo1_frequency, freq_str.c_str());
		//lv_obj_set_style_text_color(vfo1_frequency, lv_color_white(), 0);
		//lv_obj_set_style_text_color(vfo2_frequency, lv_color_hex(0x90A4AE), 0);
	}
}

void gui_vfo::set_vfo_gui(int active_vfo, long freq, int vfo_rx, int vfo_mode_no, int vfo_band, int vfo_band_index)
{
	char str[30];

	if (freq > 100000000LU)
	{
		sprintf(str, "%3ld.%03ld.%02ld", (long)(freq / 1000000), (long)((freq / 1000) % 1000), (long)((freq / 10) % 100));
	}
	else
	{
		sprintf(str, "%3ld.%03ld.%03ld", (long)(freq / 1000000), (long)((freq / 1000) % 1000), (long)((freq) % 1000));
	}

	if (active_vfo)
	{
		lv_label_set_text(vfo2_frequency, str);
		lv_obj_set_style_text_color(vfo2_frequency, lv_color_white(), 0);
		lv_obj_set_style_text_color(vfo1_frequency, lv_color_hex(0x90A4AE), 0);
	}
	else
	{
		lv_label_set_text(vfo1_frequency, str);
		lv_obj_set_style_text_color(vfo1_frequency, lv_color_white(), 0);
		lv_obj_set_style_text_color(vfo2_frequency, lv_color_hex(0x90A4AE), 0);
	}
	
	sprintf(str, "%d %s", vfo_band, Settings_file.labels.at(vfo_band_index).c_str());
	if (active_vfo)
		lv_label_set_text(band_label2, str);
	else
		lv_label_set_text(band_label, str);

	if (vfo_rx!= rxtx)
	{
		rxtx = vfo_rx;
		if (rxtx)
		{
			if (active_vfo)
				lv_label_set_text(rxtx_label2, "RX");
			else
			{
				lv_label_set_text(rxtx_label, "RX");
				lv_label_set_text(rxtx_label2, "RX");
			}
		}
		else
		{
			if (active_vfo)
				lv_label_set_text(rxtx_label2, "#ff0000 TX#");
			else
				lv_label_set_text(rxtx_label, "#ff0000 TX#");
		}
	}

	if (mode[active_vfo] != vfo_mode_no)
	{
		mode[active_vfo] = vfo_mode_no;
		std::string mode_str = mode_string(mode[active_vfo]);
		if (active_vfo)
			lv_label_set_text(mode_label2, mode_str.c_str());
		else
			lv_label_set_text(mode_label, mode_str.c_str());
	}
	if (split)
		lv_label_set_text(mode_split2, "#00ff00 Split#");
	else
		lv_label_set_text(mode_split2, "");
}

void gui_vfo::set_s_meter(float value)
{
	float smoothed = smeter_filter->process(value);
	int32_t gauge_val = static_cast<int32_t>(std::clamp(smoothed, 0.0f, 100.0f));
	smeter.set_needle(gauge_val);
	//printf("value %f smoothed %f gauge %d\n", value, smoothed, gauge_val);
}

void gui_vfo::set_span(int span)
{
	char str[30];

	sprintf(str, "%d Khz", span);
	lv_label_set_text(span_label, str);
}

void gui_vfo::set_split(bool _split)
{
	split = _split;
	if (split)
		lv_label_set_text(mode_split2, "#00ff00 Split#");
	else
		lv_label_set_text(mode_split2, "");
}

bool gui_vfo::get_split()
{
	return split;
}

void gui_vfo::bg_tuner1_clickevent_cb_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
	if (code == LV_EVENT_CLICKED)
	{
		CreateVfoKeyPadWindow(get_main_screen(), keyboardgroup);
	}
}