#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include "vfo.h"
#include "gui_vfo.h"

LV_FONT_DECLARE(FreeSansOblique42);
LV_FONT_DECLARE(FreeSansOblique32);

void set_smeter_img(lv_obj_t* box, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h);
gui_vfo	gui_vfo_inst;

void gui_vfo::gui_vfo_init(lv_obj_t* scr)
{
			
	lv_style_init(&tuner_style);
	lv_style_set_radius(&tuner_style, 0);
	lv_style_set_bg_color(&tuner_style, lv_color_black());
	
	bg_tuner1 = lv_obj_create(scr);
	lv_obj_add_style(bg_tuner1, &tuner_style, 0);
	lv_obj_set_pos(bg_tuner1, 0, topHeight);
	lv_obj_set_size(bg_tuner1, 2*(LV_HOR_RES / 6) - 3, tunerHeight);
	lv_obj_clear_flag(bg_tuner1, LV_OBJ_FLAG_SCROLLABLE);
		
	bg_tuner2 = lv_obj_create(scr);
	lv_obj_add_style(bg_tuner2, &tuner_style, 0);
	lv_obj_set_pos(bg_tuner2, 2*(LV_HOR_RES / 6) + 3, topHeight);
	lv_obj_set_size(bg_tuner2, 2*(LV_HOR_RES / 6) - 3, tunerHeight);
	lv_obj_clear_flag(bg_tuner2, LV_OBJ_FLAG_SCROLLABLE);
	
	/*bg_smeter = lv_obj_create(scr);
	lv_obj_add_style(bg_smeter, &tuner_style, 0);
	lv_obj_set_pos(bg_smeter, 4*(LV_HOR_RES / 6) + 3, topHeight);
	lv_obj_set_size(bg_smeter, 2*(LV_HOR_RES / 6) - 3, tunerHeight);
	lv_obj_clear_flag(bg_smeter, LV_OBJ_FLAG_SCROLLABLE);
	*/
	set_smeter_img(scr, 4*(LV_HOR_RES / 6) + 3, topHeight, 2*(LV_HOR_RES / 6) - 3, tunerHeight);
		
	lv_style_init(&text_style);

	/*Set a background color and a radius*/
	lv_style_set_radius(&text_style, 5);
	lv_style_set_bg_opa(&text_style, LV_OPA_COVER);
	lv_style_set_bg_color(&text_style, lv_color_black());
	lv_style_set_text_align(&text_style, LV_ALIGN_CENTER);
	lv_style_set_text_font(&text_style, &FreeSansOblique42);
	
	lv_style_set_radius(&label_style, 5);
	lv_style_set_bg_opa(&label_style, LV_OPA_COVER);
	lv_style_set_bg_color(&label_style, lv_color_black());
	lv_style_set_text_color(&label_style, lv_palette_main(LV_PALETTE_BLUE));
	lv_style_set_text_align(&label_style, LV_ALIGN_CENTER);
	
	vfo1_frequency = lv_label_create(bg_tuner1);
	//lv_label_set_long_mode(vfo1_frequency, LV_LABEL_LONG_CLIP);
	lv_obj_add_style(vfo1_frequency, &text_style, 0);
	lv_obj_set_width(vfo1_frequency, LV_HOR_RES - 20);
	lv_label_set_text(vfo1_frequency, "3,500.00");
	lv_obj_set_height(vfo1_frequency, 40);
	
	band_label = lv_label_create(bg_tuner1);
	lv_label_set_text(band_label, "1800 m ");
	lv_obj_align_to(band_label, vfo1_frequency, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
	
	mode_label = lv_label_create(bg_tuner1);
	lv_label_set_text(mode_label, "LSB ");
	lv_obj_align_to(mode_label, vfo1_frequency, LV_ALIGN_OUT_BOTTOM_LEFT, 50, 10);
	
	rxtx_label = lv_label_create(bg_tuner1);
	lv_label_set_text(rxtx_label, "RX");
	lv_obj_align_to(rxtx_label, vfo1_frequency, LV_ALIGN_OUT_BOTTOM_LEFT, 200, 10);
	lv_obj_add_style(rxtx_label, &label_style, 0);
	lv_label_set_recolor(rxtx_label, true);
	
	vfo2_frequency = lv_label_create(bg_tuner2);
	//lv_label_set_long_mode(vfo2_frequency, LV_LABEL_LONG_CLIP);
	lv_obj_add_style(vfo2_frequency, &text_style, 0);
	lv_obj_set_width(vfo2_frequency, LV_HOR_RES - 20);
	lv_label_set_text(vfo2_frequency, "7,200.00");
	lv_obj_set_height(vfo2_frequency, 40);
	
	band_label2 = lv_label_create(bg_tuner2);
	lv_label_set_text(band_label2, "1800 m ");
	lv_obj_align_to(band_label2, vfo2_frequency, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
	
	mode_label2 = lv_label_create(bg_tuner2);
	lv_label_set_text(mode_label2, "LSB ");
	lv_obj_align_to(mode_label2, vfo2_frequency, LV_ALIGN_OUT_BOTTOM_LEFT, 50, 10);
	
	rxtx_label2 = lv_label_create(bg_tuner2);
	lv_label_set_text(rxtx_label2, "RX");
	lv_obj_align_to(rxtx_label2, vfo2_frequency, LV_ALIGN_OUT_BOTTOM_LEFT, 200, 10);
	lv_obj_add_style(rxtx_label2, &label_style, 0);
	lv_label_set_recolor(rxtx_label2, true);
}


void gui_vfo::set_vfo_gui(int avfo, long long freq)
{
	char	str[20];
	
	if (freq > 10000000LU)
	{
		sprintf(str, "%3ld.%03ld,%02ld", (long)(freq / 1000000), (long)((freq / 1000) % 1000), (long)((freq / 10) % 100));
	}
	else
	{
		sprintf(str, "%3ld.%03ld,%02ld", (long)(freq / 1000000), (long)((freq / 1000) % 1000), (long)((freq / 10) % 100));
	}
	
	if (avfo)
	{
		lv_label_set_text(vfo2_frequency, str);
	}
	else
	{
		lv_label_set_text(vfo1_frequency, str);	
	}
	
	int band = vfo.get_band_no(avfo);
	sprintf(str, "%d m", band);
	
	if (avfo)
		lv_label_set_text(band_label2, str);
	else
		lv_label_set_text(band_label, str);
	
	if (vfo.get_rx() != rxtx)
	{
		rxtx = vfo.get_rx();	
		if (rxtx)
		{
			if (avfo)
				lv_label_set_text(rxtx_label2, "RX");
			else
				lv_label_set_text(rxtx_label, "RX");
		}
		else
		{
			if (avfo)
				lv_label_set_text(rxtx_label2, "#ff0000 TX#");
			else
				lv_label_set_text(rxtx_label, "#ff0000 TX#");	
		}
	}	
	
	if (mode[avfo] != vfo.get_mode_no(avfo))
	{
		mode[avfo] = vfo.get_mode_no(avfo);
		switch (mode[avfo])
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
		}
		if (avfo)
			lv_label_set_text(mode_label2, str);
		else
			lv_label_set_text(mode_label, str);
	}	
}

static lv_obj_t					*meter;
static lv_meter_indicator_t		*smeter_indic;
	
static void set_smeter_value(int32_t v)
{
	lv_meter_set_indicator_value(meter, (lv_meter_indicator_t *)smeter_indic, v);
}

static void smeter_event_cb(lv_event_t * e)
{
	lv_event_code_t			 code = lv_event_get_code(e);
	lv_obj_draw_part_dsc_t	*dsc  = (lv_obj_draw_part_dsc_t *)lv_event_get_param(e);
	
	switch (code)
	{
	case LV_EVENT_DRAW_PART_BEGIN:
		dsc->value = dsc->value / 10;
		if (dsc->value == 1)
		{
			strcpy(dsc->text, "S");			
		}
		
		if (dsc->value > 9)
		{
			if (dsc->value == 10)
				dsc->value = 20;
			if (dsc->value == 11)
				dsc->value = 40;
			if (dsc->value == 12)
				dsc->value = 60;				
		}
		lv_snprintf(dsc->text, sizeof(dsc->text), "%d", dsc->value);	
		break;
	}
}


static lv_style_t meter_style;

void set_smeter_img(lv_obj_t* box, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h)
{	
	lv_style_init(&meter_style);
	lv_style_set_radius(&meter_style, 0);
	lv_style_set_bg_color(&meter_style, lv_color_black());
	
	meter = lv_meter_create(box);
	lv_obj_set_pos(meter, x, y);
	lv_obj_set_size(meter, w, h);
	lv_obj_add_style(meter, &meter_style, 0);
	lv_obj_add_event_cb(meter, smeter_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
	
	/*Remove the circle from the middle*/
	//lv_obj_remove_style(meter, NULL, LV_PART_INDICATOR);
	lv_obj_set_style_pad_hor(meter, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(meter, 0, LV_PART_MAIN);
	lv_obj_set_style_size(meter, w, LV_PART_MAIN);
	
	/*Add a scale first*/
	lv_meter_scale_t * scale = lv_meter_add_scale(meter);
	lv_meter_set_scale_range(meter, scale, 10, 120, 100, 220);
	lv_meter_set_scale_ticks(meter, scale, 12, 10, 5, lv_palette_main(LV_PALETTE_LIGHT_GREEN));
	lv_meter_set_scale_major_ticks(meter, scale, 1, 2, 10, lv_color_hex3(0xeee), 10);
	
	//lv_meter_set_scale_major_ticks(meter, scale, 1, 4, 10, lv_color_hex3(0xeee), 10);
	
	lv_meter_indicator_t * indic;
	indic = lv_meter_add_arc(meter, scale, 3, lv_palette_main(LV_PALETTE_GREEN), 0); 
	lv_meter_set_indicator_start_value(meter, indic, 0);
	lv_meter_set_indicator_end_value(meter, indic, 90);
	
	lv_meter_indicator_t * indic1;
	indic1 = lv_meter_add_arc(meter, scale, 3, lv_palette_main(LV_PALETTE_RED), 0); 
	lv_meter_set_indicator_start_value(meter, indic1, 90);
	lv_meter_set_indicator_end_value(meter, indic1, 120);
	

	smeter_indic = lv_meter_add_needle_line(meter, scale, 1, lv_color_white(), -10);
	lv_meter_set_indicator_value(meter, smeter_indic, 0);
}

void set_s_meter(double value)
{
	//printf("value %f", value);
	//value = 30.0 + value;
	value = value + 100.0;
	//printf(" value s%f \n", value);
	lv_meter_set_indicator_value(meter, smeter_indic, value);
}
