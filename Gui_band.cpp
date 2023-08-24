#include "sdrberry.h"
#include "BandFilter.h"

Gui_band	gui_band_instance;


void band_button(lv_event_t * e);

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

static void ham_event_handler(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t * obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED) 
	{
		if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
			vfo.limit_ham_band = true;
		else
			vfo.limit_ham_band = false;
	}
}

static void band_event_handler(lv_event_t *e)
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

void Gui_band::init_button_gui(lv_obj_t *o_tab, lv_coord_t w, lv_coord_t h, SoapySDR::RangeList r)
{
	int		band;
	std::string	label;
	int		i = 0;
	
	if (!o_tab)
	{
		//nullptr then reinitialize
		o_tab = tab;
		for (int ii = 0; ii < ibuttons; ii++)
		{
			lv_obj_del(button[ii]);
			button[i] = nullptr;
		}
		ibuttons = 0;
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
	}
	
	//lv_coord_t w = lv_obj_get_width(o_tab);	
	long f_min = r.front().minimum();
	long f_max = r.front().maximum();

	const int max_rows = 4;
	const lv_coord_t x_margin = 10;
	const lv_coord_t y_margin = 10;
	const int x_number_buttons = 5;
	const int y_number_buttons = 4;
	const lv_coord_t tab_margin = 20;
	
	button_width_margin = ((w - tab_margin) / x_number_buttons);
	button_width = ((w - tab_margin) / x_number_buttons) - x_margin;
	button_height = h / max_rows - y_margin - y_margin; //40;
	button_height_margin = button_height + y_margin;
	
	lv_coord_t		pos_x = x_margin, pos_y = y_margin;
	int				ibutton_x = 0, ibutton_y = 0;

	m_button_group = lv_group_create();
	auto it_m = begin(Settings_file.labels);
	auto it_f_low = begin(Settings_file.f_low);
	ibuttons = 0; // count number of buttons
	for (auto it = begin(Settings_file.meters); it != end(Settings_file.meters); ++it) 
	{		
		band = *it;
		label = (string)*it_m;
		long	f_low = (long)*it_f_low;
		it_m++;
		it_f_low++;
		
	// place button
		if(label.length() && f_low >= f_min && f_low <= f_max)
		{	
			button[i] = lv_btn_create(o_tab);
			lv_group_add_obj(m_button_group, button[i]);
			lv_obj_add_style(button[i], &style_btn, 0); 
			lv_obj_add_event_cb(button[i], band_button, LV_EVENT_CLICKED, NULL);
			lv_obj_align(button[i], LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin, ibutton_y * button_height_margin);
			lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);		
			lv_obj_set_size(button[i], button_width, button_height);
			lv_obj_t* lv_label = lv_label_create(button[i]);
		
			char str[20];
			
			string s = RemoveChar(label, 0x22);
			sprintf(str, "%d %s", band, (char *)s.c_str());
			lv_label_set_text(lv_label, str);
			lv_obj_center(lv_label);
			
			ibutton_x++;
			if (ibutton_x >= x_number_buttons)
			{
				ibutton_x = 0;
				ibutton_y++;
			}
			i++;
			ibuttons++;
		}
	}
	lv_obj_clear_flag(o_tab, LV_OBJ_FLAG_SCROLLABLE);
	limitvfocheckbox = lv_checkbox_create(o_tab);
	lv_group_add_obj(m_button_group, limitvfocheckbox);
	lv_checkbox_set_text(limitvfocheckbox, "limit vfo to band");
	lv_obj_add_event_cb(limitvfocheckbox, ham_event_handler, LV_EVENT_ALL, NULL);
	if (ibutton_x)
		ibutton_y++;
	lv_obj_align(limitvfocheckbox, LV_ALIGN_TOP_LEFT, tab_margin, ibutton_y * button_height_margin);
	
	if (vfo.limit_ham_band)
		lv_obj_add_state(limitvfocheckbox, LV_STATE_CHECKED);

	bandfiltercheckbox = lv_checkbox_create(o_tab);
	lv_group_add_obj(m_button_group, bandfiltercheckbox);
	lv_checkbox_set_text(bandfiltercheckbox, "band filter off");
	lv_obj_add_event_cb(bandfiltercheckbox, band_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_align(bandfiltercheckbox, LV_ALIGN_TOP_LEFT, tab_margin + lv_obj_get_width(limitvfocheckbox) + button_width_margin, ibutton_y * button_height_margin);

	lv_group_add_obj(m_button_group, lv_tabview_get_tab_btns(tabview_mid));
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

void band_button(lv_event_t * e)
{
	lv_event_code_t code = lv_event_get_code(e); 
	lv_obj_t *obj = lv_event_get_target(e); 
	lv_obj_t *label = lv_obj_get_child(obj, 0L);
	char *ptr = lv_label_get_text(label);
	std::string s(ptr);
	
	if (code == LV_EVENT_CLICKED) 
	{	int			i, ii = 1;
		size_t		n;
		
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
			int index  = getIndex(Settings_file.meters, i);		
			long f_low = Settings_file.f_low.at(index);
			int f_band = Settings_file.meters.at(index);
			vfo.set_band(f_band, f_low);
			gbar.set_mode(mode);
			catinterface.SetBand(i);
		}
	
		for (int i = 0; i < gui_band_instance.getbuttons(); i++)
		{
			if ((obj != gui_band_instance.get_button_obj(i)) && (lv_obj_has_flag(gui_band_instance.get_button_obj(i), LV_OBJ_FLAG_CHECKABLE)))
			{
				lv_obj_clear_state(gui_band_instance.get_button_obj(i), LV_STATE_CHECKED);
			}
		}
		lv_obj_add_state(obj, LV_STATE_CHECKED);
	}
}

void Gui_band::set_gui(int band)
{
	for (int i = 0; i < gui_band_instance.getbuttons(); i++)
	{
		lv_obj_t *obj = gui_band_instance.get_button_obj(i);
		lv_obj_t *label = lv_obj_get_child(obj, 0L);
		char *ptr = lv_label_get_text(label);
		string s(ptr);
	
		int n = s.find("m");
		s.erase(n);
		int ii = stoi(s);	
		
		if ((ii == band) && (lv_obj_has_flag(gui_band_instance.get_button_obj(i), LV_OBJ_FLAG_CHECKABLE)))
		{
			lv_obj_add_state(gui_band_instance.get_button_obj(i), LV_STATE_CHECKED);		
		}
		
		if ((ii != band) && (lv_obj_has_flag(gui_band_instance.get_button_obj(i), LV_OBJ_FLAG_CHECKABLE)))
		{
			lv_obj_clear_state(gui_band_instance.get_button_obj(i), LV_STATE_CHECKED);
		}
	}
}