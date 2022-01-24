#include "gui_bar.h"

static const char * opts = "0.5 Khz\n"
						   "1.0 Khz\n"
						   "1.5 Khz\n"
						   "2.0 Khz\n"
						   "2.5 Khz\n"
						   "3.0 Khz\n"
						   "3.5 Khz\n"
						   "4.0 Khz";

gui_bar gbar;

static void bar_button_handler(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e); 
	
	int bmode = (int)lv_obj_get_user_data(obj);
	
	if (code == LV_EVENT_CLICKED) {
	
		for (int i = 0; i < gbar.getbuttons(); i++)
		{
			if ((obj != gbar.get_button_obj(i)) && (lv_obj_has_flag(gbar.get_button_obj(i), LV_OBJ_FLAG_CHECKABLE)))
			{
				if (bmode)
					lv_obj_clear_state(gbar.get_button_obj(i), LV_STATE_CHECKED);
			}
			if (obj == gbar.get_button_obj(i)) 
			{
				switch (i)
				{
				case 0:
					if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
						select_mode_tx(mode);
					else
						select_mode(mode);
					break;
				case 1:
				case 2:
				case 3:
				case 4:
				case 5:
					select_mode(bmode);
					lv_obj_add_state(obj, LV_STATE_CHECKED);		
					Gui_rx.set_gui_mode(bmode);
					break;
				case 7:
					if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
					{
						//	gagc.set_agc_mode(1);
						try
						{
							if (SdrDevices.SdrDevices.at(default_radio)->rx_channels.at(default_rx_channel)->get_agc())
							{
								SdrDevices.SdrDevices.at(default_radio)->setGainMode(SOAPY_SDR_RX, default_rx_channel, true);
							}
						}
						catch (const std::exception& e)
						{
							std::cout << e.what();
						}
					}
					else
					{
						try
						{
							if (SdrDevices.SdrDevices.at(default_radio)->rx_channels.at(default_rx_channel)->get_agc())
							{
								SdrDevices.SdrDevices.at(default_radio)->setGainMode(SOAPY_SDR_RX, default_rx_channel, false);
							}
						}
						catch (const std::exception& e)
						{
							std::cout << e.what();
						}						
					}
					break;
				case 6:
					if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
						vfo.set_active_vfo(1);
					else
						vfo.set_active_vfo(0);	
					break;
				case 8:
					// Noise
					if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
					{
					}
					else
					{
					}
					break;
				}
			}
		}
	}
}

static void vol_slider_event_cb(lv_event_t * e)
{
	char buf[20];
	
	lv_obj_t * slider = lv_event_get_target(e);
	sprintf(buf, "vol %d", lv_slider_get_value(slider));
	lv_label_set_text(gbar.get_vol_slider_label(), buf);
	audio_output->set_volume(lv_slider_get_value(slider));
	catinterface.SetAG(lv_slider_get_value(slider));
}

static void gain_slider_event_cb(lv_event_t * e)
{
	char buf[20];
	
	lv_obj_t * slider = lv_event_get_target(e);
	sprintf(buf, "gain %ddb", lv_slider_get_value(slider));
	lv_label_set_text(gbar.get_gain_slider_label(), buf);
	try 
	{
		SdrDevices.SdrDevices.at(default_radio)->setGain(SOAPY_SDR_RX, default_rx_channel, lv_slider_get_value(slider));
	}
	catch (const std::exception& e)
	{
		std::cout << e.what();
	}
	gagc.update_gain_slider(lv_slider_get_value(slider));
}

void gui_bar::update_gain_slider(int gain)
{	
	char buf[30];
	
	sprintf(buf, "gain %d db", gain);
	lv_label_set_text(gain_slider_label, buf);		
	lv_slider_set_value(gain_slider, gain, LV_ANIM_ON); 
}

void gui_bar::step_gain_slider(int step)
{
	set_gain_slider(lv_slider_get_value(gain_slider) + step);
}

void gui_bar::set_gain_slider(int gain)
{
	char	buf[20];
	double	max_gain {0.0};
	double min_gain{0.0};
	
	try
	{
		max_gain = (double)SdrDevices.SdrDevices.at(default_radio)->rx_channels[default_rx_channel]->get_full_gain_range().maximum();
		min_gain = (double)SdrDevices.SdrDevices.at(default_radio)->rx_channels[default_rx_channel]->get_full_gain_range().minimum();
	}
	catch (const std::exception& e)
	{
		std::cout << e.what();
		return;
	}
	
	if (gain > max_gain)
		gain = max_gain;
	if (gain < min_gain)
		gain = min_gain;
	sprintf(buf, "gain %ddb", gain);
	lv_label_set_text(gain_slider_label, buf);		
	lv_slider_set_value(gain_slider, gain, LV_ANIM_ON); 
	try
	{
		SdrDevices.SdrDevices.at(default_radio)->setGain(SOAPY_SDR_RX, default_rx_channel, (double)gain);
	}
	catch (const std::exception& e)
	{
		std::cout << e.what();
	}
	gagc.update_gain_slider(gain);
}

