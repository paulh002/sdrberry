#include "gui_setup.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>

gui_setup	gsetup;
extern 		void switch_sdrreceiver(std::string receiver);

static void receivers_button_handler(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e); 
	if (code == LV_EVENT_VALUE_CHANGED) 
	{
		char	buf[80];
		std::string receiver;
		
		receiver.reserve(80);
		int selection = lv_dropdown_get_selected(obj);
		lv_dropdown_get_selected_str(obj,buf, 79);
		switch_sdrreceiver(std::string(buf));
	}
}

static void span_slider_event_cb(lv_event_t * e)
{
	char buf[20]; 
	
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e); 

	int i = lv_slider_get_value(obj) * 50;
	if (i > 0)
	{		
		gsetup.set_span_value(i * 1000);
	}
}
	
static void brightness_slider_event_cb(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e); 

	int i = lv_slider_get_value(obj);
	if (i > 0)
	{		
		gsetup.set_brightness(i);
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

static void samplerate_button_handler(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e); 
	if (code == LV_EVENT_VALUE_CHANGED) 
	{
		int rate = lv_dropdown_get_selected(obj);
		ifrate = gsetup.get_sample_rate(rate);
		gsetup.m_ifrate = ifrate;
		vfo.vfo_re_init((long)ifrate, pcmrate);
		stop_rxtx();
		try
		{
			SdrDevices.SdrDevices.at(default_radio)->setSampleRate(SOAPY_SDR_RX, default_rx_channel, ifrate);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << endl;
			return;
		}
		gsetup.set_span_range(ifrate/2);
		gsetup.set_span_value(ifrate/2);
		select_mode(mode);
	}
}

static void event_handler_morse(lv_event_t *e)
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

void gui_setup::set_cw(bool bcw)
{
	if (bcw)
		lv_obj_add_state(check_cw, LV_STATE_CHECKED);
	else
		lv_obj_clear_state(check_cw, LV_STATE_CHECKED);
	lv_event_send(check_cw, LV_EVENT_VALUE_CHANGED, nullptr);
}

bool gui_setup::get_cw()
{
	return lv_obj_get_state(check_cw) & LV_STATE_CHECKED;
}

static void audio_button_handler(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e); 
	if (code == LV_EVENT_VALUE_CHANGED) 
	{	char buf[30];
		
		int item = lv_dropdown_get_selected(obj);
		lv_dropdown_get_selected_str(obj,buf, sizeof(buf));
		stop_rxtx();
		audio_output->close();
		audio_output->init_device(std::string(buf));
		audio_output->open();
		audio_input->close();
		audio_input->init_device(std::string(buf));
		audio_input->open();
		select_mode(mode, false);
	}
}

