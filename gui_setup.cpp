#include "gui_setup.h"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <string>
#include "Spectrum.h"
#include "gui_cal.h"
#include "gui_ft8bar.h"
#include "gui_i2csetup.h"

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
		lv_dropdown_get_selected_str(obj,buf, 79);
		switch_sdrreceiver(std::string(buf));
	}
}

static void calbox_event_cb(lv_event_t *e)
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



static void contour_slider_event_cb(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);

	int i = lv_slider_get_value(obj);
	if (i > 0)
	{
		gsetup.set_contour_value(i);
	}
}

static void floor_slider_event_cb(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);

	int i = lv_slider_get_value(obj);
	if (i > 0)
	{
		gsetup.set_floor_value(i);
	}
}

static void span_slider_event_cb(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e); 

	int i = lv_slider_get_value(obj) * 48;
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
		gsetup.set_span_range(ifrate);
		gsetup.set_span_value(ifrate);
		Settings_file.save_int(default_radio, "samplerate", ifrate / 1000);
		Settings_file.write_settings();

		if (SdrDevices.SdrDevices[default_radio]->get_bandwith_count(0) > 0)
		{
			long bw = 0L;

			int sel = gsetup.get_bandwidth_sel();
			bw = SdrDevices.SdrDevices.at(default_radio)->get_bandwith(0, sel);
			SdrDevices.SdrDevices.at(default_radio)->setBandwidth(SOAPY_SDR_RX, 0, bw);
			vfo.vfo_re_init((long)ifrate, gsetup.get_span(), audio_output->get_samplerate(), bw);
			printf("setBandwidth %ld \n", bw);
		}
		else
			vfo.vfo_re_init((long)ifrate, gsetup.get_span(), audio_output->get_samplerate(), 0L);

		destroy_demodulators(true);
		try
		{
			SdrDevices.SdrDevices.at(default_radio)->setSampleRate(SOAPY_SDR_RX, gsetup.get_current_rx_channel(), ifrate);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << endl;
			return;
		}
		select_mode(mode);
	}
}

static void bandwidth_button_handler(lv_event_t *e)
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
			vfo.vfo_re_init((long)ifrate, gsetup.get_span(), audio_output->get_samplerate(), bw);
			printf("setBandwidth %ld \n", bw);
		}
	}
}

static void audio_button_handler(lv_event_t * e)
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

