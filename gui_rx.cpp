#include "gui_rx.h"

gui_rx guirx;

static void rx_button_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_CLICKED)
	{
		for (auto con : guirx.get_buttons())
		{
			if (con.first == obj)
			{
				if (con.second > 0L)
					vfo.set_vfo(con.second, false);
			}
			else
			{
				lv_obj_clear_state(con.first, LV_STATE_CHECKED);	
			}
		}
	}
	if (code == LV_EVENT_LONG_PRESSED)
	{
		for (auto con : guirx.get_buttons())
		{
			if (con.first == obj)
			{
				vector<long> array;
				guirx.set_freq(con.first, vfo.get_frequency());
				guirx.get_buttons(array);
				Settings_file.set_array_long("preselect", "buttons", array);
			}
			else
			{
			}
		}
	}
	
}

vector<pair<lv_obj_t *, long long>> gui_rx::get_buttons() 
{
	return buttons;
}

void gui_rx::get_buttons(vector<long> &array)
{
	for (auto con : buttons)
	{
		array.push_back(con.second);
	}
}

void noise_handler(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		int noise = guirx.get_noise();
		Settings_file.save_int("Radio", "noise", noise);
	}
}

void gui_rx::set_freq(lv_obj_t *obj, long long freq)
{
	for (auto &con : buttons)
	{
		if (con.first == obj)
			con.second = freq;
	}
}

int gui_rx::get_noise()
{
	return lv_dropdown_get_selected(drp_noise);
}

void gui_rx::init(lv_obj_t *o_tab, lv_coord_t w)
{

	const lv_coord_t x_margin = 10;
	const lv_coord_t y_margin = 5;
	const int x_number_buttons = 5;
	const int y_number_buttons = 4;
	const lv_coord_t tab_margin = 20;

	int button_width_margin = ((w - tab_margin) / x_number_buttons);
	int button_width = ((w - tab_margin) / x_number_buttons) - x_margin;
	int button_height = 50;
	int button_height_margin = button_height + y_margin;
	int ibutton_x = 0, ibutton_y = 0;
	
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

	m_button_group = lv_group_create();
	int ibuttons = number_of_buttons;
	vector<long> array_long;
	Settings_file.get_array_long("preselect", "buttons", array_long);
	for (int i = 0; i < ibuttons; i++)
	{
		char str[80];

		if (i < array_long.size())
			buttons.push_back(std::make_pair(lv_btn_create(o_tab), (long long)array_long.at(i)));
		else
			buttons.push_back(std::make_pair(lv_btn_create(o_tab), (long long)0));
		lv_group_add_obj(m_button_group, buttons.back().first);
		lv_obj_add_style(buttons.back().first, &style_btn, 0);
		lv_obj_add_event_cb(buttons.back().first, rx_button_handler, (lv_event_code_t)LV_EVENT_ALL /*(LV_EVENT_CLICKED | LV_EVENT_LONG_PRESSED)*/, NULL);
		lv_obj_align(buttons.back().first, LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin, ibutton_y * button_height_margin);
		//lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
		lv_obj_set_size(buttons.back().first, button_width, button_height);

		lv_obj_t *lv_label = lv_label_create(buttons.back().first);
		switch (i)
		{
		case 0:
			strcpy(str, "Pre 1");
			lv_obj_add_flag(buttons.back().first, LV_OBJ_FLAG_CHECKABLE);
			break;
		case 1:
			strcpy(str, "Pre 2");
			lv_obj_add_flag(buttons.back().first, LV_OBJ_FLAG_CHECKABLE);
			break;
		case 2:
			strcpy(str, "Pre 3");
			lv_obj_add_flag(buttons.back().first, LV_OBJ_FLAG_CHECKABLE);
			break;
		case 3:
			strcpy(str, "Pre 4");
			lv_obj_add_flag(buttons.back().first, LV_OBJ_FLAG_CHECKABLE);
			break;
		case 4:
			strcpy(str, "Pre 5");
			lv_obj_add_flag(buttons.back().first, LV_OBJ_FLAG_CHECKABLE);
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

	lv_obj_t *noise_label = lv_label_create(o_tab);
	lv_label_set_text(noise_label, "Noise suppression");
	lv_obj_align(noise_label, LV_ALIGN_TOP_LEFT, 0, y_margin + ibutton_y * button_height_margin);

	drp_noise = lv_dropdown_create(o_tab);
	lv_obj_align(drp_noise, LV_ALIGN_TOP_LEFT, 0, y_margin + ibutton_y * 1.5 *  button_height_margin);
	lv_dropdown_clear_options(drp_noise);
	lv_group_add_obj(m_button_group, drp_noise);
	lv_dropdown_add_option(drp_noise, "Leaky LMS", LV_DROPDOWN_POS_LAST);
	//lv_dropdown_add_option(drp_noise, "LMS", LV_DROPDOWN_POS_LAST);
	lv_dropdown_add_option(drp_noise, "Spectral", LV_DROPDOWN_POS_LAST);
	lv_dropdown_add_option(drp_noise, "Kim", LV_DROPDOWN_POS_LAST);

	int noise = Settings_file.get_int("Radio", "noise");
	lv_dropdown_set_selected(drp_noise, noise);
	lv_obj_add_event_cb(drp_noise, noise_handler, (lv_event_code_t)LV_EVENT_VALUE_CHANGED, NULL);

	lv_group_add_obj(m_button_group, lv_tabview_get_tab_btns(tabview_mid));
}