static void filter_slider_event_cb(lv_event_t * e)
{
	lv_obj_t * obj = lv_event_get_target(e);
	lv_event_code_t code = lv_event_get_code(e);
	
	if (code == LV_EVENT_VALUE_CHANGED) 
	{
		int sel = lv_dropdown_get_selected(obj);
		int bandwidth = gbar.get_ifilters(sel);
		catinterface.SetSH(bandwidth);
		select_filter(bandwidth);
	}	 
}

void gui_bar::set_gain_range()
{
	int max_gain = 100, min_gain = 0;
	
	try
	{
		max_gain = (int)SdrDevices.SdrDevices.at(default_radio)->rx_channels[default_rx_channel]->get_full_gain_range().maximum();
		min_gain = (int)SdrDevices.SdrDevices.at(default_radio)->rx_channels[default_rx_channel]->get_full_gain_range().minimum();
	}
	catch (const std::exception& e)
	{
		std::cout << e.what();
	}
	lv_slider_set_range(gain_slider, min_gain, max_gain);
}


void gui_bar::init(lv_obj_t* o_parent, int mode, lv_coord_t w, lv_coord_t h)
{
	const lv_coord_t x_margin_dropdown  = 20;
	const lv_coord_t x_margin  = 2;
	const lv_coord_t y_margin  = 5;
	const int x_number_buttons = 5;
	const int y_number_buttons = 4;
	const int max_rows = 2;
	const lv_coord_t tab_margin  = w / 3;
	const int cw_margin = 20;

	int button_width_margin = ((w - tab_margin) / (x_number_buttons +1));
	int button_width = ((w - tab_margin) / (x_number_buttons+1)) - x_margin;
	int button_height = h / max_rows - y_margin - y_margin;
	int button_height_margin = button_height + y_margin;
	int	ibutton_x = 0, ibutton_y = 0;
	int i = 0;
	
	ifilters.push_back(500);
	ifilters.push_back(1000);
	ifilters.push_back(1500);
	ifilters.push_back(2000);
	ifilters.push_back(2500);
	ifilters.push_back(3000);
	ifilters.push_back(3500);
	ifilters.push_back(4000);
	ifilters.push_back(4500);
	ifilters.push_back(5000);

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
	
	lv_obj_set_style_pad_hor(o_parent, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(o_parent, 0, LV_PART_MAIN);
	lv_obj_clear_flag(o_parent, LV_OBJ_FLAG_SCROLLABLE);
	
	ibuttons = number_of_buttons;
	for (i = 0; i < ibuttons; i++)
	{
		char	str[80];
		
		if (i < number_of_buttons - 1)
		{
			button[i] = lv_btn_create(o_parent);
			lv_obj_add_style(button[i], &style_btn, 0); 
			lv_obj_add_event_cb(button[i], bar_button_handler, LV_EVENT_CLICKED, NULL);
			lv_obj_align(button[i], LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin, y_margin + ibutton_y * button_height_margin);
			lv_obj_set_size(button[i], button_width, button_height);
						
			lv_obj_t* lv_label = lv_label_create(button[i]);
			switch (i)
			{
			case 0:
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				lv_obj_set_user_data(button[i], NULL);
				strcpy(str, "TX");
				if (SdrDevices.get_tx_channels(default_radio) == 0)
					lv_obj_add_flag(button[i], LV_OBJ_FLAG_HIDDEN);
				break;
			case 1:
				strcpy(str, "USB");
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				lv_obj_set_user_data(button[i], (void *)mode_usb);
				if (mode == mode_usb)
					lv_obj_add_state(button[i], LV_STATE_CHECKED);
				break;
			case 2:
				strcpy(str, "LSB");
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				lv_obj_set_user_data(button[i], (void *)mode_lsb);
				if (mode == mode_lsb)
					lv_obj_add_state(button[i], LV_STATE_CHECKED);
				break;
			case 3:
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				lv_obj_set_user_data(button[i], (void *)mode_am);
				strcpy(str, "AM");
				if (mode == mode_am)
					lv_obj_add_state(button[i], LV_STATE_CHECKED);
				break;
			case 4:
				strcpy(str, "FM");
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				lv_obj_set_user_data(button[i], (void *)mode_narrowband_fm);
				if (mode == mode_narrowband_fm)
					lv_obj_add_state(button[i], LV_STATE_CHECKED);
				break;
			case 5:
				strcpy(str, "bFM");
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				lv_obj_set_user_data(button[i], (void *)mode_broadband_fm);
				if (mode == mode_broadband_fm)
					lv_obj_add_state(button[i], LV_STATE_CHECKED);
				break;
			case 6:
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				strcpy(str, "VFO2");
				break;
			case 7:
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				strcpy(str, "AGC");
				break;
			case 8:
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				strcpy(str, "Noise");
				break;

			}
			lv_label_set_text(lv_label, str);
			lv_obj_center(lv_label);
		}
		else
		{
			button[i] = lv_dropdown_create(o_parent);
			lv_dropdown_set_options_static(button[i], opts);
			lv_obj_align(button[i], LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin, y_margin + ibutton_y * button_height_margin);
			lv_obj_set_size(button[i], button_width + x_margin_dropdown, button_height);
			lv_obj_add_style(button[i], &style_btn, 0);
			lv_obj_add_event_cb(button[i], filter_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
			lv_dropdown_set_selected(button[i], 4);
			int bandwidth = ifilters[4];
			catinterface.SetSH(bandwidth);
		}
		
		ibutton_x++;
		if (ibutton_x >= x_number_buttons)
		{
			ibutton_x = 0;
			ibutton_y++;
		}
	}
	
	int vol_x = x_number_buttons * button_width_margin + 10 + x_margin_dropdown;
	int vol_width = (w / 3) - 20;
	vol_slider_label = lv_label_create(o_parent);
	lv_label_set_text(vol_slider_label, "vol");
	lv_obj_align(vol_slider_label, LV_ALIGN_TOP_LEFT, vol_x + vol_width + 5, 15);
	vol_slider = lv_slider_create(o_parent);
	lv_slider_set_range(vol_slider, 0, 100);
	lv_obj_set_width(vol_slider, vol_width); 
	lv_obj_align(vol_slider, LV_ALIGN_TOP_LEFT, vol_x , 15);
	lv_obj_add_event_cb(vol_slider, vol_slider_event_cb, LV_EVENT_PRESSING, NULL);
	
	int gain_y = 15 + y_margin + button_height_margin;
	gain_slider_label = lv_label_create(o_parent);
	lv_label_set_text(gain_slider_label, "gain");
	lv_obj_align(gain_slider_label, LV_ALIGN_TOP_LEFT, vol_x + vol_width + 5, gain_y);
	gain_slider = lv_slider_create(o_parent);
	lv_slider_set_range(gain_slider, 0, 100);
	lv_obj_set_width(gain_slider, vol_width); 
	lv_obj_align(gain_slider, LV_ALIGN_TOP_LEFT, vol_x, gain_y);
	lv_obj_add_event_cb(gain_slider, gain_slider_event_cb, LV_EVENT_PRESSING, NULL);
	
	try
	{
		if (SdrDevices.SdrDevices.at(default_radio)->rx_channels.at(default_rx_channel)->get_agc())
		{
			bool bAgc = SdrDevices.SdrDevices.at(default_radio)->getGainMode(SOAPY_SDR_RX, default_rx_channel);
			if (bAgc)
				lv_obj_add_state(button[7], LV_STATE_CHECKED);
		}
	}
	catch (const std::exception& e)
	{
		std::cout << e.what();
	}

	int cw_y = y_margin + 2 * button_height_margin;

	lv_style_init(&cw_style);
	lv_style_set_radius(&cw_style, 0);
	lv_style_set_bg_color(&cw_style, lv_color_black());

	cw_box = lv_obj_create(o_parent);
	lv_obj_add_style(cw_box, &cw_style, 0);
	lv_obj_set_pos(cw_box, 2, cw_y);
	lv_obj_set_size(cw_box, 5 * button_width_margin, 30);
	lv_obj_clear_flag(cw_box, LV_OBJ_FLAG_SCROLLABLE);

	cw_wpm = lv_label_create(cw_box);
	lv_obj_align(cw_wpm, LV_ALIGN_BOTTOM_LEFT, 30, 15);
	lv_label_set_text(cw_wpm, "wpm: ");

	cw_message = lv_label_create(cw_box);
	lv_obj_align(cw_message, LV_ALIGN_BOTTOM_LEFT, 100, 15);
	lv_label_set_text(cw_message, "....");
	
	cw_led = lv_led_create(cw_box);
	lv_obj_align(cw_led, LV_ALIGN_BOTTOM_LEFT, 0, 15);
	lv_led_set_color(cw_led, lv_palette_main(LV_PALETTE_RED));
	lv_obj_set_size(cw_led, 15, 15);
	lv_led_off(cw_led);
	hide_cw(true);
	}

	void gui_bar::set_led(bool status)
	{
		unique_lock<mutex> gui_lock(gui_mutex);
		if (status)
			lv_led_on(cw_led);
		else
			lv_led_off(cw_led);
	}
	
	void gui_bar::hide_cw(bool hide)
	{
		if (hide)
		{
			lv_obj_add_flag(cw_led, LV_OBJ_FLAG_HIDDEN);
			lv_obj_add_flag(cw_box, LV_OBJ_FLAG_HIDDEN);
			lv_obj_add_flag(cw_wpm, LV_OBJ_FLAG_HIDDEN);
			lv_obj_add_flag(cw_message, LV_OBJ_FLAG_HIDDEN);
		}
		else
		{
			lv_obj_clear_flag(cw_led, LV_OBJ_FLAG_HIDDEN);
			lv_obj_clear_flag(cw_box, LV_OBJ_FLAG_HIDDEN);
			lv_obj_clear_flag(cw_wpm, LV_OBJ_FLAG_HIDDEN);
			lv_obj_clear_flag(cw_message, LV_OBJ_FLAG_HIDDEN);
		}
	}
	
void gui_bar::check_agc()
{
	try
	{
		if (SdrDevices.SdrDevices.at(default_radio)->rx_channels.at(default_rx_channel)->get_agc())
		{
			bool bAgc = SdrDevices.SdrDevices.at(default_radio)->getGainMode(SOAPY_SDR_RX, default_rx_channel);
			if (bAgc)
				lv_obj_add_state(button[7], LV_STATE_CHECKED);
			else
				lv_obj_clear_state(button[7], LV_STATE_CHECKED);
		}
		else
			lv_obj_clear_state(button[7], LV_STATE_CHECKED);			
	}
	catch (const std::exception& e)
	{
		std::cout << e.what();
	}
}

void gui_bar::set_tx(bool tx)
{
	if (tx)
	{
		lv_obj_add_state(button[0], LV_STATE_CHECKED);
	}
	else
	{
		lv_obj_clear_state(button[0], LV_STATE_CHECKED);		
	}

}

void gui_bar::set_cw_message(std::string message)
{
	unique_lock<mutex> gui_lock(gui_mutex);
	lv_label_set_text(cw_message,message.c_str());
}

void gui_bar::set_cw_wpm(int wpm)
{
	char str[30];

	unique_lock<mutex> gui_lock(gui_mutex);
	sprintf(str, "wpm: %d", wpm);
	lv_label_set_text(cw_wpm, str);
}

	
void gui_bar::set_mode(int mode)
{
	if (mode == mode_usb)
	{
		lv_obj_add_state(button[1], LV_STATE_CHECKED);
		lv_obj_clear_state(button[2], LV_STATE_CHECKED);
	}
	if (mode == mode_lsb)
	{
		lv_obj_clear_state(button[1], LV_STATE_CHECKED);
		lv_obj_add_state(button[2], LV_STATE_CHECKED);
	}
	if (mode != mode_lsb && mode != mode_usb)
	{
		lv_obj_clear_state(button[1], LV_STATE_CHECKED);
		lv_obj_clear_state(button[2], LV_STATE_CHECKED);		
	}
}

void gui_bar::step_vol_slider(int step)
{
	set_vol_slider(lv_slider_get_value(vol_slider) + step);
}

void gui_bar::set_vol_slider(int volume)
{
	if (volume < 0)
		volume = 0;
	if (volume > max_volume)
		volume = max_volume;
	lv_slider_set_value(vol_slider, volume, LV_ANIM_ON);
	
	char buf[20];
	
	sprintf(buf, "vol %d", volume);
	lv_label_set_text(vol_slider_label, buf);
	audio_output->set_volume(volume);
}

int gui_bar::get_vol_range()
{
	return max_volume;
}

void gui_bar::get_filter_range(vector<string> &filters)
{
	filters.push_back("0.5 Khz");
	filters.push_back("1 Khz");
	filters.push_back("1.5 Khz");
	filters.push_back("2 Khz");
	filters.push_back("2.5 Khz");
	filters.push_back("3 Khz");
	filters.push_back("3.5 Khz");
	filters.push_back("4 Khz");
}



void gui_bar::set_filter_slider(int ifilter)
{
	if (!button[number_of_buttons - 1])
		return;
	int filter = 6;
	
	if (ifilter >= 500 && ifilter  < 1000)
		filter = 0;
	if (ifilter >= 1000 && ifilter  < 1500)
		filter = 1;
	if (ifilter >= 1500 && ifilter  < 2000)
		filter = 2;
	if (ifilter >= 2000 && ifilter  < 2500)
		filter = 3;
	if (ifilter >= 2500 && ifilter  < 3000)
		filter = 4;
	if (ifilter >= 3000 && ifilter  < 3500)
		filter = 5;
	if (ifilter >= 3500 && ifilter  < 4000)
		filter = 6;
	if (ifilter >= 4000)
		filter = 7;
	
	if (filter < 0 || filter > 7) 
		filter = 6;
	
	lv_dropdown_set_selected(button[number_of_buttons-1], filter);
	select_filter(ifilters[filter]);
}