void gui_setup::add_sample_rate(int samplerate)
{
	char	str[30];
	
	sample_rates.push_back(samplerate);
	sprintf(str, "%d Khz", samplerate / 1000);
	lv_dropdown_add_option(d_samplerate, str, LV_DROPDOWN_POS_LAST);
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

void gui_setup::init(lv_obj_t* o_tab, lv_coord_t w)
{

	const lv_coord_t x_margin  = 10;
	const lv_coord_t y_margin  = 5;
	const int x_number_buttons = 5;
	const int y_number_buttons = 4;
	const lv_coord_t tab_margin  = 20;
	
	int button_width_margin = ((w - tab_margin) / x_number_buttons);
	int button_width = ((w - tab_margin) / x_number_buttons) - x_margin;
	int button_height = 50;
	int button_height_margin = button_height + y_margin;
	int	ibutton_x = 0, ibutton_y = 0;
	m_ifrate = ifrate;
	
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
	
	d_samplerate = lv_dropdown_create(o_tab);
	lv_obj_align(d_samplerate, LV_ALIGN_TOP_LEFT, 0, y_margin + ibutton_y * button_height_margin);
	lv_dropdown_clear_options(d_samplerate);
	lv_obj_add_event_cb(d_samplerate, samplerate_button_handler, LV_EVENT_VALUE_CHANGED, NULL);
	
	d_receivers = lv_dropdown_create(o_tab);
	lv_obj_align(d_receivers, LV_ALIGN_TOP_LEFT, button_width_margin, y_margin + ibutton_y * button_height_margin);
	lv_dropdown_clear_options(d_receivers);
	lv_obj_add_event_cb(d_receivers, receivers_button_handler, LV_EVENT_VALUE_CHANGED, NULL);
	std::string def = Settings_file.find_sdr("default");
	for (auto& col : Settings_file.receivers)
	{
		lv_dropdown_add_option(d_receivers, col.c_str(), LV_DROPDOWN_POS_LAST);
	}
	
	d_audio = lv_dropdown_create(o_tab);
	lv_obj_align(d_audio, LV_ALIGN_TOP_LEFT, 2*button_width_margin, y_margin + ibutton_y * button_height_margin);
	lv_obj_set_width(d_audio, 2*button_width);
	lv_dropdown_clear_options(d_audio);
	lv_obj_add_event_cb(d_audio, audio_button_handler, LV_EVENT_VALUE_CHANGED, NULL);
	std::vector<std::string> devices;
	audio_output->listDevices(devices);
	string s = Settings_file.find_audio("device");	
	for (int i = 0; i < devices.size(); i++)
	{
		if (devices[i].length() > 0)
		{
			lv_dropdown_add_option(d_audio, devices[i].c_str(), LV_DROPDOWN_POS_LAST);
			if (s.length()> 0 && devices[i].find(s) != std::string::npos)
			{
				lv_dropdown_set_selected(d_audio, i);	
			}
		}
	}
	//int span_y = 15 + y_margin + button_height_margin;
	ibutton_y++;
	int y_span = y_margin + ibutton_y * button_height_margin + button_height_margin /2;
	span_slider = lv_slider_create(o_tab);
	lv_obj_set_width(span_slider, w / 2 - 50); 
	//lv_obj_center(span_slider);
	lv_obj_align(span_slider, LV_ALIGN_TOP_MID, 0, y_span);

	lv_obj_add_event_cb(span_slider, span_slider_event_cb, LV_EVENT_PRESSING, NULL);

	span_slider_label = lv_label_create(o_tab);
	lv_label_set_text(span_slider_label, "span");
	lv_obj_align_to(span_slider_label, span_slider, LV_ALIGN_OUT_TOP_MID, -30, -10);
	
	string span = Settings_file.find_radio("span");
	int i = atoi(span.c_str());
	if (((i * 1000) > (ifrate / 2)) || i == 0)
		i = ifrate / 2000;
	set_span_range(ifrate/2);
	set_span_value(i * 1000);
	
	int brightness_y = 15 + y_margin + 2* button_height_margin;
	brightness_slider = lv_slider_create(o_tab);
	lv_obj_set_width(brightness_slider, w / 2 - 50); 
	lv_obj_align_to(brightness_slider, span_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 40);
	lv_obj_add_event_cb(brightness_slider, brightness_slider_event_cb, LV_EVENT_PRESSING, NULL);
	lv_slider_set_range(brightness_slider, 0, 255);
	lv_slider_set_value(brightness_slider, get_brightness(), LV_ANIM_ON);
	
	brightness_slider_label = lv_label_create(o_tab);
	lv_label_set_text(brightness_slider_label, "brightness");
	lv_obj_align_to(brightness_slider_label, brightness_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);

	check_cw = lv_checkbox_create(o_tab);
	lv_checkbox_set_text(check_cw, "Morse Decoder");
	lv_obj_add_event_cb(check_cw, event_handler_morse, LV_EVENT_ALL, NULL);
	lv_obj_align_to(check_cw, d_samplerate, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
	//lv_obj_add_state(check_cw, LV_STATE_CHECKED);
}

void gui_setup::set_span_value(int span)
{
	char	buf[30];
	
	int maxv = lv_slider_get_max_value(span_slider);
	int v = span / 50000;
	
	if(v < 0 || v > maxv)
		span = maxv;
	if (v > 0)
	{	// the highest span is limited by ifrate/2
		if (((m_ifrate / 2) - (double)span) < 0.1)
		{
			lv_slider_set_value(span_slider, maxv, LV_ANIM_ON);
			span = m_ifrate / 2;
			sprintf(buf, "span %d Khz", span / 1000);
			gui_vfo_inst.set_span(span / 1000);			
		}
		else
		{
			lv_slider_set_value(span_slider, v, LV_ANIM_ON);
			sprintf(buf, "span %d Khz", v * 50);
			gui_vfo_inst.set_span(v * 50);
		}
	}
	else
	{
		lv_slider_set_value(span_slider, 1, LV_ANIM_ON);
		sprintf(buf, "span %d Khz", span / 1000);		
		gui_vfo_inst.set_span(span / 1000);
	}
	lv_label_set_text(span_slider_label, buf);
	// store in atomic<int> so demodulator thread can request it
	m_span.store(span);	
}

void gui_setup::set_span_range(int span)
{
	char	buf[30];
	
	int v = span / 50000;
	int m = span % 50000;
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
	char				buf[20];
	std::ofstream		myfile;
		
	myfile.open(f);
	if (!myfile.is_open())
	{
		f = "/sys/class/backlight/rpi_backlight/brightness";
		myfile.open(f);	
	}
	sprintf(buf,"%d", brightness);
	myfile << std::string(buf);
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
	myfile >> s;
	brightness = atoi(s.c_str());
	return brightness;
}