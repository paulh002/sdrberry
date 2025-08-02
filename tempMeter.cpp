#include "tempMeter.h"

void tempMeter::draw_event_cb_class(lv_event_t *e)
{
}

lv_obj_t * tempMeter::init(lv_obj_t *parent, int32_t w, int32_t h)
{
	meter = lv_meter_create(parent);
	lv_obj_set_size(meter, w, h);
	needle_length = w / 2 -  4;
	//lv_scale_set_label_show(scale, true);
	//lv_scale_set_mode(scale, LV_SCALE_MODE_ROUND_OUTER);
	//lv_obj_center(scale);

	lv_obj_set_style_pad_top(meter, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(meter, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_right(meter, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_left(meter, 0, LV_PART_MAIN);

	
	lv_style_init(&meter_style);
	lv_style_set_radius(&meter_style, 0);
	lv_style_set_bg_color(&meter_style, lv_color_black());
	lv_style_set_bg_opa(&meter_style, LV_OPA_100);
	lv_style_set_line_rounded(&meter_style, false);
	lv_obj_add_style(meter, &meter_style, 0);


	
	scale = lv_meter_add_scale(meter);
	lv_meter_set_scale_ticks(meter, scale, 21, 1, 5, lv_color_white());
	lv_meter_set_scale_major_ticks(meter, scale, 5, 4, 10, lv_color_white(), 10);
	lv_meter_set_scale_range(meter, scale, 0, 100, 250, 145);

	needle = lv_meter_add_needle_line(meter, scale, 2, lv_color_white(), -10);
	lv_obj_remove_style(meter, NULL, LV_PART_INDICATOR);
	
	lv_meter_indicator_t *indic;
	indic = lv_meter_add_arc(meter, scale, 2, lv_color_white(), 0);
	lv_meter_set_indicator_start_value(meter, indic, 0);
	lv_meter_set_indicator_end_value(meter, indic, 75);

	indic = lv_meter_add_arc(meter, scale, 4, lv_palette_main(LV_PALETTE_RED), 0);
	lv_meter_set_indicator_start_value(meter, indic, 75);
	lv_meter_set_indicator_end_value(meter, indic, 100);

	indic = lv_meter_add_scale_lines(meter, scale, lv_palette_main(LV_PALETTE_RED), lv_palette_main(LV_PALETTE_RED), false, 0);
	lv_meter_set_indicator_start_value(meter, indic, 75);
	lv_meter_set_indicator_end_value(meter, indic, 100);

	indic = lv_meter_add_scale_lines(meter, scale, lv_color_white(), lv_color_white(), false, 0);
	lv_meter_set_indicator_start_value(meter, indic, 25);
	lv_meter_set_indicator_end_value(meter, indic, 80);

	//static const char *custom_labels[] = {"0 °C", "25 °C", "50 °C", "75 °C", "100 °C", NULL};
	//lv_scale_set_text_src(scale, custom_labels);

	temp_label = lv_label_create(parent);
	lv_obj_align_to(temp_label, meter, LV_ALIGN_OUT_BOTTOM_MID, 0, 38);
	lv_label_set_text(temp_label, "T 20.1 °C");
	return meter;
}

void tempMeter::set_range(int32_t startvalue, int endvalue)
{
	lv_meter_set_scale_range(meter, scale, startvalue, endvalue, 75, 100);
	//lv_scale_set_range(scale, startvalue, endvalue);
}

void tempMeter::set_indicator(int32_t value)
{
	char str[80];
	sprintf(str, "T %2.1f °C", (float)value / 1000.0);
	lv_meter_set_indicator_value(meter, needle, value / 1000);
	lv_label_set_text(temp_label, str);
}

void tempMeter::set_pos(int32_t x, int32_t y)
{
	lv_obj_set_pos(meter, x, y);
}

void tempMeter::set_labels(const char *txt_src[])
{
	//lv_scale_set_text_src(scale, txt_src);
}

void tempMeter::set_section_range(int32_t start, int32_t end)
{
	//lv_scale_section_set_range(section, start, end);
}

void tempMeter::hide(bool hide)
{
	if (hide)
	{
		lv_obj_add_flag(meter, LV_OBJ_FLAG_HIDDEN);
		lv_obj_add_flag(temp_label, LV_OBJ_FLAG_HIDDEN);
	}
	else
	{
		lv_obj_clear_flag(meter, LV_OBJ_FLAG_HIDDEN);
		lv_obj_clear_flag(temp_label, LV_OBJ_FLAG_HIDDEN);
	}
}
