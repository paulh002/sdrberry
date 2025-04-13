#include "gui_setup.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include <atomic>
#include "Spectrum.h"
#include "gui_cal.h"
#include "gui_ft8bar.h"
#include "gui_bar.h"
#include "gui_i2csetup.h"
#include "gui_i2c_input.h"
#include "Demodulator.h"
#include "screen.h"
#include "WebServer.h"

gui_setup	gsetup;
extern 		void switch_sdrreceiver(std::string receiver);

static const char *cal_opts = "None\n"
							  "Manual\n"
							  "Auto\n"
							  "Both";

void gui_setup::webbox_event_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
		{
			Settings_file.save_int("web", "enabled", 1);
			webserver.StartServer();
		}
		else
		{
			Settings_file.save_int("web", "enabled", 0);
			webserver.StopServer();
		}
	Settings_file.save();
	}
}

void gui_setup::calbox_event_cb_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
		{
			gbar.hide(true);
			guift8bar.hide(true);
			gcal.hide(false);
		}
		else
		{
			Settings_file.save();
			gbar.hide(false);
			guift8bar.hide(true);
			gcal.hide(true);
		}
	}
}

void gui_setup::cal_button_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		int i = lv_dropdown_get_selected(obj);
		Demodulator::set_autocorrection(i);
		Settings_file.save_int(default_radio, "correction", i);
		Settings_file.write_settings();
	}
}

void gui_setup::dcbox_event_cb_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
		{
			Demodulator::set_dc_filter(true);
			Settings_file.save_int(default_radio, "dc", 1);
		}
		else
		{
			Demodulator::set_dc_filter(false);
			Settings_file.save_int(default_radio, "dc", 0);
		}
		Settings_file.write_settings();
	}
}

void gui_setup::brightness_slider_event_cb_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e); 

	int i = lv_slider_get_value(obj);
	if (i > 0)
	{		
		set_brightness(i);
	}
}

	
void gui_setup::audio_button_handler_class(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e); 
	if (code == LV_EVENT_VALUE_CHANGED) 
	{	char buf[30];
		
		int item = lv_dropdown_get_selected(obj);
		lv_dropdown_get_selected_str(obj,buf, sizeof(buf));
		destroy_demodulators();
		audio_input->close();
		audio_output->close();
		audio_input->open(std::string(buf));
		audio_output->open(std::string(buf));
		select_mode(mode, false);
		Settings_file.save_string("Audio", "device", std::string(buf));
		Settings_file.write_settings();
	}
}