void gui_setup::init_bandwidth()
{
	lv_dropdown_clear_options(d_bandwitdth);
	try
	{

		for (int i = 0; i < SdrDevices.SdrDevices.at(default_radio)->get_bandwith_count(0); i++)
		{
			char buf[80];

			long bw = SdrDevices.SdrDevices[default_radio]->get_bandwith(0, i);
			sprintf(buf, "%ld Khz", bw / 1000);
			lv_dropdown_add_option(d_bandwitdth, buf, LV_DROPDOWN_POS_LAST);
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
	const int x_number_buttons = 5;
	const int y_number_buttons = 4;
	const lv_coord_t tab_margin  = 20;
	
	int button_width_margin = ((w - tab_margin) / x_number_buttons);
	int button_width = ((w - tab_margin) / x_number_buttons) - x_margin;
	int button_height = 50;
	int button_height_margin = button_height + y_margin;
	int	ibutton_x = 0, ibutton_y = 0;
	m_ifrate = ifrate;

	m_button_group = lv_group_create();
	lv_obj_set_style_pad_bottom(o_tab, 5, LV_PART_MAIN);
	lv_obj_set_style_pad_right(o_tab, 5, LV_PART_MAIN);

	tileview = lv_tileview_create(o_tab);
	lv_obj_clear_flag(tileview, LV_OBJ_FLAG_SCROLL_ELASTIC);	

	settings_main = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_BOTTOM | LV_DIR_TOP);	
	
	settings_i2c = lv_tileview_add_tile(tileview, 0, 1, LV_DIR_BOTTOM | LV_DIR_TOP);
	i2csetup.init(settings_i2c, w, h,m_button_group);

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
	lv_group_add_obj(m_button_group, d_samplerate);
	lv_obj_align(d_samplerate, LV_ALIGN_TOP_LEFT, x_page_margin, y_margin + ibutton_y * button_height_margin);
	lv_dropdown_clear_options(d_samplerate);
	lv_obj_add_event_cb(d_samplerate, samplerate_button_handler, LV_EVENT_VALUE_CHANGED, NULL);
	
	d_receivers = lv_dropdown_create(settings_main);
	lv_group_add_obj(m_button_group, d_receivers);
	lv_obj_align(d_receivers, LV_ALIGN_TOP_LEFT, button_width_margin, y_margin + ibutton_y * button_height_margin);
	lv_dropdown_clear_options(d_receivers);
	lv_obj_add_event_cb(d_receivers, receivers_button_handler, LV_EVENT_VALUE_CHANGED, NULL);
	std::string def = Settings_file.find_sdr("default");
	for (auto& col : Settings_file.receivers)
	{
		lv_dropdown_add_option(d_receivers, col.c_str(), LV_DROPDOWN_POS_LAST);
	}

	d_audio = lv_dropdown_create(settings_main);
	lv_group_add_obj(m_button_group, d_audio);
	lv_obj_align(d_audio, LV_ALIGN_TOP_LEFT, 2 * button_width_margin, y_margin + ibutton_y * button_height_margin);
	lv_obj_set_width(d_audio, 1.5 * button_width); // 2*
	lv_dropdown_clear_options(d_audio);
	lv_obj_add_event_cb(d_audio, audio_button_handler, LV_EVENT_VALUE_CHANGED, NULL);
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

	d_bandwitdth = lv_dropdown_create(settings_main);
	lv_group_add_obj(m_button_group, d_bandwitdth);
	lv_obj_align(d_bandwitdth, LV_ALIGN_TOP_LEFT, 3.5 * button_width_margin, y_margin + ibutton_y * button_height_margin);
	lv_obj_set_width(d_bandwitdth, button_width); // 2*
	lv_dropdown_clear_options(d_bandwitdth);
	lv_obj_add_event_cb(d_bandwitdth, bandwidth_button_handler, LV_EVENT_VALUE_CHANGED, NULL);

	ibutton_y++;
	int y_span = y_margin + ibutton_y * button_height_margin + button_height_margin / 2;
	int brightness_y = 15 + y_margin + 2 * button_height_margin;
	span_slider = lv_slider_create(settings_main);
	lv_group_add_obj(m_button_group, span_slider);
	lv_obj_set_width(span_slider, w / 2 - 50);
	//lv_obj_center(span_slider);
	//lv_obj_align(span_slider, LV_ALIGN_TOP_MID, 0, y_span);
	lv_obj_align_to(span_slider, settings_main, LV_ALIGN_TOP_LEFT, 0, y_span);

	lv_obj_add_event_cb(span_slider, span_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

	span_slider_label = lv_label_create(settings_main);
	lv_label_set_text(span_slider_label, "span 500 Khz");
	//lv_obj_align_to(span_slider_label, span_slider, LV_ALIGN_OUT_TOP_MID, -30, -10);
	lv_obj_align_to(span_slider_label, span_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);


	int span = Settings_file.get_int(default_radio, "span");
	if (((span * 1000) > (ifrate / 1)) || span == 0)
		span = ifrate / 1000;
	set_span_range(ifrate);
	set_span_value(span * 1000);

	//lv_obj_t *contour_slider_label, *contour_slider;
	//lv_obj_t *floor_slider_label, *floor_slider;

	contour_slider = lv_slider_create(settings_main);
	lv_group_add_obj(m_button_group, contour_slider);
	lv_obj_set_width(contour_slider, w / 4 - 50);
	lv_obj_align_to(contour_slider, settings_main, LV_ALIGN_TOP_LEFT, w / 2, y_span);
	lv_slider_set_range(contour_slider, 1, 10);

	lv_obj_add_event_cb(contour_slider, contour_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	contour_slider_label = lv_label_create(settings_main);
	lv_label_set_text(contour_slider_label, "speed 1");
	lv_obj_align_to(contour_slider_label, contour_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);

	brightness_slider = lv_slider_create(settings_main);
	lv_group_add_obj(m_button_group, brightness_slider);
	lv_obj_set_width(brightness_slider, w / 2 - 50);
	lv_obj_align_to(brightness_slider, span_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 40);
	lv_obj_add_event_cb(brightness_slider, brightness_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	lv_slider_set_range(brightness_slider, 0, 255);
	lv_slider_set_value(brightness_slider, get_brightness(), LV_ANIM_ON);

	brightness_slider_label = lv_label_create(settings_main);
	lv_label_set_text(brightness_slider_label, "brightness");
	lv_obj_align_to(brightness_slider_label, brightness_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);

	floor_slider = lv_slider_create(settings_main);
	lv_group_add_obj(m_button_group, floor_slider);
	lv_obj_set_width(floor_slider, w / 4 - 50);
	lv_obj_align_to(floor_slider, span_slider, LV_ALIGN_OUT_BOTTOM_LEFT, w / 2, 40);
	lv_slider_set_range(floor_slider, 1, 20);

	lv_obj_add_event_cb(floor_slider, floor_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	floor_slider_label = lv_label_create(settings_main);
	lv_label_set_text(floor_slider_label, "Noise floor 1");
	lv_obj_align_to(floor_slider_label, floor_slider, LV_ALIGN_OUT_TOP_MID, 0, -10);

	calbox = lv_checkbox_create(settings_main);
	lv_obj_align_to(calbox, settings_main, LV_ALIGN_TOP_LEFT, w / 2 + w / 4, y_span);
	lv_checkbox_set_text(calbox, "calibration");
	lv_obj_add_event_cb(calbox, calbox_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	if (!Settings_file.get_int(default_radio, "correction"))
	{
		lv_obj_add_flag(calbox, LV_OBJ_FLAG_HIDDEN);
	}
	lv_group_add_obj(m_button_group, lv_tabview_get_tab_btns(tabview_mid));
}

void gui_setup::set_group()
{
	lv_indev_set_group(encoder_indev_t, m_button_group);
	lv_group_focus_obj(d_samplerate);
}

void gui_setup::set_span_value(int span)
{
	char	buf[30];
	
	int maxv = lv_slider_get_max_value(span_slider);
	int v = span / 48000;
	
	if(v < 0 || v > maxv)
		span = maxv;
	if (v > 0)
	{
		if ((m_ifrate - (double)span) < 0.1)
		{
			lv_slider_set_value(span_slider, maxv, LV_ANIM_ON);
			span = m_ifrate;
			sprintf(buf, "span %d Khz", span / 1000);
			gui_vfo_inst.set_span(span / 1000);			
		}
		else
		{
			lv_slider_set_value(span_slider, v, LV_ANIM_ON);
			sprintf(buf, "span %d Khz", v * 48);
			gui_vfo_inst.set_span(v * 48);
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
	vfo.set_span(span);
	m_span.store(span);
	SpectrumGraph.SetSpan(span);

	Settings_file.save_int(default_radio, "span", span / 1000);
	Settings_file.write_settings();
}

void gui_setup::set_span_range(int span)
{
	char	buf[30];
	
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

void gui_setup::set_contour_value(int speed)
{
	char buf[20];

	sprintf(buf, " speed %d ",speed);
	lv_label_set_text(contour_slider_label, buf);
}

int gui_setup::get_contour_value()
{
	return lv_slider_get_value(contour_slider);
}

void gui_setup::set_floor_value(int speed)
{
	char buf[30];

	sprintf(buf, " noise floor %d ", speed);
	lv_label_set_text(floor_slider_label, buf);
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