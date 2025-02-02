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

void gui_setup::receivers_button_handler_class(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e); 
	if (code == LV_EVENT_VALUE_CHANGED) 
	{
		char	buf[80];
		std::string receiver;
		
		receiver.reserve(80);
		lv_dropdown_get_selected_str(obj,buf, 79);
		switch_sdrreceiver(std::string(buf));
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

void gui_setup::span_slider_event_cb_class(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e); 

	int i = lv_slider_get_value(obj) * 48;
	if (i > 0)
	{		
		set_span_value(i * 1000);
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

	
int gui_setup::get_sample_rate(int rate)
{
	if (rate >= 0 && rate < sample_rates.size())
	{
		return sample_rates.at(rate);
	}
	return 0;
}

void gui_setup::decimate_button_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		set_samplerate();
	}
}

void gui_setup::samplerate_button_handler_class(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		set_samplerate();
	}
}

void gui_setup::set_samplerate()
{
	int decimate = lv_dropdown_get_selected(d_decimate);
	Settings_file.save_int(default_radio, "decimate", decimate);
	Settings_file.write_settings();
	
	int rate = lv_dropdown_get_selected(d_samplerate);
	ifrate = (double)get_sample_rate(rate) / pow(2, decimate);
	
	Settings_file.save_int(default_radio, "samplerate", get_sample_rate(rate) / 1000);
	Settings_file.write_settings();

	float rx_ifrate = ifrate * pow(2, decimate);
	printf("ifrate %f rxifrate %f \n", ifrate, rx_ifrate);
	if (SdrDevices.SdrDevices[default_radio]->get_bandwith_count(0) > 0)
	{
		long bw = 0L;

		int sel = gsetup.get_bandwidth_sel();
		bw = SdrDevices.SdrDevices.at(default_radio)->get_bandwith(0, sel);
		SdrDevices.SdrDevices.at(default_radio)->setBandwidth(SOAPY_SDR_RX, 0, bw);
		vfo.vfo_re_init((long)ifrate, span, audio_output->get_samplerate(), bw);
		printf("setBandwidth %ld \n", bw);
	}
	else
		vfo.vfo_re_init((long)ifrate, span, audio_output->get_samplerate(), 0L);

	destroy_demodulators(true);
	try
	{
		SdrDevices.SdrDevices.at(default_radio)->setSampleRate(SOAPY_SDR_RX, gsetup.get_current_rx_channel(), rx_ifrate);
	}
	catch (const std::exception &e)
	{
		std::cout << e.what() << endl;
		return;
	}
	select_mode(mode);
	set_span_range(ifrate);
	set_span_value(ifrate);
}

int gui_setup::get_decimation()
{
	return lv_dropdown_get_selected(d_decimate);
}

void gui_setup::bandwidth_button_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		int sel = lv_dropdown_get_selected(obj);
		if (sel <= SdrDevices.SdrDevices.at(default_radio)->get_bandwith_count(0))
		{
			long bw = SdrDevices.SdrDevices.at(default_radio)->get_bandwith(0, sel);
			SdrDevices.SdrDevices[default_radio]->setBandwidth(SOAPY_SDR_RX, 0, bw);
			//vfo.vfo_re_init((long)ifrate, span, audio_output->get_samplerate(), bw);
			//printf("setBandwidth %ld \n", bw);
		}
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
	}
}

void gui_setup::add_sample_rate(int samplerate)
{
	sample_rates.push_back(samplerate);
	std::string buf = strlib::sprintf("%d Khz", samplerate / 1000);
	lv_dropdown_add_option(d_samplerate, buf.c_str(), LV_DROPDOWN_POS_LAST);
}

void gui_setup::set_sample_rate(int rate)
{
	int i;
	for (i = 0; i < sample_rates.size(); i++)
	{
		if (sample_rates[i] == rate)
			break;
	}
	lv_dropdown_set_selected(d_samplerate, i);
	lv_dropdown_set_selected(d_decimate, Settings_file.get_int(default_radio, "decimate", 0));
}

void gui_setup::clear_sample_rate()
{
	sample_rates.clear();
	lv_dropdown_clear_options(d_samplerate);
}

void gui_setup::set_radio(std::string name)
{
	int i = SdrDevices.get_device_index(name);
	lv_dropdown_set_selected(d_receivers, i);
}

void gui_setup::init_bandwidth()
{
	lv_dropdown_clear_options(d_bandwitdth);
	try
	{

		for (int i = 0; i < SdrDevices.SdrDevices.at(default_radio)->get_bandwith_count(0); i++)
		{
			long bw = SdrDevices.SdrDevices[default_radio]->get_bandwith(0, i);
			std::string buf = strlib::sprintf("%ld Khz", bw / 1000);
			lv_dropdown_add_option(d_bandwitdth, buf.c_str(), LV_DROPDOWN_POS_LAST);
			if (i == 0)
				lv_dropdown_set_selected(d_bandwitdth, 0);
		}
	}
	catch (const std::exception &e)
	{
		std::cout << e.what();
	}
}