void gui_setup::init(lv_obj_t *o_tab, lv_group_t *keyboard_group, lv_coord_t w, lv_coord_t h)
{

	const lv_coord_t x_page_margin = 5;
	const lv_coord_t x_margin  = 10;
	const lv_coord_t y_margin  = 5;
	const int y_number_buttons = 4;
	const lv_coord_t tab_margin  = 20;

	int x_number_buttons = 5;
	if (w > 800)
		x_number_buttons = 8;

	int button_width_margin = ((w - tab_margin) / x_number_buttons);
	int button_width = ((w - tab_margin) / x_number_buttons) - x_margin;
	int button_height = 50;
	int button_height_margin = button_height + y_margin;
	int	ibutton_x = 0, ibutton_y = 0;

	button_group = lv_group_create();
	lv_obj_set_style_pad_top(o_tab, 5, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(o_tab, 5, LV_PART_MAIN);
	lv_obj_set_style_pad_right(o_tab, 5, LV_PART_MAIN);

	tileview = lv_tileview_create(o_tab);
	lv_obj_clear_flag(tileview, LV_OBJ_FLAG_SCROLL_ELASTIC);	

	settings_main = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_BOTTOM | LV_DIR_TOP);	
	
	settings_i2c = lv_tileview_add_tile(tileview, 0, 1, LV_DIR_BOTTOM | LV_DIR_TOP);
	i2csetup.init(settings_i2c, w, h,button_group);
	
	settings_i2c_input = lv_tileview_add_tile(tileview, 0, 2, LV_DIR_BOTTOM | LV_DIR_TOP);
	gui_i2cinput.init(settings_i2c_input, w, h, button_group);
	
	lv_obj_set_tile_id(tileview, 0, 0, LV_ANIM_OFF);
	
	lv_style_init(&text_style);
	lv_style_set_radius(&text_style, 0);
	lv_style_set_bg_color(&text_style, lv_color_black());
	
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
	lv_obj_clear_flag(settings_main, LV_OBJ_FLAG_SCROLLABLE);
	
	int xpos = 0;

	qra_textarea = lv_textarea_create(settings_main);
	lv_obj_add_style(qra_textarea, &text_style, 0);
	lv_textarea_set_one_line(qra_textarea, true);
	lv_obj_align(qra_textarea, LV_ALIGN_TOP_LEFT, xpos, y_margin + ibutton_y * button_height_margin + button_height_margin / 2);
	lv_obj_add_event_cb(qra_textarea, qra_textarea_event_handler, LV_EVENT_ALL, qra_textarea);
	lv_obj_add_state(qra_textarea, LV_STATE_FOCUSED); /*To be sure the cursor is visible*/
	//lv_obj_set_width(qra_textarea, button_width - 20);
	lv_obj_set_size(qra_textarea, button_width - 20, button_height - 10);
	lv_obj_set_style_pad_top(qra_textarea, 4, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(qra_textarea, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_left(qra_textarea, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_right(qra_textarea, 2, LV_PART_MAIN);
	if (keyboard_group != nullptr)
		lv_group_add_obj(keyboard_group, qra_textarea);
	std::string call = Settings_file.get_string("wsjtx", "call");
	//locator = Settings_file.get_string("wsjtx", "locator");
	lv_textarea_add_text(qra_textarea, call.c_str());

	text_label = lv_label_create(settings_main);
	lv_label_set_text(text_label, "QRA");
	lv_obj_align_to(text_label, qra_textarea, LV_ALIGN_OUT_TOP_LEFT, 0, -10);
	
	d_audio = lv_dropdown_create(settings_main);
	lv_group_add_obj(button_group, d_audio);
	lv_obj_align(d_audio, LV_ALIGN_TOP_LEFT, xpos + button_width +x_margin, y_margin + ibutton_y * button_height_margin + button_height_margin / 2);
	lv_obj_set_width(d_audio, 2 * button_width); // 2*
	lv_dropdown_clear_options(d_audio);
	lv_obj_add_event_cb(d_audio, audio_button_handler, LV_EVENT_VALUE_CHANGED, (void *)this);
	// std::vector<std::string> devices = audio_output->getDeviceNames();
	std::vector<std::string> devices;
	audio_input->listDevices(devices);
	string s = Settings_file.find_audio("device");
	for (int i = 0; i < devices.size(); i++)
	{
		if (devices[i].length() > 0)
		{
			lv_dropdown_add_option(d_audio, devices[i].c_str(), LV_DROPDOWN_POS_LAST);
			if (s.length() > 0 && devices[i].find(s) != std::string::npos)
			{
				lv_dropdown_set_selected(d_audio, i);
			}
		}
	}
	
	audio_label = lv_label_create(settings_main);
	lv_label_set_text(audio_label, "Audio device");
	lv_obj_align_to(audio_label, d_audio, LV_ALIGN_OUT_TOP_LEFT, 0, -10);
	
	int y_cal = y_margin + ibutton_y * button_height_margin + button_height_margin / 2;
	calibration_dropdown = lv_dropdown_create(settings_main);
	lv_group_add_obj(button_group, calibration_dropdown);
	lv_obj_align(calibration_dropdown, LV_ALIGN_TOP_LEFT, w / 2 + w / 4, y_cal);
	lv_obj_set_width(calibration_dropdown, button_width); // 2*
	lv_dropdown_clear_options(calibration_dropdown);
	lv_obj_add_event_cb(calibration_dropdown, cal_button_handler, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_dropdown_set_options_static(calibration_dropdown, cal_opts);
	cal_label = lv_label_create(settings_main);
	lv_label_set_text(cal_label, "calibration");
	lv_obj_align_to(cal_label, calibration_dropdown, LV_ALIGN_OUT_TOP_LEFT, 0, -10);
	lv_dropdown_set_selected(calibration_dropdown, Settings_file.get_int(default_radio, "correction", 0));
	Demodulator::set_autocorrection(Settings_file.get_int(default_radio, "correction", 0));

	ibutton_y++;
	y_cal = y_margin + ibutton_y * button_height_margin + button_height_margin / 2;
	
	calbox = lv_checkbox_create(settings_main);
	lv_obj_align_to(calbox, settings_main, LV_ALIGN_TOP_LEFT, w / 2 + w / 4, y_cal);
	lv_checkbox_set_text(calbox, "cal");
	lv_obj_add_event_cb(calbox, calbox_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);

	webbox = lv_checkbox_create(settings_main);
	lv_obj_align_to(webbox, settings_main, LV_ALIGN_TOP_LEFT, w / 2 + w / 4 + w / 8, y_cal);
	lv_checkbox_set_text(webbox, "web");
	lv_obj_add_event_cb(webbox, webbox_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
	if (Settings_file.get_int("web", "enabled"))
	{
		lv_obj_add_state(webbox, LV_STATE_CHECKED);
	}	
	dcbox = lv_checkbox_create(settings_main);
	lv_obj_align_to(dcbox, settings_main, LV_ALIGN_TOP_LEFT, w / 2 + w / 4, y_cal + 50);
	lv_checkbox_set_text(dcbox, "dc filter");
	lv_obj_add_event_cb(dcbox, dcbox_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
	if (Settings_file.get_int(default_radio, "dc"))
	{
		lv_obj_add_state(dcbox, LV_STATE_CHECKED);
		Demodulator::set_dc_filter(true);
	}
	else
	{
		Demodulator::set_dc_filter(false);
	}

	//ibutton_y++;
	// xpos = xpos + x_margin + 2 * button_width;
	int y_span = y_margin + ibutton_y * button_height_margin + button_height_margin ;
	brightness_slider = lv_slider_create(settings_main);
	lv_group_add_obj(button_group, brightness_slider);
	lv_obj_set_width(brightness_slider, w / 2 - 50);
	// lv_obj_align_to(brightness_slider, span_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 40);
	lv_obj_align_to(brightness_slider, o_tab, LV_ALIGN_TOP_LEFT, 0, y_span);
	lv_obj_add_event_cb(brightness_slider, brightness_slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_slider_set_range(brightness_slider, 0, get_maxbrightness());
	lv_slider_set_value(brightness_slider, get_brightness(), LV_ANIM_ON);

	brightness_slider_label = lv_label_create(settings_main);
	lv_label_set_text(brightness_slider_label, "brightness");
	lv_obj_align_to(brightness_slider_label, brightness_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);
	
	lv_group_add_obj(button_group, lv_tabview_get_tab_btns(tabview_mid));
}

void gui_setup::qra_textarea_event_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_CLICKED && kb == nullptr)
	{
		//kb = lv_keyboard_create(lv_scr_act());
		//lv_keyboard_set_textarea(kb, qra_textarea);
		//Settings_file.save();
	}
}

void gui_setup::set_group()
{
	lv_indev_set_group(encoder_indev_t, button_group);
	lv_group_focus_obj(d_audio);
}

void gui_setup::set_brightness(int brightness)
{
	std::string			f{"/sys/class/backlight/10-0045/brightness"};
	std::ofstream		myfile;
		
	myfile.open(f);
	if (!myfile.is_open())
	{
		f = "/sys/class/backlight/rpi_backlight/brightness";
		myfile.open(f);	
	}
	if (!myfile.is_open())
	{
		f = "/sys/class/backlight/11-0045/brightness";
		myfile.open(f);
	}
	std::string buf = strlib::sprintf("%d", brightness);
	myfile << buf;
}

int gui_setup::get_brightness()
{
	int brightness;
	std::string		f{"/sys/class/backlight/10-0045/actual_brightness"};
	std::string		s;
	std::ifstream	myfile;
	
	myfile.open(f);
	if (!myfile.is_open())
	{
		f = "/sys/class/backlight/rpi_backlight/actual_brightness";
		myfile.open(f);	
	}
	if (!myfile.is_open())
	{
		f = "/sys/class/backlight/11-0045/actual_brightness";
		myfile.open(f);
	}
	if (myfile.is_open())
	{
		myfile >> s;
		brightness = atoi(s.c_str());
	}
	return brightness;
}

int gui_setup::get_maxbrightness()
{
	int brightness = 50;
	std::string f{"/sys/class/backlight/10-0045/max_brightness"};
	std::string s;
	std::ifstream myfile;

	myfile.open(f);
	if (!myfile.is_open())
	{
		f = "/sys/class/backlight/rpi_backlight/max_brightness";
		myfile.open(f);
	}
	if (!myfile.is_open())
	{
		f = "/sys/class/backlight/11-0045/max_brightness";
		myfile.open(f);
	}
	if (myfile.is_open())
	{
		myfile >> s;
		brightness = atoi(s.c_str());
	}
	return brightness;
}