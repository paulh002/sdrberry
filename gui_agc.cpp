#include "gui_agc.h"

Gui_agc gagc;

static void agc_button_handler(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e); 
	
	if (code == LV_EVENT_CLICKED) {
	
		for (int i = 0; i < gagc.getbuttons(); i++)
		{
			if ((obj != gagc.get_button_obj(i)) && (lv_obj_has_flag(gagc.get_button_obj(i), LV_OBJ_FLAG_CHECKABLE)))
			{
				lv_obj_clear_state(gagc.get_button_obj(i), LV_STATE_CHECKED);
			}
			else
				gagc.set_agc_mode(i);
		}
	}
}

static void threshold_slider_event_cb(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *slider = lv_event_get_target(e); 
	
	char	str[80];
	sprintf(str, "threshold %d db", lv_slider_get_value(slider) - gagc.max_threshold);
	lv_label_set_text(gagc.get_threshold_slider_label(), str);
}

static void slope_slider_event_cb(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *slider = lv_event_get_target(e); 
	
	char	str[80];
	sprintf(str, "slope %d db", lv_slider_get_value(slider));
	lv_label_set_text(gagc.get_slope_slider_label(), str);
}

static void delay_slider_event_cb(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *slider = lv_event_get_target(e); 
	
	char	str[80];
	sprintf(str, "delay %d ms", lv_slider_get_value(slider));
	lv_label_set_text(gagc.get_delay_slider_label(), str);
}
	
static void gain_slider_event_cb(lv_event_t * e)
{
	lv_obj_t * slider = lv_event_get_target(e);
	char buf[20];
	sprintf(buf, "gain %d db", lv_slider_get_value(slider));
	lv_label_set_text(gagc.get_gain_slider_label(), buf);
	soapy_devices[0].sdr->setGain(SOAPY_SDR_RX, soapy_devices[0].rx_channel, lv_slider_get_value(slider));
	gbar.update_gain_slider(lv_slider_get_value(slider));
}

