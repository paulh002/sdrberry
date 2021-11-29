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
					select_mode(bmode);
					break;
				case 3:
					if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
						gagc.set_agc_mode(1);
					else
						gagc.set_agc_mode(0);						
					break;
				case 4:
					if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
						vfo.set_active_vfo(1);
					else
						vfo.set_active_vfo(0);	
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
	sprintf(buf, "volume %d", lv_slider_get_value(slider));
	lv_label_set_text(gbar.get_vol_slider_label(), buf);
	audio_output->set_volume(lv_slider_get_value(slider));
}

static void filter_slider_event_cb(lv_event_t * e)
{
	lv_obj_t * obj = lv_event_get_target(e);
	lv_event_code_t code = lv_event_get_code(e);
	
	if (code == LV_EVENT_VALUE_CHANGED) 
	{
		int sel = lv_dropdown_get_selected(obj);
		select_filter(sel);
	}
	 
}

void gui_bar::init(lv_obj_t* o_parent, int mode, lv_coord_t w, lv_coord_t h)
{
	const lv_coord_t x_margin_dropdown  = 20;
	const lv_coord_t x_margin  = 2;
	const lv_coord_t y_margin  = 5;
	const int x_number_buttons = 7;
	const int y_number_buttons = 4;
	const lv_coord_t tab_margin  = w / 3;
	
	int button_width_margin = ((w - tab_margin) / x_number_buttons);
	int button_width = ((w - tab_margin) / x_number_buttons) - x_margin;
	int button_height = h - y_margin - y_margin;
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
				break;
			case 1:
				strcpy(str, "Usb");
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
				strcpy(str, "AGC");
				break;
			case 4:
				lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
				strcpy(str, "VFO2");
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
			lv_dropdown_set_selected(button[i], 6);
		}
		
		ibutton_x++;
		if (ibutton_x >= x_number_buttons)
		{
			ibutton_x = 0;
			ibutton_y++;
		}
	}
	
	int vol_x = ibutton_x * button_width_margin + 20 + x_margin_dropdown;
	int vol_width = (w / 3) - 20;
	vol_slider_label = lv_label_create(o_parent);
	lv_label_set_text(vol_slider_label, "vol");
	lv_obj_align(vol_slider_label, LV_ALIGN_TOP_LEFT, vol_x + vol_width + 5, 15);
	vol_slider = lv_slider_create(o_parent);
	lv_slider_set_range(vol_slider, 0, 100);
	lv_obj_set_width(vol_slider, vol_width); 
	lv_obj_align(vol_slider, LV_ALIGN_TOP_LEFT, vol_x , 15);
	lv_obj_add_event_cb(vol_slider, vol_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
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

void gui_bar::set_filter_slider(int filter)
{
	if (filter < 0 || filter > 7) 
		filter = 6;
	lv_dropdown_set_selected(button[number_of_buttons-1], filter);
	select_filter(filter);
}