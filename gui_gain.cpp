#include "gui_gain.h"

gui_gain guigain;

void gui_gain::init(lv_obj_t *o_tab, lv_coord_t w, lv_coord_t h)
{
	pageobj = o_tab;
	width = w;
	button_group = lv_group_create();
	lv_obj_set_style_pad_top(o_tab, 5, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(o_tab, 5, LV_PART_MAIN);
	lv_obj_set_style_pad_right(o_tab, 5, LV_PART_MAIN);

	lv_style_init(&style_btn);
	lv_style_set_radius(&style_btn, 10);
	lv_style_set_bg_color(&style_btn, lv_color_make(0x60, 0x60, 0x60));
	lv_style_set_bg_grad_color(&style_btn, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_bg_grad_dir(&style_btn, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_btn, 255);
	lv_style_set_border_color(&style_btn, lv_color_make(0x9b, 0x36, 0x36)); // lv_color_make(0x2e, 0x44, 0xb2)
	lv_style_set_border_width(&style_btn, 2);
	lv_style_set_border_opa(&style_btn, 255);
	lv_style_set_outline_color(&style_btn, lv_color_black());
	lv_style_set_outline_opa(&style_btn, 255);

	lv_obj_add_flag(pageobj, LV_OBJ_FLAG_HIDDEN);
	// lv_obj_clear_flag(o_tab, LV_OBJ_FLAG_SCROLLABLE);
	// lv_group_add_obj(button_group, lv_tabview_get_tab_btns(tabview_mid));

	store_gain_settings = lv_checkbox_create(o_tab);
	lv_group_add_obj(button_group, store_gain_settings);
	lv_checkbox_set_text(store_gain_settings, "store settings");
	lv_obj_add_event_cb(store_gain_settings, event_handler_store, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_obj_add_flag(store_gain_settings, LV_OBJ_FLAG_CHECKABLE);
	lv_obj_align(store_gain_settings, LV_ALIGN_BOTTOM_LEFT, width / 2, 0);

	int store_gain = Settings_file.get_int(default_radio, "store_gain_settings", 0);
	if (store_gain)
		lv_obj_add_state(store_gain_settings, LV_STATE_CHECKED);
}

void gui_gain::reset_gains()
{
	const lv_coord_t x_page_margin = 5;
	const lv_coord_t x_margin = 10;
	const lv_coord_t y_margin = 5;
	const int y_number_buttons = 4;
	const lv_coord_t tab_margin = 20;

	int x_number_buttons = 5;
	if (width > 800)
		x_number_buttons = 8;

	int button_width_margin = ((width - tab_margin) / x_number_buttons);
	int button_width = ((width - tab_margin) / x_number_buttons) - x_margin;
	int button_height = 50;
	int button_height_margin = button_height + y_margin;
	int ibutton_x = 0, ibutton_y = 0;
	
	rxgains.clear();
	rxgains = SdrDevices.SdrDevices.at(default_radio)->listGains(SOAPY_SDR_RX, channel);
	rxRanges.clear();
	for (auto col : rxgains)
	{
		SoapySDR::Range r;

		r = SdrDevices.SdrDevices.at(default_radio)->getGainRange(SOAPY_SDR_RX, channel, col);
		rxRanges.push_back(r);
	}
	channel = 0;
	
	if (rxgains.size() > 0)
		lv_obj_clear_flag(pageobj, LV_OBJ_FLAG_HIDDEN);
	else
		lv_obj_add_flag(pageobj, LV_OBJ_FLAG_HIDDEN);

	for (auto col : gain_labels)
		{
			lv_obj_del(col);
		}
	for (auto col : gain_sliders)
	{
		lv_obj_del(col);
	}

	int store_gain = Settings_file.get_int(default_radio, "store_gain_settings",0);
	int i = 0, x_column = 0;
	for (auto col : rxgains)
	{
		lv_obj_t *obj_slider = lv_slider_create(pageobj);
		gain_sliders.push_back(obj_slider);
		lv_obj_set_width(obj_slider, width / 2 - 50);
		lv_slider_set_range(obj_slider, rxRanges.at(i).minimum(), rxRanges.at(i).maximum() / rxRanges.at(i).step());
		lv_obj_align(obj_slider, LV_ALIGN_TOP_LEFT, x_margin + x_column, 5 * y_margin + ibutton_y * button_height_margin);
		lv_obj_add_event_cb(obj_slider, gain_slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
		lv_group_add_obj(button_group, obj_slider);
		if (!store_gain)
			lv_slider_set_value(obj_slider, (int)SdrDevices.SdrDevices.at(default_radio)->getGain(SOAPY_SDR_RX, 0, rxgains.at(i)) / rxRanges.at(i).step(), LV_ANIM_OFF);
		else
		{
			int gain = Settings_file.get_int(default_radio, rxgains.at(i), 0);
			lv_slider_set_value(obj_slider, gain, LV_ANIM_OFF);
			SdrDevices.SdrDevices.at(default_radio)->setGain(SOAPY_SDR_RX, channel, rxgains.at(i), (float)gain * rxRanges.at(i).step());
		}
		lv_obj_t *obj = lv_label_create(pageobj);
		gain_labels.push_back(obj);
		lv_label_set_text(obj, col.c_str());
		lv_obj_align_to(obj, obj_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);
		ibutton_y++;
		i++;
		if (i > 3)
		{
			x_column = width / 2;
			ibutton_y = 0;
		}
		if (i > 6) // max 6 
			break;
	}
}

void gui_gain::gain_slider_event_cb_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		int i = 0;
		for (auto col : gain_sliders)
		{
			if (col == obj)
				break;
			i++;
		}
		SdrDevices.SdrDevices.at(default_radio)->setGain(SOAPY_SDR_RX, channel, rxgains.at(i), (float)lv_slider_get_value(obj) * rxRanges.at(i).step());
		Settings_file.save_int(default_radio, rxgains.at(i), lv_slider_get_value(obj));
		Settings_file.write_settings();
	}
}

void gui_gain::set_gains()
{
	int i = 0;
	for (auto col : gain_sliders)
	{
		lv_slider_set_value(col, (int)SdrDevices.SdrDevices.at(default_radio)->getGain(SOAPY_SDR_RX, 0, rxgains.at(i)) / rxRanges.at(i).step(), LV_ANIM_OFF);
		i++;
	}	
}

int gui_gain::gains_count()
{
	return rxRanges.size();
}

void gui_gain::event_handler_store_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);

	if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
	{
		Settings_file.save_int(default_radio, "store_gain_settings", 1);
		Settings_file.write_settings();
	}
	else
	{
		Settings_file.save_int(default_radio, "store_gain_settings", 0);
		Settings_file.write_settings();
	}	
}

void gui_gain::clear_store_class()
{
	lv_obj_clear_state(store_gain_settings, LV_STATE_CHECKED);
}