void Gui_agc::init(lv_obj_t* o_tab, lv_coord_t w)
{

	const lv_coord_t x_margin  = 10;
	const lv_coord_t y_margin  = 20;
	const int x_number_buttons = 5;
	const int y_number_buttons = 4;
	const lv_coord_t tab_margin  = 20;
	
	int button_width_margin = ((w - tab_margin) / x_number_buttons);
	int button_width = ((w - tab_margin) / x_number_buttons) - x_margin;
	int button_height = 50;
	int button_height_margin = button_height + y_margin;
	int	ibutton_x = 0, ibutton_y = 0;
	int i = 0;
	
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
	
	ibuttons = number_of_buttons;
	for (i = 0; i < ibuttons; i++)
	{
		char	str[80];
		
		button[i] = lv_btn_create(o_tab);
		lv_obj_add_style(button[i], &style_btn, 0); 
		lv_obj_add_event_cb(button[i], agc_button_handler, LV_EVENT_CLICKED, NULL);
		lv_obj_align(button[i], LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin, ibutton_y * button_height_margin);
		lv_obj_set_size(button[i], button_width, button_height);
		lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);		
			
		lv_obj_t* lv_label = lv_label_create(button[i]);
		switch (i)
		{
		case 0:
			strcpy(str, "Off");
			lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
			lv_obj_set_user_data(button[i], (void *)mode_usb);
			break;
		case 1:
			strcpy(str, "Fast");
			lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
			lv_obj_set_user_data(button[i], (void *)mode_lsb);
			break;
		case 2:
			strcpy(str, "Med");
			lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
			lv_obj_set_user_data(button[i], (void *)mode_cw);
			break;
		case 3:
			strcpy(str, "Slow");
			lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
			lv_obj_set_user_data(button[i], (void *)mode_narrowband_fm);
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
	
	lv_obj_add_state(get_button_obj(agc_mode), LV_STATE_CHECKED);
	
	agc_mode = Settings_file.agc_mode();
	if (agc_mode > 3 || agc_mode < 0)
		agc_mode = 0;
	ibutton_y++;
	gain_slider = lv_slider_create(o_tab);
	lv_obj_set_width(gain_slider, w / 2 - 50); 
	lv_slider_set_range(gain_slider, 0, 100);
	lv_obj_align_to(gain_slider, o_tab, LV_ALIGN_TOP_LEFT, 0, ibutton_y * button_height_margin + 10);
	lv_obj_add_event_cb(gain_slider, gain_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	
	gain_slider_label = lv_label_create(o_tab);
	lv_label_set_text(gain_slider_label, "gain 0 db");
	lv_obj_align_to(gain_slider_label, gain_slider, LV_ALIGN_TOP_MID, 0,-20);
	
	
	//lv_obj_align_to(threshold_slider_label, o_tab, LV_ALIGN_CENTER, 0, -40);
	threshold_slider_label = lv_label_create(o_tab);
	lv_label_set_text(threshold_slider_label, "threshold -150 db");
	
	threshold_slider = lv_slider_create(o_tab);
	lv_obj_set_width(threshold_slider, w / 2 - 50); 
	lv_slider_set_range(threshold_slider, 0, 150);
	lv_obj_align_to(threshold_slider, o_tab, LV_ALIGN_TOP_LEFT, w / 2, ibutton_y * button_height_margin + 10);
	lv_obj_add_event_cb(threshold_slider, threshold_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	set_threshold_slider(max_threshold + Settings_file.agc_threshold());
	
	lv_obj_align_to(threshold_slider_label, threshold_slider, LV_ALIGN_TOP_MID, 0, -20);
	
	
	slope_slider_label = lv_label_create(o_tab);
	lv_label_set_text(slope_slider_label, "Slope ");
	lv_obj_align_to(slope_slider_label, gain_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	slope_slider = lv_slider_create(o_tab);
	lv_slider_set_range(slope_slider, 0, 20);
	lv_obj_set_width(slope_slider, w / 2 - 50); 
	lv_obj_align_to(slope_slider, slope_slider_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	lv_obj_add_event_cb(slope_slider, slope_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	set_slope_slider(Settings_file.agc_slope());
	
	delay_slider_label = lv_label_create(o_tab);
	lv_label_set_text(delay_slider_label, "Delay ");
	lv_obj_align_to(delay_slider_label, threshold_slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	delay_slider = lv_slider_create(o_tab);
	lv_slider_set_range(delay_slider, 0, 1000);
	lv_obj_set_width(delay_slider, w / 2 - 50); 
	lv_obj_align_to(delay_slider, delay_slider_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
	lv_obj_add_event_cb(delay_slider, delay_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
	set_delay_slider(Settings_file.agc_delay());
}

void Gui_agc::set_slope_slider(int t)
{
	char	str[80];
	
	lv_slider_set_value(slope_slider, t, LV_ANIM_ON);
	sprintf(str, "slope %d db", t);
	lv_label_set_text(slope_slider_label, str);		
}

void Gui_agc::set_threshold_slider(int t)
{
	char	str[80];
	
	lv_slider_set_value(threshold_slider, abs(t), LV_ANIM_ON);
	sprintf(str, "threshold -%d db", abs(t - gagc.max_threshold));
	lv_label_set_text(threshold_slider_label, str);	
}

void Gui_agc::set_delay_slider(int t)
{
	char	str[80];
	
	lv_slider_set_value(delay_slider, t, LV_ANIM_ON);
	sprintf(str, "delay %d ms", t);
	lv_label_set_text(delay_slider_label, str);	
}

void Gui_agc::set_gain_range(int min, int max)
{
	lv_slider_set_range(gain_slider, min, max);
}

void Gui_agc::step_gain_slider(int step)
{
	char buf[20];
	
	int gain = lv_slider_get_value(gain_slider) + step;
	int max_gain = (int)soapy_devices[0].channel_structure_rx[soapy_devices[0].rx_channel].full_gain_range.maximum();
	int min_gain = (int)soapy_devices[0].channel_structure_rx[soapy_devices[0].rx_channel].full_gain_range.minimum();
	if (gain > max_gain)
		gain = max_gain;
	if (gain < min_gain)
		gain = min_gain;
	sprintf(buf, "gain %d db", gain);
	lv_label_set_text(gain_slider_label, buf);		
	lv_slider_set_value(gain_slider, gain, LV_ANIM_ON); 
	soapy_devices[0].sdr->setGain(SOAPY_SDR_RX, 0, (double)gain);
}

void Gui_agc::set_gain_range()
{
	int max_gain = (int)soapy_devices[0].channel_structure_rx[soapy_devices[0].rx_channel].full_gain_range.maximum();
	int min_gain = (int)soapy_devices[0].channel_structure_rx[soapy_devices[0].rx_channel].full_gain_range.minimum();
	lv_slider_set_range(gain_slider, min_gain, max_gain);
}

void Gui_agc::update_gain_slider(int gain)
{	
	char buf[20];
	sprintf(buf, "gain %d db", gain);
	lv_label_set_text(gain_slider_label, buf);		
	lv_slider_set_value(gain_slider, gain, LV_ANIM_ON); 
}

void Gui_agc::set_gain_slider(int gain)
{
	char buf[20];
	
	double max_gain = soapy_devices[0].channel_structure_rx[soapy_devices[0].rx_channel].full_gain_range.maximum();
	if (gain > max_gain)
		gain = max_gain;
	
	sprintf(buf, "gain %d db", gain);
	lv_label_set_text(gain_slider_label, buf);		
	lv_slider_set_value(gain_slider, gain, LV_ANIM_ON); 
	soapy_devices[0].sdr->setGain(SOAPY_SDR_RX, 0, (double)gain);
	gbar.update_gain_slider(gain);
}

void Gui_agc::get_gain_range(int &max_gain, int &min_gain)
{
	max_gain = (int)soapy_devices[0].channel_structure_rx[soapy_devices[0].rx_channel].full_gain_range.maximum();
	min_gain = (int)soapy_devices[0].channel_structure_rx[soapy_devices[0].rx_channel].full_gain_range.minimum();
	return ;
}