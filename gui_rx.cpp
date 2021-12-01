#include "gui_rx.h"

gui_rx	Gui_rx;
const int number_of_buttons {8};

static void rx_button_handler(lv_event_t * e);
static void samplerate_button_handler(lv_event_t * e);
	
void gui_rx::gui_rx_init(lv_obj_t* o_tab, lv_coord_t w)
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
	
	lv_style_init(&style_btn);
	lv_style_set_radius(&style_btn, 10);
	lv_style_set_bg_color(&style_btn, lv_color_make(0x60, 0x60, 0x60));
	lv_style_set_bg_grad_color(&style_btn, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_bg_grad_dir(&style_btn, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_btn, 255);
	lv_style_set_border_color(&style_btn, lv_color_make(0x9b, 0x36, 0x36));         // lv_color_make(0x2e, 0x44, 0xb2)
	lv_style_set_border_width(&style_btn, 2);
	lv_style_set_border_opa(&style_btn, 255);
	lv_style_set_outline_color(&style_btn, lv_color_black());
	lv_style_set_outline_opa(&style_btn, 255);
	
	lv_obj_t* label = lv_label_create(o_tab);
	lv_label_set_text(label, "Mode");
	lv_obj_align(label, LV_ALIGN_TOP_LEFT, 0, 0);
	lv_obj_clear_flag(label, LV_OBJ_FLAG_SCROLLABLE);
	
	ibuttons = number_of_buttons;
	for (int i = 0; i < ibuttons; i++)
	{
		char	str[80];
		
		rx_button[i] = lv_btn_create(o_tab);
		lv_obj_add_style(rx_button[i], &style_btn, 0); 
		lv_obj_add_event_cb(rx_button[i], rx_button_handler, LV_EVENT_CLICKED, NULL);
		lv_obj_align(rx_button[i], LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin, y_margin + ibutton_y * button_height_margin);
		lv_obj_set_size(rx_button[i], button_width, button_height);
		
		lv_obj_t* lv_label = lv_label_create(rx_button[i]);
		switch (i)
		{
		case 0:
			strcpy(str, "USB");
			lv_obj_add_flag(rx_button[i], LV_OBJ_FLAG_CHECKABLE);
			lv_obj_set_user_data(rx_button[i], (void *)mode_usb);
			break;
		case 1:
			strcpy(str, "LSB");
			lv_obj_add_flag(rx_button[i], LV_OBJ_FLAG_CHECKABLE);
			lv_obj_set_user_data(rx_button[i], (void *)mode_lsb);
			break;
		case 2:
			strcpy(str, "CW");
			lv_obj_add_flag(rx_button[i], LV_OBJ_FLAG_CHECKABLE);
			lv_obj_set_user_data(rx_button[i], (void *)mode_cw);
			break;
		case 3:
			strcpy(str, "FM Nb");
			lv_obj_add_flag(rx_button[i], LV_OBJ_FLAG_CHECKABLE);
			lv_obj_set_user_data(rx_button[i], (void *)mode_narrowband_fm);
			break;
		case 4:
			strcpy(str, "FM Bb");
			lv_obj_add_flag(rx_button[i], LV_OBJ_FLAG_CHECKABLE);
			lv_obj_set_user_data(rx_button[i], (void *)mode_broadband_fm);
			break;
		case 5:
			strcpy(str, "FT8");
			lv_obj_add_flag(rx_button[i], LV_OBJ_FLAG_CHECKABLE);
			lv_obj_set_user_data(rx_button[i], (void *)mode_ft8);
			break;
		case 6:
			strcpy(str, "FT4");
			lv_obj_add_flag(rx_button[i], LV_OBJ_FLAG_CHECKABLE);
			lv_obj_set_user_data(rx_button[i], (void *)mode_ft4);
			break;
		case 7:
			strcpy(str, "RTTY");
			lv_obj_add_flag(rx_button[i], LV_OBJ_FLAG_CHECKABLE);
			lv_obj_set_user_data(rx_button[i], (void *)mode_rtty);
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
	
	ibutton_y++;
	d_samplerate = lv_dropdown_create(o_tab);
	lv_obj_align(d_samplerate, LV_ALIGN_TOP_LEFT, 0, y_margin + ibutton_y * button_height_margin);
	lv_dropdown_clear_options(d_samplerate);
	lv_obj_add_event_cb(d_samplerate, samplerate_button_handler, LV_EVENT_VALUE_CHANGED, NULL);
}

int gui_rx::get_sample_rate(int rate)
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
		ifrate = Gui_rx.get_sample_rate(rate);
		vfo.vfo_re_init((long)ifrate, pcmrate);
		stop_rxtx();
		soapy_devices[0].sdr->setSampleRate(SOAPY_SDR_RX, 0, ifrate);
		select_mode(mode);
	}
}

static void rx_button_handler(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e); 
	int bmode = (int)lv_obj_get_user_data(obj);
	
	if (code == LV_EVENT_CLICKED) {
	
		for (int i=0 ; i < Gui_rx.getbuttons(); i++)
			{
				if ((obj != Gui_rx.get_button_obj(i)) && (lv_obj_has_flag(Gui_rx.get_button_obj(i), LV_OBJ_FLAG_CHECKABLE)))
					{
						lv_obj_clear_state(Gui_rx.get_button_obj(i), LV_STATE_CHECKED);
					}
			}
		select_mode(bmode);
		gbar.set_mode(bmode);
	}
}

void gui_rx::add_sample_rate(int samplerate)
{
	char	str[30];
	
	sample_rates.push_back(samplerate);
	sprintf(str, "%d Khz", samplerate / 1000);
	lv_dropdown_add_option(d_samplerate, str, LV_DROPDOWN_POS_LAST);
}

void	gui_rx::set_gui_mode(int mode)
{
	int i = 0;
	switch (mode)
	{
	case mode_broadband_fm:
		i = 4;
		break;	
	case mode_lsb:
		i = 1;
		break;
	case mode_usb:
		i = 0;
		break;
	case mode_dsb:
		break;
	case mode_am:
		break;
	case mode_cw:
		i = 2;
		break;
	}
	
	lv_obj_t *obj = Gui_rx.get_button_obj(i);
	lv_obj_add_state(obj, LV_STATE_CHECKED);
	for (int i = 0; i < Gui_rx.getbuttons(); i++)
	{
		if ((obj != Gui_rx.get_button_obj(i)) && (lv_obj_has_flag(Gui_rx.get_button_obj(i), LV_OBJ_FLAG_CHECKABLE)))
		{
			lv_obj_clear_state(Gui_rx.get_button_obj(i), LV_STATE_CHECKED);
		}
	}
}

void gui_rx::set_sample_rate(int rate)
{
	int i;
	for (i = 0; i < sample_rates.size(); i++)
	{
		if (sample_rates[i] == rate)
			break;
	}
	lv_dropdown_set_selected(d_samplerate, i);
}