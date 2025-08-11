#include "Gui_band.h"
#include "BandFilter.h"
#include "Catinterface.h"
#include "gui_bar.h"
#include "vfo.h"
#include "WebServer.h"
#include "screen.h"
#include "gui_edit_band.h"

Gui_band gui_band_instance;

string RemoveChar(string str, char c)
{
	std::string result;
	for (size_t i = 0; i < str.size(); i++)
	{
		char currentChar = str[i];
		if (currentChar != c)
			result += currentChar;
	}
	return result;
}

void Gui_band::ham_event_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
			vfo.set_vfo_limit_ham_band(true);
		else
			vfo.set_vfo_limit_ham_band(false);
	}
}

void Gui_band::band_event_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
			bpf.Setpasstrough(true);
		else
			bpf.Setpasstrough(false);
	}
}

void Gui_band::init_button_gui(lv_obj_t *o_tab, lv_group_t *keyboard_group,lv_coord_t w, lv_coord_t h, SoapySDR::RangeList r)
{
	int band, i = 0;
	std::string label;
	
	if (!o_tab)
	{
		// nullptr then reinitialize
		o_tab = tab;
		for (auto col : button)
			lv_obj_del(col);
		button.clear();
		lv_obj_del(limitvfocheckbox);
		lv_obj_del(bandfiltercheckbox);
	}
	else
	{
		tab = o_tab;
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
		lv_obj_set_style_pad_top(o_tab, 5, LV_PART_MAIN);
		lv_obj_set_style_pad_bottom(o_tab, 5, LV_PART_MAIN);
		lv_obj_set_style_pad_right(o_tab, 5, LV_PART_MAIN);
		
		tileview = lv_tileview_create(o_tab);
		lv_obj_clear_flag(tileview, LV_OBJ_FLAG_SCROLL_ELASTIC);

		main_tile = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_BOTTOM | LV_DIR_TOP);
		edit_tile = lv_tileview_add_tile(tileview, 0, 1, LV_DIR_BOTTOM | LV_DIR_TOP);
		guieditband.init(edit_tile, w, h, NULL, keyboard_group);
	}

	// lv_coord_t w = lv_obj_get_width(o_tab);

	f_min = r.front().minimum();
	f_max = r.front().maximum();

	// constants
	max_rows = 5;
	x_margin = 10;
	y_margin = 10;
	x_number_buttons = 5;
	tab_margin = 20;

	if (screenHeight < 500)
		max_rows = 4;

	button_width_margin = (((w -15) - tab_margin) / x_number_buttons);
	button_width = (((w - 15) - tab_margin) / x_number_buttons) - x_margin;
	button_height = h / max_rows - y_margin - y_margin; // 40;
	button_height_margin = button_height + y_margin;

	lv_coord_t pos_x = x_margin, pos_y = y_margin;
	int ibutton_x = 0, ibutton_y = 0;

	m_button_group = lv_group_create();
	auto it_m = begin(Settings_file.labels);
	auto it_f_low = begin(Settings_file.f_low);
	for (auto col : Settings_file.meters)
	{
		band = col;
		label = (string)*it_m;
		long f_low = (long)*it_f_low;
		it_m++;
		it_f_low++;

		// place button
		if (label.length() && f_low >= f_min && f_low <= f_max)
		{
			button.push_back(lv_btn_create(main_tile));
			lv_group_add_obj(m_button_group, button[i]);
			lv_obj_add_style(button.back(), &style_btn, 0);
			lv_obj_add_event_cb(button.back(), band_button, LV_EVENT_CLICKED, (void *)this);
			lv_obj_align(button.back(), LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin, ibutton_y * button_height_margin);
			lv_obj_add_flag(button.back(), LV_OBJ_FLAG_CHECKABLE);
			lv_obj_set_size(button.back(), button_width, button_height);
			lv_obj_t *lv_label = lv_label_create(button[i]);

			char str[20];

			string s = RemoveChar(label, 0x22);
			sprintf(str, "%d %s", band, (char *)s.c_str());
			lv_label_set_text(lv_label, str);
			lv_obj_center(lv_label);
			buttons.push_back(str);

			ibutton_x++;
			if (ibutton_x >= x_number_buttons)
			{
				ibutton_x = 0;
				ibutton_y++;
			}
			i++;
		}
	}
	lv_obj_clear_flag(main_tile, LV_OBJ_FLAG_SCROLLABLE);
	limitvfocheckbox = lv_checkbox_create(main_tile);
	lv_group_add_obj(m_button_group, limitvfocheckbox);
	lv_checkbox_set_text(limitvfocheckbox, "limit vfo to band");
	lv_obj_add_event_cb(limitvfocheckbox, ham_event_handler, LV_EVENT_ALL, (void *)this);
	if (ibutton_x)
		ibutton_y++;
	lv_obj_align(limitvfocheckbox, LV_ALIGN_TOP_LEFT, tab_margin, ibutton_y * button_height_margin);

	if (vfo.is_vfo_limit_ham_band())
		lv_obj_add_state(limitvfocheckbox, LV_STATE_CHECKED);

	bandfiltercheckbox = lv_checkbox_create(main_tile);
	lv_group_add_obj(m_button_group, bandfiltercheckbox);
	lv_checkbox_set_text(bandfiltercheckbox, "band filter off");
	lv_obj_add_event_cb(bandfiltercheckbox, band_event_handler, LV_EVENT_ALL, (void *)this);
	lv_obj_align(bandfiltercheckbox, LV_ALIGN_TOP_LEFT, tab_margin + lv_obj_get_width(limitvfocheckbox) + button_width_margin, ibutton_y * button_height_margin);

	lv_group_add_obj(m_button_group, lv_tabview_get_tab_btns(tabview_mid));
	update_web();
}

