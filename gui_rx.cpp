#include "gui_rx.h"
#include "gui_bar.h"
#include "Catinterface.h"
#include "Demodulator.h"
#include "Spectrum.h"
#include "screen.h"

gui_rx guirx;

void gui_rx::event_handler_morse_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
		{
			gbar.hide_cw(false);
			lv_obj_set_height(bar_view, barHeight + MorseHeight);
			lv_obj_set_pos(tabview_mid, 0, topHeight + tunerHeight + barHeight + MorseHeight);
			lv_obj_set_height(tabview_mid, screenHeight - topHeight - tunerHeight - barHeight - MorseHeight);
		}
		else
		{
			gbar.hide_cw(true);
			lv_obj_set_height(bar_view, barHeight);
			lv_obj_set_pos(tabview_mid, 0, topHeight + tunerHeight + barHeight);
			lv_obj_set_height(tabview_mid, screenHeight - topHeight - tunerHeight - barHeight);
		}
	}
}

void gui_rx::toggle_cw()
{
	if (lv_obj_get_state(check_cw) & LV_STATE_CHECKED)
	{
		int bandwidth{2500};
		lv_obj_clear_state(check_cw, LV_STATE_CHECKED);
		gbar.set_filter_slider(bandwidth);
		catinterface.SetSH(bandwidth);
	}
	else
	{
		int bandwidth{500};
		lv_obj_add_state(check_cw, LV_STATE_CHECKED);
		gbar.set_filter_slider(bandwidth);
		catinterface.SetSH(bandwidth);
	}
	lv_event_send(check_cw, LV_EVENT_VALUE_CHANGED, nullptr);
}

void gui_rx::set_cw(bool bcw)
{
	if (bcw)
		lv_obj_add_state(check_cw, LV_STATE_CHECKED);
	else
		lv_obj_clear_state(check_cw, LV_STATE_CHECKED);
	lv_event_send(check_cw, LV_EVENT_VALUE_CHANGED, nullptr);
}

bool gui_rx::get_cw()
{
	return lv_obj_get_state(check_cw) & LV_STATE_CHECKED;
}

void gui_rx::rx_button_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_CLICKED)
	{
		for (auto con : get_buttons())
		{
			if (con.first == obj)
			{
				if (con.second > 0L)
					vfo.set_vfo(con.second);
			}
			else
			{
				lv_obj_clear_state(con.first, LV_STATE_CHECKED);	
			}
		}
	}
	if (code == LV_EVENT_LONG_PRESSED)
	{
		for (auto con : get_buttons())
		{
			if (con.first == obj)
			{
				vector<long> array;
				set_freq(con.first, vfo.get_frequency());
				get_buttons(array);
				Settings_file.set_array_long("preselect", "buttons", array);
			}
			else
			{
			}
		}
	}
	
}

vector<pair<lv_obj_t *, long long>> gui_rx::get_buttons() 
{
	return buttons;
}

void gui_rx::get_buttons(vector<long> &array)
{
	for (auto con : buttons)
	{
		array.push_back(con.second);
	}
}

void gui_rx::waterfallsize_slider_event_class(lv_event_t *e)
{
	char buf[80];
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		sprintf(buf, "Waterfallsize level %d", lv_slider_get_value(obj));
		lv_label_set_text(waterfallsize_slider_label, buf);
		waterfallsize = lv_slider_get_value(obj);
		Settings_file.save_int("Radio", "waterfallsize", waterfallsize);
		SpectrumGraph.setWaterfallSize(waterfallsize);
	}
}

void gui_rx::noise_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		int noise = get_noise();
		Settings_file.save_int("Radio", "noise", noise);
		if (gbar.get_noise())
		{
			Demodulator::set_noise_filter(noise + 1);
		}
	}
}

void gui_rx::event_handler_hold_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	
	if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
	{
		SpectrumGraph.enable_second_data_series(true);
	}
	else
	{
		SpectrumGraph.enable_second_data_series(false);
	}
}

void gui_rx::set_freq(lv_obj_t *obj, long long freq)
{
	for (auto &con : buttons)
	{
		if (con.first == obj)
			con.second = freq;
	}
}

int gui_rx::get_noise()
{
	return lv_dropdown_get_selected(drp_noise);
}

