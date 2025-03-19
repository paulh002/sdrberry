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
#include "cmeter.h"
#include "Modes.h"
#include "screen.h"
#include "Settings.h"
#include "VfoKeyPad.h"

LV_FONT_DECLARE(FreeSansOblique42);
LV_FONT_DECLARE(FreeSansOblique32);

gui_vfo gui_vfo_inst;

void gui_vfo::gui_vfo_init(lv_obj_t *scr)
{

	lv_style_init(&tuner_style);
	lv_style_set_radius(&tuner_style, 0);
	lv_style_set_bg_color(&tuner_style, lv_color_black());
	lv_style_set_bg_opa(&tuner_style, LV_OPA_100);
	
	bg_tuner1 = lv_obj_create(scr);
	lv_obj_add_style(bg_tuner1, &tuner_style, 0);
	lv_obj_set_pos(bg_tuner1, 0, topHeight);
	lv_obj_set_size(bg_tuner1, 2 * (LV_HOR_RES / 6) - 3, tunerHeight);
	lv_obj_clear_flag(bg_tuner1, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_event_cb(bg_tuner1, bg_tuner1_clickevent_cb, LV_EVENT_CLICKED, (void *)this);

	bg_tuner2 = lv_obj_create(scr);
	lv_obj_add_style(bg_tuner2, &tuner_style, 0);
	lv_obj_set_pos(bg_tuner2, 2 * (LV_HOR_RES / 6) , topHeight);
	lv_obj_set_size(bg_tuner2, 2 * (LV_HOR_RES / 6) - 3, tunerHeight);
	lv_obj_clear_flag(bg_tuner2, LV_OBJ_FLAG_SCROLLABLE);

	lv_obj_t *bg_tuner3 = lv_obj_create(scr);
	lv_obj_add_style(bg_tuner3, &tuner_style, 0);
	lv_obj_set_pos(bg_tuner3, 4 * (LV_HOR_RES / 6) , topHeight);
	lv_obj_set_size(bg_tuner3, 2 * (LV_HOR_RES / 6) , tunerHeight);
	lv_obj_clear_flag(bg_tuner3, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_pad_hor(bg_tuner3, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(bg_tuner3, 0, LV_PART_MAIN);
	set_smeter_img(bg_tuner3, 0, 0, 2 * (LV_HOR_RES / 6) - 3, tunerHeight);

	/*Set a background color and a radius*/
	lv_style_init(&text_style);
	lv_style_set_radius(&text_style, 5);
	lv_style_set_bg_opa(&text_style, LV_OPA_COVER);
	lv_style_set_bg_color(&text_style, lv_color_black());
	lv_style_set_text_align(&text_style, LV_ALIGN_CENTER);
	lv_style_set_text_font(&text_style, &FreeSansOblique42);

	lv_style_init(&label_style);
	lv_style_set_radius(&label_style, 5);
	lv_style_set_bg_opa(&label_style, LV_OPA_COVER);
	lv_style_set_bg_color(&label_style, lv_color_black());
	lv_style_set_text_color(&label_style, lv_palette_main(LV_PALETTE_BLUE));
	lv_style_set_text_align(&label_style, LV_ALIGN_CENTER);

	vfo1_frequency = lv_label_create(bg_tuner1);
	//lv_label_set_long_mode(vfo1_frequency, LV_LABEL_LONG_CLIP);
	lv_obj_add_style(vfo1_frequency, &text_style, 0);
	lv_obj_set_width(vfo1_frequency, LV_HOR_RES - 20);
	lv_label_set_text(vfo1_frequency, "3.500.00");
	lv_obj_set_height(vfo1_frequency, 40);

	band_label = lv_label_create(bg_tuner1);
	lv_label_set_text(band_label, "1800 m ");
	lv_obj_align_to(band_label, vfo1_frequency, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

	mode_label = lv_label_create(bg_tuner1);
	lv_label_set_text(mode_label, "LSB ");
	lv_obj_align_to(mode_label, vfo1_frequency, LV_ALIGN_OUT_BOTTOM_LEFT, 50, 10);

	span_label = lv_label_create(bg_tuner1);
	lv_label_set_text(span_label, "100 Khz ");
	lv_obj_align_to(span_label, vfo1_frequency, LV_ALIGN_OUT_BOTTOM_LEFT, 100, 10);

	rxtx_label = lv_label_create(bg_tuner1);
	lv_label_set_text(rxtx_label, "RX");
	lv_obj_align_to(rxtx_label, vfo1_frequency, LV_ALIGN_OUT_BOTTOM_LEFT, 200, 10);
	lv_obj_add_style(rxtx_label, &label_style, 0);
	lv_label_set_recolor(rxtx_label, true);

	vfo2_frequency = lv_label_create(bg_tuner2);
	//lv_label_set_long_mode(vfo2_frequency, LV_LABEL_LONG_CLIP);
	lv_obj_add_style(vfo2_frequency, &text_style, 0);
	lv_obj_set_width(vfo2_frequency, LV_HOR_RES - 20);
	lv_label_set_text(vfo2_frequency, "7.200.00");
	lv_obj_set_height(vfo2_frequency, 40);

	band_label2 = lv_label_create(bg_tuner2);
	lv_label_set_text(band_label2, "1800 m ");
	lv_obj_align_to(band_label2, vfo2_frequency, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

	mode_label2 = lv_label_create(bg_tuner2);
	lv_label_set_text(mode_label2, "LSB ");
	lv_obj_align_to(mode_label2, vfo2_frequency, LV_ALIGN_OUT_BOTTOM_LEFT, 50, 10);

	mode_split2 = lv_label_create(bg_tuner2);
	lv_label_set_text(mode_split2, "#00ff00 Split#");
	lv_obj_align_to(mode_split2, vfo2_frequency, LV_ALIGN_OUT_BOTTOM_LEFT, 100, 10);
	lv_label_set_recolor(mode_split2, true);

	rxtx_label2 = lv_label_create(bg_tuner2);
	lv_label_set_text(rxtx_label2, "RX");
	lv_obj_align_to(rxtx_label2, vfo2_frequency, LV_ALIGN_OUT_BOTTOM_LEFT, 200, 10);
	lv_obj_add_style(rxtx_label2, &label_style, 0);
	lv_label_set_recolor(rxtx_label2, true);

	smeter_delay = Settings_file.get_int("Radio", "s-meter-delay", 0);
}

void gui_vfo::set_vfo_gui(int active_vfo, long long freq, int vfo_rx, int vfo_mode_no, int vfo_band)
{
	char str[30];

	if (freq > 10000000LU)
	{
		sprintf(str, "%3ld.%03ld,%02ld", (long)(freq / 1000000), (long)((freq / 1000) % 1000), (long)((freq / 10) % 100));
	}
	else
	{
		sprintf(str, "%3ld.%03ld,%02ld", (long)(freq / 1000000), (long)((freq / 1000) % 1000), (long)((freq / 10) % 100));
	}

	if (active_vfo)
	{
		lv_label_set_text(vfo2_frequency, str);
	}
	else
	{
		lv_label_set_text(vfo1_frequency, str);
	}
	sprintf(str, "%d m", vfo_band);
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
				lv_label_set_text(rxtx_label, "RX");
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
		switch (mode[active_vfo])
		{
		case mode_broadband_fm:
			strcpy(str, "FM");
			break;
		case mode_lsb:
			strcpy(str, "LSB");
			break;
		case mode_usb:
			strcpy(str, "USB");
			break;
		case mode_dsb:
			strcpy(str, "DSB");
		case mode_am:
			strcpy(str, "AM");
			break;
		case mode_cw:
			strcpy(str, "CW");
			break;
		case mode_ft8:
			strcpy(str, "FT8");
			break;
		}
		if (active_vfo)
			lv_label_set_text(mode_label2, str);
		else
			lv_label_set_text(mode_label, str);
	}
	if (split)
		lv_label_set_text(mode_split2, "#00ff00 Split#");
	else
		lv_label_set_text(mode_split2, "");
}

void gui_vfo::set_smeter_value(int32_t v)
{
	if (cmeter_ptr != nullptr)
		cmeter_ptr->lv_meter_set_indicator_value((lv_meter_indicator_c *)smeter_indic, v);
}

void gui_vfo::smeter_event_cb_class(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_draw_part_dsc_t *dsc = (lv_obj_draw_part_dsc_t *)lv_event_get_param(e);
	lv_meter_c *meter = (lv_meter_c *)obj;
	
	if (code == LV_EVENT_DRAW_PART_BEGIN && (lv_meter_scale_c *)dsc->sub_part_ptr == scale)
	{
		dsc->value = dsc->value / 10 - 1;
		if (dsc->value > 9)
		{
			if (dsc->value == 10)
				dsc->value = 20;
			if (dsc->value == 11)
				dsc->value = 40;
			if (dsc->value == 12)
				dsc->value = 60;
		}
		switch (dsc->value)
		{
		case 0:
			strcpy(dsc->text, "S");
			break;
		case 2:
		//case 3:
		case 4:
		case 6:
		//case 7:
		case 8:
		case 20:
			strcpy(dsc->text, "");
			break;

		case 40:
		case 60:
			lv_snprintf(dsc->text, sizeof(dsc->text), "+%d", dsc->value);
			break;

		default:
			lv_snprintf(dsc->text, sizeof(dsc->text), "%d", dsc->value);
			break;
		}
	}
	if (code == LV_EVENT_DRAW_PART_BEGIN && (lv_meter_scale_c *)dsc->sub_part_ptr == scale2)
	{
		if (dsc->value == 0)
			strcpy(dsc->text, "Po");
	}
}

void gui_vfo::set_smeter_img(lv_obj_t *box, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h)
{
	const int center_modifier = 150;
	const int arc_angle_range = 48;
	const int arc_rotation = 246;

	cmeter_ptr = std::make_unique<cmeter>(box, x, y, w, h);
	/*Add a scale first*/
	scale = cmeter_ptr->lv_meter_add_scale(0, center_modifier);
	cmeter_ptr->lv_obj_add_event_cb(smeter_event_cb, LV_EVENT_DRAW_PART_BEGIN, (void *)this);

	//lv_meter_set_scale_range(meter, scale, 10, 120, 100, 220);
	cmeter_ptr->lv_meter_set_scale_range(scale, 10, 120, arc_angle_range, arc_rotation);
	cmeter_ptr->lv_meter_set_scale_ticks(scale, 12, 10, 5, lv_palette_main(LV_PALETTE_LIGHT_GREEN));
	cmeter_ptr->lv_meter_set_scale_major_ticks(scale, 1, 2, 10, lv_color_hex3(0xeee), -20);

	//lv_meter_set_scale_major_ticks(meter, scale, 1, 4, 10, lv_color_hex3(0xeee), 10);
	const int arc_start_value = 0;
	const int arc_first_sector = 90;
	const int arc_second_sector = 120;

	lv_meter_indicator_c *indic;
	indic = cmeter_ptr->lv_meter_add_arc(scale, 3, lv_palette_main(LV_PALETTE_GREEN), -5, center_modifier); // -5
	cmeter_ptr->lv_meter_set_indicator_start_value(indic, arc_start_value);
	cmeter_ptr->lv_meter_set_indicator_end_value(indic, arc_first_sector);

	lv_meter_indicator_c *indic1;
	indic1 = cmeter_ptr->lv_meter_add_arc(scale, 3, lv_palette_main(LV_PALETTE_RED), -5, center_modifier);
	cmeter_ptr->lv_meter_set_indicator_start_value(indic1, arc_first_sector);
	cmeter_ptr->lv_meter_set_indicator_end_value(indic1, arc_second_sector);

	lv_meter_indicator_c *indic2;
	indic2 = cmeter_ptr->lv_meter_add_arc(scale, 3, lv_color_white(), -10, center_modifier);
	cmeter_ptr->lv_meter_set_indicator_start_value(indic2, arc_start_value);
	cmeter_ptr->lv_meter_set_indicator_end_value(indic2, arc_second_sector);

	const int scale_2 = -33;

	lv_meter_indicator_c *indic3;
	indic3 = cmeter_ptr->lv_meter_add_arc(scale, 3, lv_color_white(), scale_2 -7, center_modifier);
	cmeter_ptr->lv_meter_set_indicator_start_value(indic3, arc_start_value);
	cmeter_ptr->lv_meter_set_indicator_end_value(indic3, arc_second_sector);

	lv_meter_indicator_c *indic4;
	indic4 = cmeter_ptr->lv_meter_add_arc(scale, 3, lv_palette_main(LV_PALETTE_RED), scale_2 -3, center_modifier);
	cmeter_ptr->lv_meter_set_indicator_start_value(indic4, arc_first_sector);
	cmeter_ptr->lv_meter_set_indicator_end_value(indic4, arc_second_sector);

	lv_meter_indicator_c *indic5;
	indic5 = cmeter_ptr->lv_meter_add_arc(scale, 3, lv_palette_main(LV_PALETTE_BLUE), scale_2 -3, center_modifier);
	cmeter_ptr->lv_meter_set_indicator_start_value(indic5, arc_start_value);
	cmeter_ptr->lv_meter_set_indicator_end_value(indic5, arc_first_sector);

	scale2 = cmeter_ptr->lv_meter_add_scale(scale_2, center_modifier);
	//lv_meter_set_scale_range(meter, scale, 10, 120, 100, 220);
	cmeter_ptr->lv_meter_set_scale_range(scale2, 0, 10, arc_angle_range, arc_rotation);
	cmeter_ptr->lv_meter_set_scale_major_ticks(scale2, 1, 2, 10, lv_color_hex3(0xeee), -20);

	smeter_indic = cmeter_ptr->lv_meter_add_needle_line(scale, 3, lv_palette_main(LV_PALETTE_RED), 5);
	cmeter_ptr->lv_meter_set_indicator_value(smeter_indic, 0);
}

void gui_vfo::set_s_meter(double value)
{
	//printf("value %f\n", value);
	//value = 30.0 + value;
	//value = value + 200.0;
	//printf(" value s%f \n", value);

	uint32_t s_value = value;
	switch (smeter_delay)
	{
	case 1:
		s_value = smeter2(value);
		break;
	case 2:
		s_value = smeter4(value);
		break;
	case 3:
		s_value = smeter6(value);
		break;
	case 4:
		s_value = smeter8(value);
		break;	
	}
	// cmeter_ptr->lv_meter_set_indicator_value(smeter_indic, value);
	cmeter_ptr->lv_meter_set_indicator_value(smeter_indic, s_value);
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
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_CLICKED)
	{
		CreateVfoKeyPadWindow(lv_scr_act());
	}
}