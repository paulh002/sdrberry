#include "gui_rx.h"
#include "gui_bar.h"
#include "gui_vfo.h"
#include "Catinterface.h"
#include "Demodulator.h"
#include "Spectrum.h"
#include "screen.h"
#include "gui_gain.h"

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
		gbar.set_filter_dropdown(bandwidth);
	}
	else
	{
		int bandwidth{500};
		lv_obj_add_state(check_cw, LV_STATE_CHECKED);
		gbar.set_filter_dropdown(bandwidth);
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

void gui_rx::waterfallsize_slider_event_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		std::string buf = strlib::sprintf("Waterfallsize %d", lv_slider_get_value(obj));
		lv_label_set_text(waterfallsize_slider_label, buf.c_str());
		waterfallsize = lv_slider_get_value(obj);
		Settings_file.save_int("Radio", "waterfallsize", waterfallsize);
		SpectrumGraph.setWaterfallSize(waterfallsize);
		Settings_file.write_settings();
	}
}

void gui_rx::signal_strength_offset_event_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		int signal_strength_offset = lv_slider_get_value(obj);
		std::string buf = strlib::sprintf("s meter offset %d", signal_strength_offset);
		lv_label_set_text(signal_strength_offset_slider_label, buf.c_str());
		Settings_file.save_int("Radio", "s-meter-offset", signal_strength_offset);
		Settings_file.write_settings();
		SpectrumGraph.setSignalStrenthOffset(signal_strength_offset);
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
		Settings_file.write_settings();
		if (gbar.get_noise())
		{
			Demodulator::set_noise_filter(noise + 1);
		}
	}
}

void gui_rx::filter_type_handler_cb_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		int filter_type = lv_dropdown_get_selected(obj);
		Settings_file.save_int("Radio", "filter_type", filter_type);
		Settings_file.write_settings();
		Demodulator::set_filter_type(filter_type);
	}
}

void gui_rx::filter_slider_event_cb_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		std::string buf = strlib::sprintf("filter offset %d", 10 * lv_slider_get_value(obj));
		lv_label_set_text(filter_slider_label, buf.c_str());
		Settings_file.save_int("Radio", "filter_offset", 10 * lv_slider_get_value(obj));
		Settings_file.write_settings();
		Demodulator::set_filter_offset(10 * lv_slider_get_value(obj));
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