void Gui_band::reload_buttons()
{
	int ibutton_x = 0, ibutton_y = 0;
	int band, i = 0;
	std::string label;

	for (auto col : button)
		lv_obj_del(col);
	button.clear();
	
	auto it_m = begin(Settings_file.labels);
	auto it_f_low = begin(Settings_file.f_low);
	for (auto col : Settings_file.meters)
	{
		band = col;
		label = (string)*it_m;
		long f_low = (long)*it_f_low;
		it_m++;
		it_f_low++;

		// place button
		if (label.length() && f_low >= f_min && f_low <= f_max)
		{
			button.push_back(lv_btn_create(main_tile));
			lv_group_add_obj(m_button_group, button[i]);
			lv_obj_add_style(button.back(), &style_btn, 0);
			lv_obj_add_event_cb(button.back(), band_button, LV_EVENT_CLICKED, (void *)this);
			lv_obj_align(button.back(), LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin, ibutton_y * button_height_margin);
			lv_obj_add_flag(button.back(), LV_OBJ_FLAG_CHECKABLE);
			lv_obj_set_size(button.back(), button_width, button_height);
			lv_obj_t *lv_label = lv_label_create(button[i]);

			char str[20];

			string s = RemoveChar(label, 0x22);
			sprintf(str, "%d %s", band, (char *)s.c_str());
			lv_label_set_text(lv_label, str);
			lv_obj_center(lv_label);
			buttons.push_back(str);

			ibutton_x++;
			if (ibutton_x >= x_number_buttons)
			{
				ibutton_x = 0;
				ibutton_y++;
			}
			i++;
		}
	}
}

void Gui_band::set_group()
{
	lv_indev_set_group(encoder_indev_t, m_button_group);
	lv_group_focus_obj(button[0]);
}

int getIndex(vector<int> v, int s)
{
	int i = 0;
	for (auto it = begin(v); it != end(v); ++it)
	{
		if (*it == s)
		{
			return i;
		}
		i++;
	}
	return -1;
}

void Gui_band::band_button_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	lv_obj_t *label = lv_obj_get_child(obj, 0L);
	char *ptr = lv_label_get_text(label);
	std::string s(ptr);

	if (code == LV_EVENT_CLICKED)
	{
		int i, ii = 1;
		size_t n;

		n = s.find("c");
		if (n != std::string::npos)
		{
			s.erase(n);
			i = stoi(s);
		}
		n = s.find("m");
		if (n != std::string::npos)
		{
			s.erase(n);
			i = stoi(s);
		}
		if (ptr != NULL)
		{
			int index = getIndex(Settings_file.meters, i);
			long f_low = Settings_file.f_low.at(index);
			int f_band = Settings_file.meters.at(index);
			vfo.set_band(f_band, f_low);
			gbar.set_mode(mode);
			catinterface->SetBand(i);
		}

		for (auto col : button)
		{
			if ((obj != col) && (lv_obj_has_flag(col, LV_OBJ_FLAG_CHECKABLE)))
			{
				lv_obj_clear_state(col, LV_STATE_CHECKED);
			}
		}
		lv_obj_add_state(obj, LV_STATE_CHECKED);
		create_spectrum_page_time();
	}
}

void Gui_band::set_gui(int band)
{
	for (auto col : button)
	{
		lv_obj_t *obj = col;
		lv_obj_t *label = lv_obj_get_child(obj, 0L);
		char *ptr = lv_label_get_text(label);
		string s(ptr);

		int n = s.find("m");
		s.erase(n);
		int ii = stoi(s);

		if ((ii == band) && (lv_obj_has_flag(col, LV_OBJ_FLAG_CHECKABLE)))
		{
			lv_obj_add_state(col, LV_STATE_CHECKED);
		}

		if ((ii != band) && (lv_obj_has_flag(col, LV_OBJ_FLAG_CHECKABLE)))
		{
			lv_obj_clear_state(col, LV_STATE_CHECKED);
		}
	}
}

void Gui_band::update_web()
{
	if (webserver.isEnabled())
	{
		json message;

		message.emplace("type", "bands");
		message.emplace("data", buttons);
		webserver.SendMessage(message);
	}	
}