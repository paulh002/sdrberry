#include "gui_tx.h"
#include "Settings.h"
#include "sdrberry.h"
#include "vfo.h"
#include "gui_vfo.h"

static void mic_slider_event_cb(lv_event_t * e);
static void tx_button_handler(lv_event_t * e);
static void drv_slider_event_cb(lv_event_t * e);

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

void gui_tx::gui_tx_init(lv_obj_t* o_tab, lv_coord_t w)
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
	lv_obj_clear_flag(o_tab, LV_OBJ_FLAG_SCROLLABLE);
	lv_coord_t		pos_x = x_margin, pos_y = y_margin;
	int				ibutton_x = 0, ibutton_y = 0;
	
	m_button_group = lv_group_create();
	ibuttons = number_of_buttons;
	for (int i = 0; i < ibuttons; i++)
	{	char	str[80];
		
		tx_button[i] = lv_btn_create(o_tab);
		lv_group_add_obj(m_button_group, tx_button[i]);
		lv_obj_add_style(tx_button[i], &style_btn, 0); 
		lv_obj_add_event_cb(tx_button[i], tx_button_handler, LV_EVENT_CLICKED, NULL);
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
			strcpy(str, "Tune");
			lv_obj_add_flag(tx_button[i], LV_OBJ_FLAG_CHECKABLE);
			break;
		case 5:
			strcpy(str, "2 Tone");
			lv_obj_add_flag(tx_button[i], LV_OBJ_FLAG_CHECKABLE);
			break;
		case 6:
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
	
	mic_slider = lv_slider_create(o_tab);
	lv_obj_set_width(mic_slider, w / 2 - 50); 
	lv_slider_set_range(mic_slider, 0, micgain);
	lv_obj_center(mic_slider);
	lv_obj_add_event_cb(mic_slider, mic_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	mic_slider_label = lv_label_create(o_tab);
	lv_obj_align_to(mic_slider_label, mic_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	set_mic_slider(Settings_file.micgain());
	lv_group_add_obj(m_button_group, mic_slider);
	
	drv_slider = lv_slider_create(o_tab);
	lv_obj_set_width(drv_slider, w / 2 - 50); 
	lv_slider_set_range(drv_slider, 0, 15);
	lv_obj_align_to(drv_slider, mic_slider_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	lv_obj_add_event_cb(drv_slider, drv_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	drv_slider_label = lv_label_create(o_tab);
	lv_obj_align_to(drv_slider_label, drv_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	set_drv_slider(Settings_file.drive());
	lv_group_add_obj(m_button_group, drv_slider);

	ibutton_y++;
	drp_samplerate = lv_dropdown_create(o_tab);
	lv_obj_align(drp_samplerate, LV_ALIGN_TOP_LEFT, 0, y_margin + ibutton_y * button_height_margin);
	lv_dropdown_clear_options(drp_samplerate);
	lv_group_add_obj(m_button_group, drp_samplerate);

	lv_group_add_obj(m_button_group, lv_tabview_get_tab_btns(tabview_mid));
}

void gui_tx::set_group()
{
	lv_indev_set_group(encoder_indev_t, m_button_group);
	lv_group_focus_obj(tx_button[0]);
}

static void mic_slider_event_cb(lv_event_t * e)
{
	lv_obj_t * slider = lv_event_get_target(e);
	char buf[30];
	
	sprintf(buf, "mic gain %d db", lv_slider_get_value(slider));
	lv_label_set_text(Gui_tx.get_mic_label(), buf);
	lv_obj_align_to(Gui_tx.get_mic_label(), slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	if (audio_input != nullptr)
		audio_input->set_volume(lv_slider_get_value(slider));
	Settings_file.set_micgain(lv_slider_get_value(slider));
}

void gui_tx::step_mic_slider(int step)
{
	set_mic_slider(lv_slider_get_value(mic_slider) + step);
	Settings_file.set_micgain(lv_slider_get_value(mic_slider));
}

void gui_tx::set_mic_slider(int volume)
{
	if (volume < 0)
		volume = 0;
	if (volume > micgain)
		volume = micgain;
	lv_slider_set_value(mic_slider, volume, LV_ANIM_ON);
	char buf[20];
	
	sprintf(buf, "mic gain %d", volume);
	lv_label_set_text(mic_slider_label, buf);
	lv_obj_align_to(mic_slider_label, mic_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	if (audio_input != nullptr)
		audio_input->set_volume(volume);
}

static void tx_button_handler(lv_event_t * e)
{
	
	lv_obj_t *obj = lv_event_get_target(e); 
	lv_obj_t *label = lv_obj_get_child(obj, 0L);
	char *ptr = lv_label_get_text(label);
	string s(ptr);
	
	if (s == "TX")
	{
		if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
		{
			select_mode_tx(mode);
			gbar.set_tx(true);
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
			select_mode_tx(mode, SingleTone);
		else
			select_mode(mode);
	}
	if (s == "2 Tone")
	{
		if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
			select_mode_tx(mode, TwoTone);
		else
			select_mode(mode);
	}
	if (s == "4 Tone")
	{
		if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
			select_mode_tx(mode, FourTone);
		else
			select_mode(mode);
	}
	if (s == "Sync RX vfo")
	{
		lv_obj_clear_state(Gui_tx.get_button_obj(3), LV_STATE_CHECKED); 
		vfo.set_active_vfo(0);			
		vfo.sync_rx_vfo();
	}
	if (s == "Split TX vfo")
	{
		if ((lv_obj_get_state(Gui_tx.get_button_obj(3)) & LV_STATE_CHECKED) &&
		 (lv_obj_get_state(Gui_tx.get_button_obj(0)) & LV_STATE_CHECKED))
		{
			// If Vfo split mode set active vfo 1
			vfo.set_active_vfo(1);
		}
		else
		{
			vfo.set_active_vfo(0);			
		}
	}
}

static void drv_slider_event_cb(lv_event_t * e)
{
	lv_obj_t * slider = lv_event_get_target(e);
	char buf[20];
	
	sprintf(buf, "drive %d", lv_slider_get_value(slider));
	lv_label_set_text(Gui_tx.get_drv_label(), buf);
	lv_obj_align_to(Gui_tx.get_drv_label(), slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	Settings_file.set_drive(lv_slider_get_value(slider));
	try
	{
		SdrDevices.SdrDevices.at(default_radio)->setGain(SOAPY_SDR_TX, gsetup.get_current_tx_channel(), (double)lv_slider_get_value(slider));
	}
	catch (const std::exception& e)
	{
		std::cout << e.what() << endl;
	}
}

void gui_tx::step_drv_slider(int step)
{
	set_drv_slider(lv_slider_get_value(drv_slider) + step);
	Settings_file.set_drive(lv_slider_get_value(drv_slider));
}

void gui_tx::set_drv_range()
{	int max_gain, min_gain;
	try
	{
		max_gain = (int)SdrDevices.SdrDevices.at(default_radio)->tx_channels[gsetup.get_current_tx_channel()]->get_full_gain_range().maximum();
		min_gain = (int)SdrDevices.SdrDevices.at(default_radio)->tx_channels[gsetup.get_current_tx_channel()]->get_full_gain_range().minimum();	
	}
	catch (const std::exception& e)
	{
		max_gain = 100;
		min_gain = 0;
	}
	lv_slider_set_range(drv_slider, min_gain, max_gain);
	set_drv_slider(Settings_file.drive());
}

void gui_tx::set_drv_slider(int drive)
{
	int max_gain, min_gain;
	try
	{
		max_gain = (int)SdrDevices.SdrDevices.at(default_radio)->tx_channels[gsetup.get_current_tx_channel()]->get_full_gain_range().maximum();
		min_gain = (int)SdrDevices.SdrDevices.at(default_radio)->tx_channels[gsetup.get_current_tx_channel()]->get_full_gain_range().minimum();	
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
	lv_obj_align_to(drv_slider_label, drv_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	try
	{
		SdrDevices.SdrDevices.at(default_radio)->setGain(SOAPY_SDR_TX, gsetup.get_current_tx_channel(), (double)drive);
	}
	catch(const std::exception& e)
	{
		std::cout << e.what();
	}
}

int gui_tx::get_drv_pos()
{
	return Settings_file.drive();
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

void gui_tx::clear_sample_rate()
{
	lv_dropdown_clear_options(drp_samplerate);
}

void gui_tx::add_sample_rate(int samplerate)
{
	char	str[30];
	
	sample_rates.push_back(samplerate);
	sprintf(str, "%d Khz", samplerate / 1000);
	lv_dropdown_add_option(drp_samplerate, str, LV_DROPDOWN_POS_LAST);
}

void gui_tx::set_sample_rate(int rate)
{
	int i;
	for (i = 0; i < sample_rates.size(); i++)
	{
		if (sample_rates[i] == rate)
			break;
	}
	lv_dropdown_set_selected(drp_samplerate, i);
}