void gui_rx::smeter_delay_event_cb_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		int delay = lv_slider_get_value(obj);
		std::string buf = strlib::sprintf("s meter delay %d", delay);
		Settings_file.save_int("Radio", "s-meter-delay", delay);
		Settings_file.write_settings();
		lv_label_set_text(smeter_delay_slider_label, buf.c_str());
		gui_vfo_inst.set_smeter_delay(delay);
		printf("smeter delay %d\n", delay);
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
	
	lv_obj_set_style_pad_top(o_tab, 10, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(o_tab, 5, LV_PART_MAIN);
	lv_obj_set_style_pad_right(o_tab, 5, LV_PART_MAIN);
	
	tileview = lv_tileview_create(o_tab);
	lv_obj_clear_flag(tileview, LV_OBJ_FLAG_SCROLL_ELASTIC);
	gain_tile = lv_tileview_add_tile(tileview, 0, 3, LV_DIR_BOTTOM | LV_DIR_TOP);
	guigain.init(gain_tile, w, screenWidth);
	main_tile = lv_tileview_add_tile(tileview, 0, 1, LV_DIR_BOTTOM | LV_DIR_TOP);
	settings_tile = lv_tileview_add_tile(tileview, 0, 2, LV_DIR_BOTTOM | LV_DIR_TOP);
	
	
	lv_obj_set_style_pad_top(settings_tile, 0, LV_PART_MAIN);
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
	lv_obj_clear_flag(main_tile, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_clear_flag(settings_tile, LV_OBJ_FLAG_SCROLLABLE);

	button_group = lv_group_create();
	
	lv_obj_t *filter_type_label = lv_label_create(main_tile);
	lv_label_set_text(filter_type_label, "Filter type");
	lv_obj_align(filter_type_label, LV_ALIGN_TOP_LEFT, 0, y_margin);

	filter_type_dropdown = lv_dropdown_create(main_tile);
	lv_obj_align(filter_type_dropdown, LV_ALIGN_TOP_LEFT, 0, y_margin + button_height_margin * 0.5);
	lv_dropdown_clear_options(filter_type_dropdown);
	lv_group_add_obj(button_group, filter_type_dropdown);
	lv_dropdown_set_options(filter_type_dropdown, "Butterworth\n" "Chebyshev - 1\n" "Chebyshev - 2\n" "Elliptic");
	int filter_type = Settings_file.get_int("Radio", "filter_type");
	Demodulator::set_filter_type(filter_type);
	lv_dropdown_set_selected(filter_type_dropdown, filter_type);
	lv_obj_add_event_cb(filter_type_dropdown, filter_type_handler_cb, (lv_event_code_t)LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_obj_set_width(filter_type_dropdown, button_width_margin);

	lv_obj_t *noise_label = lv_label_create(main_tile);
	lv_label_set_text(noise_label, "Noise suppression");
	lv_obj_align(noise_label, LV_ALIGN_TOP_RIGHT, x_margin * -2, y_margin);

	drp_noise = lv_dropdown_create(main_tile);
	lv_obj_align(drp_noise, LV_ALIGN_TOP_RIGHT, x_margin * -2, y_margin + 0.5 *  button_height_margin);
	lv_dropdown_clear_options(drp_noise);
	lv_group_add_obj(button_group, drp_noise);
	lv_dropdown_add_option(drp_noise, "Leaky LMS", LV_DROPDOWN_POS_LAST);
	//lv_dropdown_add_option(drp_noise, "LMS", LV_DROPDOWN_POS_LAST);
	lv_dropdown_add_option(drp_noise, "Spectral", LV_DROPDOWN_POS_LAST);
	lv_dropdown_add_option(drp_noise, "Kim", LV_DROPDOWN_POS_LAST);
	lv_dropdown_add_option(drp_noise, "fft", LV_DROPDOWN_POS_LAST);

	int noise = Settings_file.get_int("Radio", "noise");
	lv_dropdown_set_selected(drp_noise, noise);
	lv_obj_add_event_cb(drp_noise, noise_handler, (lv_event_code_t)LV_EVENT_VALUE_CHANGED, (void*)this);

	filter_slider_label = lv_label_create(main_tile);
	lv_obj_align(filter_slider_label, LV_ALIGN_TOP_MID, 0, y_margin + ibutton_y * button_height_margin);
	std::string buf = strlib::sprintf("filter offset %d hz", Settings_file.get_int("Radio", "filter_offset"));
	lv_label_set_text(filter_slider_label, buf.c_str());

	filter_slider = lv_slider_create(main_tile);
	lv_obj_set_width(filter_slider, w / 2 - 50);
	lv_slider_set_range(filter_slider, 0, 100);
	lv_obj_align_to(filter_slider, filter_slider_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	lv_obj_add_event_cb(filter_slider, filter_slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_group_add_obj(button_group, filter_slider);
	lv_slider_set_value(filter_slider, Settings_file.get_int("Radio", "filter_offset") / 10, LV_ANIM_OFF);
	Demodulator::set_filter_offset(Settings_file.get_int("Radio", "filter_offset"));

	ibutton_y++;
	noise_slider_label = lv_label_create(main_tile);
	lv_obj_align(noise_slider_label, LV_ALIGN_TOP_MID, 0, y_margin + ibutton_y * button_height_margin);

	buf = strlib::sprintf("noise thresshold %d db", Settings_file.get_int("Radio", "NoiseThreshold"));
	lv_label_set_text(noise_slider_label, buf.c_str());
	Demodulator::set_noise_threshold(Settings_file.get_int("Radio", "NoiseThreshold"));

	noise_slider = lv_slider_create(main_tile);
	lv_obj_set_width(noise_slider, w / 2 - 50);
	lv_slider_set_range(noise_slider, -100, 0);
	lv_obj_align_to(noise_slider, noise_slider_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	lv_obj_add_event_cb(noise_slider, noise_slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_group_add_obj(button_group, noise_slider);

	lv_group_add_obj(button_group, lv_tabview_get_tab_btns(tabview_mid));
	
	ibutton_y++;
	check_cw = lv_checkbox_create(main_tile);
	lv_group_add_obj(button_group, check_cw);
	lv_checkbox_set_text(check_cw, "Morse Decoder");
	lv_obj_add_event_cb(check_cw, event_handler_morse, LV_EVENT_ALL, (void *)this);
	lv_obj_align(check_cw, LV_ALIGN_TOP_LEFT, x_margin, y_margin + (ibutton_y + 1) * button_height_margin);
	lv_group_add_obj(button_group, check_cw);

	waterfallgain = Settings_file.get_int("Radio", "Waterfallgain", 35);
	waterfall_slider = lv_slider_create(settings_tile);
	lv_obj_set_width(waterfall_slider, w / 2 - 50);
	lv_slider_set_range(waterfall_slider, -20, 100);
	lv_obj_align(waterfall_slider, LV_ALIGN_TOP_LEFT, x_margin, 5 * y_margin);

	//lv_obj_align_to(waterfall_slider, noise_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);
	lv_obj_add_event_cb(waterfall_slider, waterfall_slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_group_add_obj(button_group, waterfall_slider);
	lv_slider_set_value(waterfall_slider, waterfallgain, LV_ANIM_OFF);

	waterfall_slider_label = lv_label_create(settings_tile);
	buf = strlib::sprintf("Waterfall level %d db", waterfallgain);
	lv_label_set_text(waterfall_slider_label, buf.c_str());
	lv_obj_align_to(waterfall_slider_label, waterfall_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);

	waterfallsize = Settings_file.get_int("Radio", "waterfallsize", 3);
	waterfallsize_slider = lv_slider_create(settings_tile);
	lv_obj_set_width(waterfallsize_slider, w / 2 - 50);
	lv_slider_set_range(waterfallsize_slider, 0, 10);
	//lv_obj_align_to(waterfallsize_slider, waterfall_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);
	lv_obj_align(waterfallsize_slider, LV_ALIGN_TOP_RIGHT, x_margin * -2, 5 * y_margin);
	lv_obj_add_event_cb(waterfallsize_slider, waterfallsize_slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_group_add_obj(button_group, waterfallsize_slider);
	lv_slider_set_value(waterfallsize_slider, waterfallsize, LV_ANIM_OFF);

	waterfallsize_slider_label = lv_label_create(settings_tile);
	buf = strlib::sprintf("Waterfallsize %d", waterfallsize);
	lv_label_set_text(waterfallsize_slider_label, buf.c_str());
	lv_obj_align_to(waterfallsize_slider_label, waterfallsize_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);

	int signal_strength_offset = Settings_file.get_int("Radio", "s-meter-offset", 200);
	signal_strength_offset_slider = lv_slider_create(settings_tile);
	lv_obj_set_width(signal_strength_offset_slider, w / 2 - 50);
	lv_slider_set_range(signal_strength_offset_slider, 0, 400);
	lv_obj_align_to(signal_strength_offset_slider, waterfall_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);
	lv_obj_add_event_cb(signal_strength_offset_slider, signal_strength_offset_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_group_add_obj(button_group, signal_strength_offset_slider);
	lv_slider_set_value(signal_strength_offset_slider, signal_strength_offset, LV_ANIM_OFF);

	signal_strength_offset_slider_label = lv_label_create(settings_tile);
	buf = strlib::sprintf("s meter offset %d", signal_strength_offset);
	lv_label_set_text(signal_strength_offset_slider_label, buf.c_str());
	lv_obj_align_to(signal_strength_offset_slider_label, signal_strength_offset_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);

	int smeter_delay = Settings_file.get_int("Radio", "s-meter-delay", 0);
	smeter_delay_slider = lv_slider_create(settings_tile);
	lv_obj_set_width(smeter_delay_slider, w / 2 - 50);
	lv_slider_set_range(smeter_delay_slider, 0, 4);
	lv_obj_align_to(smeter_delay_slider, waterfallsize_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);
	lv_obj_add_event_cb(smeter_delay_slider, smeter_delay_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_group_add_obj(button_group, smeter_delay_slider);
	lv_slider_set_value(smeter_delay_slider, smeter_delay, LV_ANIM_OFF);
	
	smeter_delay_slider_label = lv_label_create(settings_tile);
	buf = strlib::sprintf("s meter delay %d", smeter_delay);
	lv_label_set_text(smeter_delay_slider_label, buf.c_str());
	lv_obj_align_to(smeter_delay_slider_label, smeter_delay_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);
	
	spectrumgain = Settings_file.get_int("Radio", "Spectrumgain", 0);
	spectrum_slider = lv_slider_create(settings_tile);
	lv_obj_set_width(spectrum_slider, w / 2 - 50);
	lv_slider_set_range(spectrum_slider, -50, 50);
	lv_obj_align_to(spectrum_slider, signal_strength_offset_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 30);
	lv_obj_add_event_cb(spectrum_slider, spectrum_slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_group_add_obj(button_group, spectrum_slider);
	lv_slider_set_value(spectrum_slider, spectrumgain, LV_ANIM_OFF);

	spectrum_slider_label = lv_label_create(settings_tile);
	buf = strlib::sprintf("Spectrum level %d db", spectrumgain);
	lv_label_set_text(spectrum_slider_label, buf.c_str());
	lv_obj_align_to(spectrum_slider_label, spectrum_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);

	waterfall_hold = lv_checkbox_create(settings_tile);
	lv_group_add_obj(button_group, waterfall_hold);
	lv_checkbox_set_text(waterfall_hold, "Spectrum hold");
	lv_obj_add_event_cb(waterfall_hold, event_handler_hold, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_obj_add_flag(waterfall_hold, LV_OBJ_FLAG_CHECKABLE);
	ibutton_y++;
	lv_obj_align(waterfall_hold, LV_ALIGN_TOP_LEFT, x_margin, y_margin + ibutton_y * button_height_margin);
	//lv_obj_align_to(spectrum_slider, waterfall_hold, LV_ALIGN_TOP_LEFT, x_margin + 2 * button_width_margin, 0);
	lv_group_add_obj(button_group, waterfall_hold);
	
	lv_group_add_obj(button_group, lv_tabview_get_tab_btns(tabview_mid));
	lv_obj_set_tile_id(tileview, 0, 0, LV_ANIM_OFF);
}

void gui_rx::enable_filter_settings(bool enable)
{
	if (enable)
	{
		lv_obj_clear_state(filter_type_dropdown, LV_STATE_DISABLED);
		lv_obj_clear_state(filter_slider, LV_STATE_DISABLED);
	}
	else
	{
		lv_obj_add_state(filter_type_dropdown, LV_STATE_DISABLED);
		lv_obj_add_state(filter_slider, LV_STATE_DISABLED);
	}
}

void gui_rx::noise_slider_event_cb_class(lv_event_t *e)
{
	lv_obj_t *slider = lv_event_get_target(e);

	std::string buf = strlib::sprintf("noise thresshold %d db", lv_slider_get_value(slider));
	lv_label_set_text(noise_slider_label, buf.c_str());
	lv_obj_align_to(noise_slider_label, slider, LV_ALIGN_OUT_TOP_MID, 0, -10);
	Settings_file.save_int("Radio","NoiseThreshold",lv_slider_get_value(slider));
	Settings_file.write_settings();
	if (gbar.get_noise())
	{
		Demodulator::set_noise_threshold(lv_slider_get_value(slider));
	}
}

void gui_rx::waterfall_slider_event_cb_class(lv_event_t *e)
{
	lv_obj_t *slider = lv_event_get_target(e);

	std::string buf = strlib::sprintf("Waterfall level %d db", lv_slider_get_value(slider));
	lv_label_set_text(waterfall_slider_label, buf.c_str());
	lv_obj_align_to(waterfall_slider_label, waterfall_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);
	waterfallgain = lv_slider_get_value(slider);
	Settings_file.save_int("Radio", "Waterfallgain", waterfallgain);
	Settings_file.write_settings();
}

void gui_rx::spectrum_slider_event_cb_class(lv_event_t *e)
{
	lv_obj_t *slider = lv_event_get_target(e);

	std::string buf = strlib::sprintf("Spectrum level %d db", lv_slider_get_value(slider));
	lv_label_set_text(spectrum_slider_label, buf.c_str());
	lv_obj_align_to(spectrum_slider_label, spectrum_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);
	spectrumgain = lv_slider_get_value(slider);
	Settings_file.save_int("Radio", "Spectrumgain", spectrumgain);
	Settings_file.write_settings();
}