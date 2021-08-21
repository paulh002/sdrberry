#include "gui_tx.h"
#include "Settings.h"
#include "sdrberry.h"
#include "vfo.h"
#include "gui_vfo.h"

static void mic_slider_event_cb(lv_event_t * e);
static void tx_button_handler(lv_event_t * e);
static void drv_slider_event_cb(lv_event_t * e);

const int micgain {100};


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

lv_obj_t*	mic_slider_label, *mic_slider, *drv_slider, *drv_slider_label;
static lv_style_t	style_btn;
static lv_obj_t		*tx_button[10];

void gui_tx_init(lv_obj_t* o_tab, lv_coord_t w)
{
	const lv_coord_t x_margin  = 10;
	const lv_coord_t y_margin  = 10;
	const int x_number_buttons = 5;
	const int y_number_buttons = 4;
	const lv_coord_t tab_margin  = 20;
	
	int button_width_margin = ((w - tab_margin) / x_number_buttons);
	int button_width = ((w - tab_margin) / x_number_buttons) - x_margin;
	int button_height = 50;
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
	
	lv_coord_t		pos_x = x_margin, pos_y = y_margin;
	int				ibutton_x = 0, ibutton_y = 0;
	
	for (int i = 0; i < 4; i++)
	{	char	str[80];
		
		tx_button[i] = lv_btn_create(o_tab);
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
	lv_obj_set_width(mic_slider, rightWidth - 40); 
	lv_slider_set_range(mic_slider, 0, micgain);
	lv_obj_center(mic_slider);
	lv_obj_add_event_cb(mic_slider, mic_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	mic_slider_label = lv_label_create(o_tab);
	lv_obj_align_to(mic_slider_label, mic_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	set_mic_slider(Settings_file.micgain());
	
	drv_slider = lv_slider_create(o_tab);
	lv_obj_set_width(drv_slider, rightWidth - 40); 
	lv_slider_set_range(drv_slider, 0, 15);
	lv_obj_align_to(drv_slider, mic_slider_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	lv_obj_add_event_cb(drv_slider, drv_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	drv_slider_label = lv_label_create(o_tab);
	lv_obj_align_to(drv_slider_label, drv_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	set_drv_slider(Settings_file.drive());
}

static void mic_slider_event_cb(lv_event_t * e)
{
	lv_obj_t * slider = lv_event_get_target(e);
	char buf[20];
	
	sprintf(buf, "gain %d db", lv_slider_get_value(slider));
	lv_label_set_text(mic_slider_label, buf);
	lv_obj_align_to(mic_slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	if (audio_input != nullptr)
		audio_input->set_volume(lv_slider_get_value(slider));
	Settings_file.set_micgain(lv_slider_get_value(slider));
}

void step_mic_slider(int step)
{
	set_mic_slider(lv_slider_get_value(mic_slider) + step);
	Settings_file.set_micgain(lv_slider_get_value(mic_slider));
}

void set_mic_slider(int volume)
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
		audio_input->set_volume(volume / 100.0);
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
			select_mode_tx(mode);
		else
			select_mode(mode);
	}
	if (s == "Sync RX vfo")
	{
		lv_obj_clear_state(tx_button[3], LV_STATE_CHECKED); 
		vfo.set_active_vfo(0);			
		vfo.sync_rx_vfo();
	}
	if (s == "Split TX vfo")
	{
		if ((lv_obj_get_state(tx_button[3]) & LV_STATE_CHECKED) &&
		 (lv_obj_get_state(tx_button[0]) & LV_STATE_CHECKED))
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
	lv_label_set_text(drv_slider_label, buf);
	lv_obj_align_to(drv_slider_label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	Settings_file.set_drive(lv_slider_get_value(slider));
}

void step_drv_slider(int step)
{
	set_drv_slider(lv_slider_get_value(drv_slider) + step);
	Settings_file.set_drive(lv_slider_get_value(drv_slider));
}

void set_drv_slider(int drive)
{
	if (drive < 0)
		drive = 0;
	if (drive > 15)
		drive = 15;
	lv_slider_set_value(drv_slider, drive, LV_ANIM_ON);
	char buf[20];
	
	sprintf(buf, "drive %d", drive);
	lv_label_set_text(drv_slider_label, buf);
	lv_obj_align_to(drv_slider_label, drv_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
}

void set_tx_state(bool state)
{
	if (state)
	{
		lv_obj_add_state(tx_button[0], LV_STATE_CHECKED); 
		if (lv_obj_get_state(tx_button[3]) & LV_STATE_CHECKED)
			{
				// If Vfo split mode set active vfo 1
				vfo.set_active_vfo(1);
			}
	}
	else
	{
		lv_obj_clear_state(tx_button[0], LV_STATE_CHECKED); 		
		vfo.set_active_vfo(0);
	}
}
