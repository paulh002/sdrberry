#include "gui_squelch.h"
#include "Settings.h"
#include "vfo.h"
#include "gui_vfo.h"
#include "screen.h"

gui_squelch guisquelch;

void gui_squelch::init(lv_obj_t *o_tab, lv_obj_t *tabbuttons, lv_coord_t w)
{

	const lv_coord_t x_margin = 10;
	const lv_coord_t y_margin = 20;
	const int x_number_buttons = 6;
	const int y_number_buttons = 4;
	const lv_coord_t tab_margin = 20;

	int button_width_margin = ((w - tab_margin) / x_number_buttons);
	int button_width = ((w - tab_margin) / x_number_buttons) - x_margin;
	int button_height = 40;
	int button_height_margin = button_height + y_margin;
	int ibutton_x = 0, ibutton_y = 0;
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
	m_button_group = lv_group_create();
	ibuttons = number_of_buttons;
	for (i = 0; i < ibuttons; i++)
	{
		char str[80];

		button[i] = lv_btn_create(o_tab);
		lv_group_add_obj(m_button_group, button[i]);
		lv_obj_add_style(button[i], &style_btn, 0);
		lv_obj_add_event_cb(button[i], button_handler, LV_EVENT_CLICKED, (void *)this);
		lv_obj_align(button[i], LV_ALIGN_TOP_LEFT, ibutton_x * button_width_margin, ibutton_y * button_height_margin);
		lv_obj_set_size(button[i], button_width, button_height);
		lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);

		lv_obj_t *lv_label = lv_label_create(button[i]);
		switch (i)
		{
		case 0:
			strcpy(str, "Off");
			lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
			lv_obj_set_user_data(button[i], NULL);
			break;
		case 1:
			strcpy(str, "Agc");
			lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
			lv_obj_set_user_data(button[i], NULL);
			break;
		case 2:
			strcpy(str, "Squelch");
			lv_obj_add_flag(button[i], LV_OBJ_FLAG_CHECKABLE);
			lv_obj_set_user_data(button[i], NULL);
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
	squelch_mode = Settings_file.get_int("Squelch", "enabled", 0);
	if (squelch_mode > 3 || squelch_mode < 0)
		squelch_mode = 0;
	lv_obj_add_state(button[squelch_mode], LV_STATE_CHECKED);
	ibutton_y++;
	bandwidth_slider = lv_slider_create(o_tab);
	lv_obj_set_width(bandwidth_slider, w / 2 - 50);
	lv_slider_set_range(bandwidth_slider, 1, 100);
	lv_obj_align_to(bandwidth_slider, o_tab, LV_ALIGN_TOP_LEFT, 0, ibutton_y * button_height_margin + 10);
	lv_obj_add_event_cb(bandwidth_slider, bandwidth_slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
	lv_group_add_obj(m_button_group, bandwidth_slider);

	bandwidth_slider_label = lv_label_create(o_tab);
	lv_label_set_text(bandwidth_slider_label, "bandwidth");
	lv_obj_align_to(bandwidth_slider_label, bandwidth_slider, LV_ALIGN_TOP_MID, 0, -20);
	set_bandwidth_slider(Settings_file.get_int("Squelch", "bandwidth", 0));

	// lv_obj_align_to(threshold_slider_label, o_tab, LV_ALIGN_CENTER, 0, -40);
	threshold_slider_label = lv_label_create(o_tab);
	lv_label_set_text(threshold_slider_label, "threshold -10 db");

	threshold_slider = lv_slider_create(o_tab);
	lv_obj_set_width(threshold_slider, w / 2 - 50);
	lv_slider_set_range(threshold_slider, -100, 0);
	lv_obj_align_to(threshold_slider, o_tab, LV_ALIGN_TOP_LEFT, w / 2, ibutton_y * button_height_margin + 10);
	lv_obj_add_event_cb(threshold_slider, threshold_slider_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);
	set_threshold_slider(Settings_file.get_int("Squelch", "threshold", -100));
	lv_group_add_obj(m_button_group, threshold_slider);
	lv_obj_align_to(threshold_slider_label, threshold_slider, LV_ALIGN_TOP_MID, 0, -20);

	lv_group_add_obj(m_button_group, tabbuttons);
}

void gui_squelch::set_bandwidth_slider(int _bandwidth)
{
	bandwidth = _bandwidth;
	std::string buf = strlib::sprintf("bandwidth %d hz", bandwidth.load());
	lv_label_set_text(bandwidth_slider_label, buf.c_str());
	lv_slider_set_value(bandwidth_slider, bandwidth, LV_ANIM_ON);
	Settings_file.save_int("Squelch", "bandwidth", bandwidth.load());
}

void gui_squelch::set_threshold_slider(int _threshold)
{
	threshold = _threshold;
	std::string buf = strlib::sprintf("threshold %d db", threshold.load());
	lv_label_set_text(threshold_slider_label, buf.c_str());
	lv_slider_set_value(threshold_slider, threshold, LV_ANIM_ON);
	Settings_file.save_int("Squelch", "threshold", threshold.load());
}

void gui_squelch::button_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);

	if (code == LV_EVENT_CLICKED)
	{
		for (int i = 0; i < ibuttons; i++)
		{
			if ((obj != button[i]) && (lv_obj_has_flag(button[i], LV_OBJ_FLAG_CHECKABLE)))
			{
				lv_obj_clear_state(button[i], LV_STATE_CHECKED);
			}
			else
			{
				squelch_mode = i;
				Settings_file.save_int("Squelch", "enabled", squelch_mode.load());
			}
		}
	}
}

void gui_squelch::threshold_slider_event_cb_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		threshold.store(lv_slider_get_value(obj));
		std::string buf = strlib::sprintf("threshold %d db", threshold.load());
		lv_label_set_text(threshold_slider_label, buf.c_str());
		Settings_file.save_int("Squelch", "threshold", threshold.load());
	}
}

void gui_squelch::bandwidth_slider_event_cb_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_VALUE_CHANGED)
	{
		bandwidth.store(lv_slider_get_value(obj));
		std::string buf = strlib::sprintf("bandwidth %d Hz", bandwidth.load());
		lv_label_set_text(bandwidth_slider_label, buf.c_str());
		Settings_file.save_int("Squelch", "bandwidth", bandwidth.load());
	}
}