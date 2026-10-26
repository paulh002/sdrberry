#include "gui_mic.h"
#include "audiobar.h"

gui_mic guimic;

void gui_mic::init(lv_obj_t *parent, lv_coord_t w, lv_coord_t h)
{
	lv_style_init(&mic_style);
	lv_style_set_radius(&mic_style, 0);
	lv_style_set_bg_color(&mic_style, lv_color_black());
	lv_style_set_bg_opa(&mic_style, LV_OPA_100);
	
	lv_obj_set_style_pad_top(parent, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(parent, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_right(parent, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_left(parent, 0, LV_PART_MAIN);
	int w_box = 2 * (w / 6);
	mic_bar = lv_obj_create(parent);
	lv_obj_add_style(mic_bar, &mic_style, 0);
	lv_obj_set_pos(mic_bar, 0, 0);
	lv_obj_set_size(mic_bar, w_box - 5, h - 5);
	lv_obj_clear_flag(mic_bar, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_pad_top(mic_bar, 5, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(mic_bar, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_right(mic_bar, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_left(mic_bar, 0, LV_PART_MAIN);
	
	lv_obj_t *mic_label = lv_label_create(mic_bar);
	lv_label_set_text(mic_label, " Mic:");
	lv_obj_align(mic_label, LV_ALIGN_LEFT_MID, 3, 0);
	
	audiobar_mic.init(mic_bar, 7 * (w / 8), 0);
	audiobar_mic.set_size(0, 20);
	audiobar_mic.align(mic_bar, LV_ALIGN_LEFT_MID, 50, 0);
	audiobar_mic.SetRange(300);
	mic_filter = std::make_unique<SMeterFilter>(20.0f, 25.0f, 280.0f); // SSB defaults
	
	alc_bar = lv_obj_create(parent);
	lv_obj_add_style(alc_bar, &mic_style, 0);
	lv_obj_set_pos(alc_bar, w_box, 0);
	lv_obj_set_size(alc_bar, w_box - 3, h);
	lv_obj_clear_flag(alc_bar, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_pad_top(alc_bar, 5, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(alc_bar, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_right(alc_bar, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_left(alc_bar, 0, LV_PART_MAIN);
	
	lv_obj_t *agc_label = lv_label_create(alc_bar);
	lv_label_set_text(agc_label, " Alc:");
	lv_obj_align(agc_label, LV_ALIGN_LEFT_MID, 3, 0);	
	
	audiobar_alc.init(alc_bar, 7 * (w / 8), 0);
	audiobar_alc.set_size(0, 20);
	audiobar_alc.align(alc_bar, LV_ALIGN_LEFT_MID, 50, 0);
	audiobar_alc.SetRange(25);
	alc_filter = std::make_unique<SMeterFilter>(20.0f, 25.0f, 280.0f); // SSB defaults
	
	smeter_bar = lv_obj_create(parent);
	lv_obj_add_style(smeter_bar, &mic_style, 0);
	lv_obj_set_pos(smeter_bar, 2 * w_box, 0);
	lv_obj_set_size(smeter_bar, w_box - 5, h);
	lv_obj_clear_flag(smeter_bar, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_style_pad_top(smeter_bar, 5, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(smeter_bar, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_right(smeter_bar, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_left(smeter_bar, 0, LV_PART_MAIN);
	
	lv_obj_t *smeter_label = lv_label_create(smeter_bar);
	lv_label_set_text(smeter_label, "Sign:");
	lv_obj_align(smeter_label, LV_ALIGN_LEFT_MID, 3, 0);	
	
	audiobar_s.init(smeter_bar, 7 * (w / 8), 0);
	audiobar_s.set_size(0, 20);
	audiobar_s.align(smeter_bar, LV_ALIGN_LEFT_MID, 50, 0);
	audiobar_s.SetRange(100);
	s_filter = std::make_unique<SMeterFilter>(30.0f, 25.0f, 280.0f); // SSB defaults
	
	lv_obj_clear_flag(parent, LV_OBJ_FLAG_SCROLLABLE);
}

void gui_mic::set_value_mic(float value)
{
	if (mic_filter)
	{
		float smoothed = mic_filter->process(value + 350.0f);
		audiobar_mic.value(smoothed);
	}
}

void gui_mic::set_value_alc(float value)
{
	if (alc_filter)
	{
		value = value * -1.0;
		float smoothed = alc_filter->process(value);
		audiobar_alc.value(smoothed);
	}
}

void gui_mic::set_value_s(float value)
{
	if (s_filter)
	{
		float smoothed = s_filter->process(value);
		audiobar_s.value(smoothed);
	}
}