int gui_setup::get_current_rx_channel()
{
	return 0;
}

int gui_setup::get_current_tx_channel()
{
	return 0;
}

void gui_setup::init(lv_obj_t *o_tab, lv_coord_t w, lv_coord_t h, AudioOutput &audioDevice)
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
	lv_obj_set_style_pad_bottom(o_tab, 5, LV_PART_MAIN);
	lv_obj_set_style_pad_right(o_tab, 5, LV_PART_MAIN);

	tileview = lv_tileview_create(o_tab);
	lv_obj_clear_flag(tileview, LV_OBJ_FLAG_SCROLL_ELASTIC);	

	settings_main = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_BOTTOM | LV_DIR_TOP);	
	
	settings_i2c = lv_tileview_add_tile(tileview, 0, 1, LV_DIR_BOTTOM | LV_DIR_TOP);
	i2csetup.init(settings_i2c, w, h,button_group);

	lv_obj_set_tile_id(tileview, 0, 0, LV_ANIM_OFF);

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
	
	d_samplerate = lv_dropdown_create(settings_main);
	lv_group_add_obj(button_group, d_samplerate);
	lv_obj_align(d_samplerate, LV_ALIGN_TOP_LEFT, x_page_margin, y_margin + ibutton_y * button_height_margin);
	lv_dropdown_clear_options(d_samplerate);
	lv_obj_add_event_cb(d_samplerate, samplerate_button_handler, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_obj_set_width(d_samplerate, button_width);

	int xpos = x_page_margin + button_width + x_margin;
	d_decimate = lv_dropdown_create(settings_main);
	lv_group_add_obj(button_group, d_decimate);
	lv_obj_align(d_decimate, LV_ALIGN_TOP_LEFT, xpos , y_margin + ibutton_y * button_height_margin);
	lv_dropdown_clear_options(d_decimate);
	lv_obj_add_event_cb(d_decimate, decimate_button_handler, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_obj_set_width(d_decimate, 0.5 * button_width);
	for (int i = 0; i < 6; i++)
	{
		std::string decimation = std::to_string((int)floor(pow(2, i)));
		lv_dropdown_add_option(d_decimate, decimation.c_str(), LV_DROPDOWN_POS_LAST);
	}
	lv_dropdown_set_selected(d_decimate, Settings_file.get_int(default_radio, "decimate", 0));
	
	xpos = 0.5 * button_width + xpos + x_margin;
	d_receivers = lv_dropdown_create(settings_main);
	lv_group_add_obj(button_group, d_receivers);
	lv_obj_align(d_receivers, LV_ALIGN_TOP_LEFT, xpos, y_margin + ibutton_y * button_height_margin);
	lv_dropdown_clear_options(d_receivers);
	lv_obj_add_event_cb(d_receivers, receivers_button_handler, LV_EVENT_VALUE_CHANGED, (void *)this);
	std::string def = Settings_file.find_sdr("default");
	for (auto& col : Settings_file.receivers)
	{
		lv_dropdown_add_option(d_receivers, col.c_str(), LV_DROPDOWN_POS_LAST);
	}
	lv_obj_set_width(d_receivers, button_width);

	xpos = xpos + x_margin + button_width;
	d_audio = lv_dropdown_create(settings_main);
	lv_group_add_obj(button_group, d_audio);
	lv_obj_align(d_audio, LV_ALIGN_TOP_LEFT, xpos, y_margin + ibutton_y * button_height_margin);
	lv_obj_set_width(d_audio, 2 * button_width); // 2*
	lv_dropdown_clear_options(d_audio);
	lv_obj_add_event_cb(d_audio, audio_button_handler, LV_EVENT_VALUE_CHANGED, (void *)this);
	std::vector<std::string> devices = audio_output->getDeviceNames();
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

	xpos = xpos + x_margin + 2 * button_width;
	d_bandwitdth = lv_dropdown_create(settings_main);
	lv_group_add_obj(button_group, d_bandwitdth);
	lv_obj_align(d_bandwitdth, LV_ALIGN_TOP_LEFT, xpos, y_margin + ibutton_y * button_height_margin);
	lv_obj_set_width(d_bandwitdth, button_width); // 2*
	lv_dropdown_clear_options(d_bandwitdth);
	lv_obj_add_event_cb(d_bandwitdth, bandwidth_button_handler, LV_EVENT_VALUE_CHANGED, (void *)this);

	ibutton_y++;
	int y_span = y_margin + ibutton_y * button_height_margin + button_height_margin / 2;
	int brightness_y = 15 + y_margin + 2 * button_height_margin;
	span_slider = lv_slider_create(settings_main);
	lv_group_add_obj(button_group, span_slider);
	lv_obj_set_width(span_slider, w / 2 - 50);
	//lv_obj_center(span_slider);
	//lv_obj_align(span_slider, LV_ALIGN_TOP_MID, 0, y_span);
	lv_obj_align_to(span_slider, settings_main, LV_ALIGN_TOP_LEFT, 0, y_span);

	lv_obj_add_event_cb(span_slider, span_slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);

	span_slider_label = lv_label_create(settings_main);
	lv_label_set_text(span_slider_label, "span 500 Khz");
	//lv_obj_align_to(span_slider_label, span_slider, LV_ALIGN_OUT_TOP_MID, -30, -10);
	lv_obj_align_to(span_slider_label, span_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);

	brightness_slider = lv_slider_create(settings_main);
	lv_group_add_obj(button_group, brightness_slider);
	lv_obj_set_width(brightness_slider, w / 2 - 50);
	lv_obj_align_to(brightness_slider, span_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 40);
	lv_obj_add_event_cb(brightness_slider, brightness_slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_slider_set_range(brightness_slider, 0, get_maxbrightness());
	lv_slider_set_value(brightness_slider, get_brightness(), LV_ANIM_ON);

	brightness_slider_label = lv_label_create(settings_main);
	lv_label_set_text(brightness_slider_label, "brightness");
	lv_obj_align_to(brightness_slider_label, brightness_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);

	calibration_dropdown = lv_dropdown_create(settings_main);
	lv_group_add_obj(button_group, calibration_dropdown);
	lv_obj_align(calibration_dropdown, LV_ALIGN_TOP_LEFT, w / 2, y_span);
	lv_obj_set_width(calibration_dropdown, button_width); // 2*
	lv_dropdown_clear_options(calibration_dropdown);
	lv_obj_add_event_cb(calibration_dropdown, cal_button_handler, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_dropdown_set_options_static(calibration_dropdown, cal_opts);
	cal_label = lv_label_create(settings_main);
	lv_label_set_text(cal_label, "calibration");
	lv_obj_align_to(cal_label, calibration_dropdown, LV_ALIGN_OUT_TOP_LEFT, 0, -10);
	lv_dropdown_set_selected(calibration_dropdown, Settings_file.get_int(default_radio, "correction", 0));
	Demodulator::set_autocorrection(Settings_file.get_int(default_radio, "correction", 0));

	calbox = lv_checkbox_create(settings_main);
	lv_obj_align_to(calbox, settings_main, LV_ALIGN_TOP_LEFT, w / 2 + w / 4, y_span);
	lv_checkbox_set_text(calbox, "cal");
	lv_obj_add_event_cb(calbox, calbox_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);

	webbox = lv_checkbox_create(settings_main);
	lv_obj_align_to(webbox, settings_main, LV_ALIGN_TOP_LEFT, w / 2 + w / 4 + w / 8, y_span);
	lv_checkbox_set_text(webbox, "web");
	lv_obj_add_event_cb(webbox, webbox_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
	if (Settings_file.get_int("web", "enabled"))
	{
		lv_obj_add_state(webbox, LV_STATE_CHECKED);
	}	
	dcbox = lv_checkbox_create(settings_main);
	lv_obj_align_to(dcbox, settings_main, LV_ALIGN_TOP_LEFT, w / 2 + w / 4, y_span + 50);
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
	lv_group_add_obj(button_group, lv_tabview_get_tab_btns(tabview_mid));
}

void gui_setup::set_group()
{
	lv_indev_set_group(encoder_indev_t, button_group);
	lv_group_focus_obj(d_samplerate);
}

void gui_setup::set_span_value(long span)
{
	std::string	buf;
	
	int maxv = lv_slider_get_max_value(span_slider);
	int v = span / 48000;
	
	if(v < 0 || v > maxv)
		span = maxv;
	if (v > 0)
	{
		if ((ifrate - (double)span) < 0.1)
		{
			lv_slider_set_value(span_slider, maxv, LV_ANIM_ON);
			span = ifrate;
			buf = strlib::sprintf("span %ld Khz", span / 1000);
			gui_vfo_inst.set_span(span / 1000);			
		}
		else
		{
			lv_slider_set_value(span_slider, v, LV_ANIM_ON);
			buf = strlib::sprintf("span %d Khz", v * 48);
			gui_vfo_inst.set_span(v * 48);
		}
	}
	else
	{
		lv_slider_set_value(span_slider, 1, LV_ANIM_ON);
		buf = strlib::sprintf("span %ld Khz", span / 1000);		
		gui_vfo_inst.set_span(span / 1000);
	}
	lv_label_set_text(span_slider_label, buf.c_str());
	// store in atomic<int> so demodulator thread can request it
	gui_setup::span = span;
	vfo.set_span(span);
	vfo.set_vfo(vfo.get_frequency());
	SpectrumGraph.SetFftParts();
	gbar.updateweb();
	Settings_file.save_int(default_radio, "span", span / 1000);
	Settings_file.write_settings();
}

void gui_setup::set_span_range(long span)
{
	int v = span / 48000;
	int m = span % 48000;
	if (v < 0 || v > 80)
		span = 80;
	if (v == 0)
		span = 1;
	if (m > 0)
		v++;
	lv_slider_set_range(span_slider, 1, v);
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

		myfile >> s;
	}
	brightness = atoi(s.c_str());
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

		myfile >> s;
	}
	if (myfile.is_open())
		brightness = atoi(s.c_str());
	return brightness;
}