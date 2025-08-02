#include "gui_tx.h"
#include "Settings.h"
#include "sdrberry.h"
#include "vfo.h"
#include "gui_vfo.h"
#include "gui_bar.h"
#include "gui_setup.h"
#include "gui_sdr.h"
#include "gui_speech.h"
#include "screen.h"
#include <memory>

const int micgain {100};

int drv_min = 0;
int drv_max = 15;


/*
 * To do:
 * 1x VFO for TX
 * 2x Mic volume slider
 * 3 Vox 
 * 4 Show audio spectrum
 * 5x Button for VFO sync TX
 * 6 Button for tune
 * 7 CW interface ?
 * 8x TX button
 */

gui_tx	Gui_tx;

void gui_tx::gui_tx_init(lv_obj_t* tx_tile, lv_coord_t w, bool disable)
{
	const lv_coord_t x_margin  = 10;
	const lv_coord_t y_margin  = 10;
	const int x_number_buttons = 6;
	const int y_number_buttons = 4;
	const lv_coord_t tab_margin  = 20;
	
	int button_width_margin = ((w - tab_margin) / x_number_buttons);
	int button_width = ((w - tab_margin) / x_number_buttons) - x_margin;
	int button_height = 40;
	int button_height_margin = button_height + y_margin;

	lv_obj_set_style_pad_top(tx_tile, 10, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(tx_tile, 5, LV_PART_MAIN);
	lv_obj_set_style_pad_right(tx_tile, 5, LV_PART_MAIN);
	lv_obj_set_style_pad_left(tx_tile, 5, LV_PART_MAIN);

	tileview = lv_tileview_create(tx_tile);
	lv_obj_clear_flag(tileview, LV_OBJ_FLAG_SCROLL_ELASTIC);
	tx_tile = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_BOTTOM | LV_DIR_TOP);
	speech_tile = lv_tileview_add_tile(tileview, 0, 1, LV_DIR_BOTTOM | LV_DIR_TOP);
	gspeech.init(speech_tile, w);
	
	
	disabled = disable;
	lv_style_init(&style_btn);
	lv_style_set_radius(&style_btn, 10);
	lv_style_set_bg_color(&style_btn, lv_color_make(0x60, 0x60, 0x60));
	lv_style_set_bg_grad_color(&style_btn, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_bg_grad_dir(&style_btn, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_btn, 255);
	lv_style_set_border_color(&style_btn, lv_color_make(0x9b, 0x36, 0x36));       // lv_color_make(0x2e, 0x44, 0xb2)
	lv_style_set_border_width(&style_btn, 2);
	lv_style_set_border_opa(&style_btn, 255);
	lv_style_set_outline_color(&style_btn, lv_color_black());
	lv_style_set_outline_opa(&style_btn, 255);
	lv_obj_clear_flag(tx_tile, LV_OBJ_FLAG_SCROLLABLE);
	lv_coord_t		pos_x = x_margin, pos_y = y_margin;
	int				ibutton_x = 0, ibutton_y = 0;
	
	m_button_group = lv_group_create();
	ibuttons = number_of_buttons;
	for (int i = 0; i < ibuttons; i++)
	{	char	str[80];
		
		tx_button[i] = lv_btn_create(tx_tile);
		lv_group_add_obj(m_button_group, tx_button[i]);
		lv_obj_add_style(tx_button[i], &style_btn, 0); 
		lv_obj_add_event_cb(tx_button[i], tx_button_handler, LV_EVENT_CLICKED, (void *)this);
		lv_obj_align(tx_button[i], LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin, ibutton_y * button_height_margin);
		//lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);		
		lv_obj_set_size(tx_button[i], button_width, button_height);
		
		lv_obj_t* lv_label = lv_label_create(tx_button[i]);
		switch (i)
		{
		case 0:
			strcpy(str, "TX");
			lv_obj_add_flag(tx_button[i], LV_OBJ_FLAG_CHECKABLE);
			break;
		case 1:
			strcpy(str, "Tune");
			lv_obj_add_flag(tx_button[i], LV_OBJ_FLAG_CHECKABLE);
			break;
		case 2:
			strcpy(str, "Sync RX vfo");
			break;
		case 3:
			strcpy(str, "Split TX vfo");
			lv_obj_add_flag(tx_button[i], LV_OBJ_FLAG_CHECKABLE);
			break;
		case 4:
			strcpy(str, "2 Tone");
			lv_obj_add_flag(tx_button[i], LV_OBJ_FLAG_CHECKABLE);
			break;
		case 5:
			strcpy(str, "4 Tone");
			lv_obj_add_flag(tx_button[i], LV_OBJ_FLAG_CHECKABLE);
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
	
	mic_slider = lv_slider_create(tx_tile);
	lv_obj_set_width(mic_slider, w / 2 - 50); 
	lv_slider_set_range(mic_slider, 0, micgain);
	lv_obj_center(mic_slider);
	lv_obj_add_event_cb(mic_slider, mic_slider_event_cb, LV_EVENT_VALUE_CHANGED, (void*)this);
	mic_slider_label = lv_label_create(tx_tile);
	lv_obj_align_to(mic_slider_label, mic_slider, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
	set_mic_slider(Settings_file.get_int("Radio", "micgain", 85));
	lv_group_add_obj(m_button_group, mic_slider);

	digital_slider = lv_slider_create(tx_tile);
	lv_obj_set_width(digital_slider, w / 2 - 50);
	lv_slider_set_range(digital_slider, 0, micgain);
	lv_obj_align_to(digital_slider, mic_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
	lv_obj_add_event_cb(digital_slider, digital_slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
	digital_slider_label = lv_label_create(tx_tile);
	lv_obj_align_to(digital_slider_label, digital_slider, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
	set_digital_slider(Settings_file.get_int("Radio", "digitalgain", 80));
	lv_group_add_obj(m_button_group, digital_slider);
	
	drv_slider = lv_slider_create(tx_tile);
	lv_obj_set_width(drv_slider, w / 2 - 50); 
	lv_slider_set_range(drv_slider, 0, 15);
	lv_obj_align_to(drv_slider, digital_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
	lv_obj_add_event_cb(drv_slider, drv_slider_event_cb, LV_EVENT_VALUE_CHANGED, (void*)this);
	drv_slider_label = lv_label_create(tx_tile);
	lv_obj_align_to(drv_slider_label, drv_slider, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
	set_drv_slider(Settings_file.get_int(default_radio, "drive", 50));
	lv_group_add_obj(m_button_group, drv_slider);
	lv_obj_set_tile_id(tileview, 0, 0, LV_ANIM_OFF);
	ibutton_y++;
	tempSensor::start_read_out();
	temp_meter = tempmeter.init(tx_tile, 100, 100);
	tempmeter.set_pos(x_margin, button_height_margin + y_margin);
	if (tempSensor::count_tempsensors() == 0)
	{
		tempmeter.hide(true);
	}
}

void gui_tx::set_group()
{
	lv_indev_set_group(encoder_indev_t, m_button_group);
	lv_group_focus_obj(tx_button[0]);
}

void gui_tx::get_measurements()
{
	tempmeter.set_indicator(tempSensor::get_temperature(0));
}

void gui_tx::mic_slider_event_cb_class(lv_event_t * e)
{
	lv_obj_t * slider = lv_event_get_target(e);
	char buf[30];
	
	sprintf(buf, "mic gain %d db", lv_slider_get_value(slider));
	lv_label_set_text(mic_slider_label, buf);
	if (audio_input != nullptr)
		audio_input->set_volume(lv_slider_get_value(slider));
	Settings_file.save_int("Radio", "micgain", lv_slider_get_value(slider));
	Settings_file.write_settings();
}

void gui_tx::digital_slider_event_cb_class(lv_event_t *e)
{
	lv_obj_t *slider = lv_event_get_target(e);
	char buf[30];

	sprintf(buf, "digital gain %d db", lv_slider_get_value(slider));
	lv_label_set_text(digital_slider_label, buf);
	if (audio_input != nullptr)
		audio_input->set_digital_volume(lv_slider_get_value(slider));
	Settings_file.save_int("Radio", "digitalgain", lv_slider_get_value(slider));
	Settings_file.write_settings();
}

void gui_tx::set_digital_slider(int volume)
{
	if (volume < 0)
		volume = 0;
	if (volume > micgain)
		volume = micgain;
	lv_slider_set_value(digital_slider, volume, LV_ANIM_ON);
	char buf[20];

	sprintf(buf, "digital gain %d db", volume);
	lv_label_set_text(digital_slider_label, buf);
	if (audio_input != nullptr)
		audio_input->set_digital_volume(volume);
}

void gui_tx::set_mic_slider(int volume)
{
	if (volume < 0)
		volume = 0;
	if (volume > micgain)
		volume = micgain;
	lv_slider_set_value(mic_slider, volume, LV_ANIM_ON);
	char buf[20];
	
	sprintf(buf, "mic gain %d db", volume);
	lv_label_set_text(mic_slider_label, buf);
	if (audio_input != nullptr)
		audio_input->set_volume(volume);
}

void gui_tx::tx_button_handler_class(lv_event_t * e)
{
	
	lv_obj_t *obj = lv_event_get_target(e); 
	lv_obj_t *label = lv_obj_get_child(obj, 0L);
	char *ptr = lv_label_get_text(label);
	string s(ptr);
	
	if (s == "TX")
	{
		if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
		{
			if (select_mode_tx(mode))
				gbar.set_tx(true);
			else
				lv_obj_clear_state(obj, LV_STATE_CHECKED);	
		}
		else
		{
			select_mode(mode);
			gbar.set_tx(false);
		}
	}
	if (s == "Tune") 
	{
		if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
		{
			if (!select_mode_tx(mode, SingleTone))
				lv_obj_clear_state(obj, LV_STATE_CHECKED);
		}
		else
			select_mode(mode);
	}
	if (s == "2 Tone")
	{
		if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
		{
			if (!select_mode_tx(mode, TwoTone))
				lv_obj_clear_state(obj, LV_STATE_CHECKED);
		}
		else
			select_mode(mode);
	}
	if (s == "4 Tone")
	{
		if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
		{
			if (!select_mode_tx(mode, FourTone))
				lv_obj_clear_state(obj, LV_STATE_CHECKED);
		}
		else
			select_mode(mode);
	}
	if (s == "Sync RX vfo")
	{
		vfo.set_active_vfo(0);			
		vfo.sync_rx_vfo();
	}
	if (s == "Split TX vfo")
	{
		if (lv_obj_get_state(get_button_obj(3)) & LV_STATE_CHECKED)
		{
			gui_vfo_inst.set_split(true);
		}
		else
		{
			gui_vfo_inst.set_split(false);
		}
	}
}

void gui_tx::drv_slider_event_cb_class(lv_event_t * e)
{
	lv_obj_t * slider = lv_event_get_target(e);
	char buf[20];
	
	sprintf(buf, "drive %d", lv_slider_get_value(slider));
	lv_label_set_text(drv_slider_label, buf);
	Settings_file.save_int(default_radio, "drive", lv_slider_get_value(slider));
	Settings_file.write_settings();
	try
	{
		SdrDevices.SdrDevices.at(default_radio)->setGain(SOAPY_SDR_TX, guisdr.get_current_tx_channel(), (double)lv_slider_get_value(slider));
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << endl;
	}
}

void gui_tx::step_drv_slider(int step)
{
	set_drv_slider(lv_slider_get_value(drv_slider) + step);
	Settings_file.save_int(default_radio,"drive",lv_slider_get_value(drv_slider));
	Settings_file.write_settings();
}

void gui_tx::set_drv_range()
{	int max_gain, min_gain;
	try
	{
		max_gain = (int)SdrDevices.SdrDevices.at(default_radio)->tx_channels[guisdr.get_current_tx_channel()]->get_full_gain_range().maximum();
		min_gain = (int)SdrDevices.SdrDevices.at(default_radio)->tx_channels[guisdr.get_current_tx_channel()]->get_full_gain_range().minimum();	
	}
	catch (const std::exception& e)
	{
		max_gain = 100;
		min_gain = 0;
	}
	lv_slider_set_range(drv_slider, min_gain, max_gain);
	set_drv_slider(Settings_file.get_int(default_radio, "drive", 50));
}

void gui_tx::set_drv_slider(int drive)
{
	if (disabled)
		return;
	int max_gain, min_gain;
	try
	{
		max_gain = (int)SdrDevices.SdrDevices.at(default_radio)->tx_channels[guisdr.get_current_tx_channel()]->get_full_gain_range().maximum();
		min_gain = (int)SdrDevices.SdrDevices.at(default_radio)->tx_channels[guisdr.get_current_tx_channel()]->get_full_gain_range().minimum();	
	}
	catch (const std::exception& e)
	{
		max_gain = 100;
		min_gain = 0;
	}
	if (drive < min_gain)
		drive = min_gain;
	if (drive > max_gain)
		drive = max_gain;
	lv_slider_set_value(drv_slider, drive, LV_ANIM_ON);
	char buf[20];
	
	sprintf(buf, "drive %d", drive);
	lv_label_set_text(drv_slider_label, buf);
	try
	{
		SdrDevices.SdrDevices.at(default_radio)->setGain(SOAPY_SDR_TX, guisdr.get_current_tx_channel(), (double)drive);
	}
	catch(const std::exception& e)
	{
		std::cout << e.what();
	}
}

int gui_tx::get_drv_pos()
{
	return Settings_file.get_int(default_radio, "drive", 50);
}

void gui_tx::set_tx_state(bool state)
{
	if (state)
	{
		lv_obj_add_state(tx_button[0], LV_STATE_CHECKED); 
		if (lv_obj_get_state(tx_button[3]) & LV_STATE_CHECKED)
			{
				// If Vfo split mode set active vfo 1
				//vfo.set_active_vfo(1);
			}
	}
	else
	{
		lv_obj_clear_state(tx_button[0], LV_STATE_CHECKED); 		
		//vfo.set_active_vfo(0);
	}
}

lv_obj_t* gui_tx::get_button_obj(int i)
{
	if (i >= ibuttons)
		return nullptr;
	return tx_button[i];
}

void gui_tx::set_split(bool _split)
{
	if (disabled)
		return;
	if (_split)
	{
		gui_vfo_inst.set_split(true);
		lv_obj_add_state(get_button_obj(3), LV_STATE_CHECKED);
	}
	else
	{
		gui_vfo_inst.set_split(false);
		lv_obj_clear_state(get_button_obj(3), LV_STATE_CHECKED);
	}
}

void gui_tx::enable_tx(bool enable)
{
	if (enable)
		disabled = false;
	else
		disabled = true;
}