void gui_rx::init(lv_obj_t *o_tab, lv_coord_t w)
{

	const lv_coord_t x_margin = 10;
	const lv_coord_t y_margin = 5;
	const int x_number_buttons = 5;
	const int y_number_buttons = 4;
	const lv_coord_t tab_margin = 20;

	int button_width_margin = ((w - tab_margin) / x_number_buttons);
	int button_width = ((w - tab_margin) / x_number_buttons) - x_margin;
	int button_height = 40;
	int button_height_margin = button_height + y_margin;
	int ibutton_x = 0, ibutton_y = 0;
	
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
	lv_obj_clear_flag(o_tab, LV_OBJ_FLAG_SCROLLABLE);

	m_button_group = lv_group_create();
	int ibuttons = number_of_buttons;
	vector<long> array_long;
	Settings_file.get_array_long("preselect", "buttons", array_long);
	for (int i = 0; i < ibuttons; i++)
	{
		char str[80];

		if (i < array_long.size())
			buttons.push_back(std::make_pair(lv_btn_create(o_tab), (long long)array_long.at(i)));
		else
			buttons.push_back(std::make_pair(lv_btn_create(o_tab), (long long)0));
		lv_group_add_obj(m_button_group, buttons.back().first);
		lv_obj_add_style(buttons.back().first, &style_btn, 0);
		lv_obj_add_event_cb(buttons.back().first, rx_button_handler, (lv_event_code_t)LV_EVENT_ALL /*(LV_EVENT_CLICKED | LV_EVENT_LONG_PRESSED)*/, (void*)this);
		lv_obj_align(buttons.back().first, LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin, ibutton_y * button_height_margin);
		//lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
		lv_obj_set_size(buttons.back().first, button_width, button_height);

		lv_obj_t *lv_label = lv_label_create(buttons.back().first);
		switch (i)
		{
		case 0:
			strcpy(str, "Pre 1");
			lv_obj_add_flag(buttons.back().first, LV_OBJ_FLAG_CHECKABLE);
			break;
		case 1:
			strcpy(str, "Pre 2");
			lv_obj_add_flag(buttons.back().first, LV_OBJ_FLAG_CHECKABLE);
			break;
		case 2:
			strcpy(str, "Pre 3");
			lv_obj_add_flag(buttons.back().first, LV_OBJ_FLAG_CHECKABLE);
			break;
		case 3:
			strcpy(str, "Pre 4");
			lv_obj_add_flag(buttons.back().first, LV_OBJ_FLAG_CHECKABLE);
			break;
		case 4:
			strcpy(str, "Pre 5");
			lv_obj_add_flag(buttons.back().first, LV_OBJ_FLAG_CHECKABLE);
			break;
		}
		lv_label_set_text(lv_label, str);
		lv_obj_center(lv_label);

		ibutton_x++;
		if (ibutton_x >= x_number_buttons)
		{
			ibutton_x = 0;
			ibutton_y++;
		}
	}

	lv_obj_t *noise_label = lv_label_create(o_tab);
	lv_label_set_text(noise_label, "Noise suppression");
	lv_obj_align(noise_label, LV_ALIGN_TOP_LEFT, 0, y_margin + ibutton_y * button_height_margin);

	drp_noise = lv_dropdown_create(o_tab);
	lv_obj_align(drp_noise, LV_ALIGN_TOP_LEFT, 0, y_margin + ibutton_y * 1.5 *  button_height_margin);
	lv_dropdown_clear_options(drp_noise);
	lv_group_add_obj(m_button_group, drp_noise);
	lv_dropdown_add_option(drp_noise, "Leaky LMS", LV_DROPDOWN_POS_LAST);
	//lv_dropdown_add_option(drp_noise, "LMS", LV_DROPDOWN_POS_LAST);
	lv_dropdown_add_option(drp_noise, "Spectral", LV_DROPDOWN_POS_LAST);
	lv_dropdown_add_option(drp_noise, "Kim", LV_DROPDOWN_POS_LAST);
	lv_dropdown_add_option(drp_noise, "fft", LV_DROPDOWN_POS_LAST);

	int noise = Settings_file.get_int("Radio", "noise");
	lv_dropdown_set_selected(drp_noise, noise);
	lv_obj_add_event_cb(drp_noise, noise_handler, (lv_event_code_t)LV_EVENT_VALUE_CHANGED, (void*)this);

	check_cw = lv_checkbox_create(o_tab);
	lv_group_add_obj(m_button_group, check_cw);
	lv_checkbox_set_text(check_cw, "Morse Decoder");
	lv_obj_add_event_cb(check_cw, event_handler_morse, LV_EVENT_ALL, (void *)this);
	lv_obj_align(check_cw, LV_ALIGN_TOP_LEFT, 1 * button_width_margin, y_margin + ibutton_y * button_height_margin);
	lv_group_add_obj(m_button_group, check_cw);

	noise_slider_label = lv_label_create(o_tab);
	lv_obj_center(noise_slider_label);
	char buf[80];
	int ii = Settings_file.get_int("Radio", "NoiseThreshold");
	sprintf(buf, "noise thresshold %d db", ii);
	lv_label_set_text(noise_slider_label, buf);
	Demodulator::set_noise_threshold(ii);
	
	noise_slider = lv_slider_create(o_tab);
	lv_obj_set_width(noise_slider, w / 2 - 50);
	lv_slider_set_range(noise_slider, -100, 0);
	lv_obj_align_to(noise_slider, noise_slider_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	lv_obj_add_event_cb(noise_slider, noise_slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_group_add_obj(m_button_group, noise_slider);

	lv_group_add_obj(m_button_group, lv_tabview_get_tab_btns(tabview_mid));

	waterfallgain = Settings_file.get_int("Radio", "Waterfallgain", 35);
	waterfall_slider = lv_slider_create(o_tab);
	lv_obj_set_width(waterfall_slider, w / 2 - 50);
	lv_slider_set_range(waterfall_slider, 0, 100);
	lv_obj_align_to(waterfall_slider, noise_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);
	lv_obj_add_event_cb(waterfall_slider, waterfall_slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_group_add_obj(m_button_group, waterfall_slider);
	lv_slider_set_value(waterfall_slider, waterfallgain, LV_ANIM_OFF);

	waterfall_slider_label = lv_label_create(o_tab);
	sprintf(buf, "Waterfall level %d db", waterfallgain);
	lv_label_set_text(waterfall_slider_label, buf);
	lv_obj_align_to(waterfall_slider_label, waterfall_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);

	waterfall_hold = lv_checkbox_create(o_tab);
	lv_group_add_obj(m_button_group, waterfall_hold);
	lv_checkbox_set_text(waterfall_hold, "Spectrum hold");
	lv_obj_add_event_cb(waterfall_hold, event_handler_hold, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_obj_add_flag(waterfall_hold, LV_OBJ_FLAG_CHECKABLE);
	lv_obj_align_to(waterfall_hold, waterfall_slider, LV_ALIGN_OUT_RIGHT_TOP, 30, 0);
	//lv_obj_align(waterfall_hold, LV_ALIGN_TOP_LEFT, 1 * button_width_margin, y_margin + ibutton_y * button_height_margin);
	lv_group_add_obj(m_button_group, waterfall_hold);

	waterfallsize = Settings_file.get_int("Radio", "waterfallsize", 3);
	waterfallsize_slider = lv_slider_create(o_tab);
	lv_obj_set_width(waterfallsize_slider, w / 2 - 50);
	lv_slider_set_range(waterfallsize_slider, 0, 10);
	lv_obj_align_to(waterfallsize_slider, waterfall_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);
	lv_obj_add_event_cb(waterfallsize_slider, waterfallsize_slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_group_add_obj(m_button_group, waterfallsize_slider);
	lv_slider_set_value(waterfallsize_slider, waterfallsize, LV_ANIM_OFF);

	waterfallsize_slider_label = lv_label_create(o_tab);
	sprintf(buf, "Waterfallsize level %d", waterfallsize);
	lv_label_set_text(waterfallsize_slider_label, buf);
	lv_obj_align_to(waterfallsize_slider_label, waterfallsize_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);

	lv_group_add_obj(m_button_group, lv_tabview_get_tab_btns(tabview_mid));
}

void gui_rx::noise_slider_event_cb_class(lv_event_t *e)
{
	lv_obj_t *slider = lv_event_get_target(e);
	char buf[30];

	sprintf(buf, "noise thresshold %d db", lv_slider_get_value(slider));
	lv_label_set_text(noise_slider_label, buf);
	lv_obj_align_to(noise_slider_label, slider, LV_ALIGN_OUT_TOP_MID, 0, -10);
	Settings_file.save_int("Radio","NoiseThreshold",lv_slider_get_value(slider));
	if (gbar.get_noise())
	{
		Demodulator::set_noise_threshold(lv_slider_get_value(slider));
	}
}

void gui_rx::waterfall_slider_event_cb_class(lv_event_t *e)
{
	lv_obj_t *slider = lv_event_get_target(e);
	char buf[30];

	sprintf(buf, "Waterfall level %d db", lv_slider_get_value(slider));
	lv_label_set_text(waterfall_slider_label, buf);
	lv_obj_align_to(waterfall_slider_label, waterfall_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);
	waterfallgain = lv_slider_get_value(slider);
	Settings_file.save_int("Radio", "Waterfallgain", waterfallgain);
	Settings_file.write_settings();
}
