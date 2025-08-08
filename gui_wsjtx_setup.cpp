#include "gui_wsjtx_setup.h"
#include "gui_ft8.h"

gui_wsjtx_setup guiwsjtxsetup;

void gui_wsjtx_setup::init(lv_obj_t *o_tab, lv_group_t *keyboard_group, lv_coord_t w, lv_coord_t h, lv_group_t *bg)
{
	const lv_coord_t x_page_margin = 5;
	const lv_coord_t x_margin = 10;
	const lv_coord_t y_margin = 5;
	const int y_number_buttons = 4;
	const lv_coord_t tab_margin = 20;

	int x_number_buttons = 5;
	if (w > 800)
		x_number_buttons = 8;

	int button_width_margin = ((w - tab_margin) / x_number_buttons);
	int button_width = ((w - tab_margin) / x_number_buttons) - x_margin;
	int button_height = 50;
	int button_height_margin = button_height + y_margin;
	int ibutton_x = 0, ibutton_y = 0;

	filter_cq = lv_checkbox_create(o_tab);
	lv_obj_align(filter_cq, LV_ALIGN_TOP_LEFT, 2 * (x_margin + button_width), y_margin + (button_height * 3) / 4);
	lv_checkbox_set_text(filter_cq, "filter on CQ");
	lv_obj_add_event_cb(filter_cq, filter_cq_event_cb, LV_EVENT_VALUE_CHANGED, (void *)this);

	std::string s = Settings_file.get_string(std::string("wsjtx"), std::string("filter_on_cq"));
	if (s == "true")
	{
		gft8.filter_on_cq(true);
		lv_obj_add_state(filter_cq, LV_STATE_CHECKED);
	}

	text_label = lv_label_create(o_tab);
	lv_label_set_text(text_label, "QRA");
	lv_obj_align(text_label, LV_ALIGN_TOP_LEFT, x_margin, y_margin);

	lv_style_init(&text_style);
	lv_style_set_radius(&text_style, 0);
	lv_style_set_bg_color(&text_style, lv_color_black());

	qra_textarea = lv_textarea_create(o_tab);
	lv_obj_add_style(qra_textarea, &text_style, 0);
	lv_textarea_set_one_line(qra_textarea, true);
	lv_obj_align(qra_textarea, LV_ALIGN_TOP_LEFT, x_margin, y_margin + button_height_margin / 2);
	lv_obj_add_event_cb(qra_textarea, qra_textarea_event_handler, LV_EVENT_ALL, qra_textarea);
	lv_obj_add_state(qra_textarea, LV_STATE_FOCUSED); /*To be sure the cursor is visible*/
	lv_obj_set_size(qra_textarea, button_width - 20, button_height - 10);
	lv_obj_set_style_pad_top(qra_textarea, 4, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(qra_textarea, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_left(qra_textarea, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_right(qra_textarea, 2, LV_PART_MAIN);
	if (keyboard_group != nullptr)
		lv_group_add_obj(keyboard_group, qra_textarea);
	std::string call = Settings_file.get_string("wsjtx", "call");
	lv_textarea_add_text(qra_textarea, call.c_str());

	text_label = lv_label_create(o_tab);
	lv_label_set_text(text_label, "locator");
	lv_obj_align(text_label, LV_ALIGN_TOP_LEFT, x_margin + button_width, y_margin);
	
	locator_textarea = lv_textarea_create(o_tab);
	lv_obj_add_style(locator_textarea, &text_style, 0);
	lv_textarea_set_one_line(locator_textarea, true);
	lv_obj_align(locator_textarea, LV_ALIGN_TOP_LEFT, x_margin + button_width, y_margin + button_height_margin / 2);
	//lv_obj_add_event_cb(locator_textarea, qra_textarea_event_handler, LV_EVENT_ALL, qra_textarea);
	lv_obj_add_state(locator_textarea, LV_STATE_FOCUSED); /*To be sure the cursor is visible*/
	lv_obj_set_size(locator_textarea, button_width - 20, button_height - 10);
	lv_obj_set_style_pad_top(locator_textarea, 4, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(locator_textarea, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_left(locator_textarea, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_right(locator_textarea, 2, LV_PART_MAIN);
	if (keyboard_group != nullptr)
		lv_group_add_obj(keyboard_group, locator_textarea);
	std::string locator = Settings_file.get_string("wsjtx", "locator");
	lv_textarea_add_text(locator_textarea, locator.c_str());
}

void gui_wsjtx_setup::qra_textarea_event_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_CLICKED && kb == nullptr)
	{
		// kb = lv_keyboard_create(lv_scr_act());
		// lv_keyboard_set_textarea(kb, qra_textarea);
		// Settings_file.save();
	}
}

void gui_wsjtx_setup::filter_cq_event_cb_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_current_target(e);

	if (code == LV_EVENT_VALUE_CHANGED)
	{
		if (lv_obj_get_state(obj) & LV_STATE_CHECKED)
		{
			gft8.filter_on_cq(true);
			Settings_file.save_string(std::string("wsjtx"), std::string("filter_on_cq"), "true");
			Settings_file.write_settings();
		}
		else
		{
			gft8.filter_on_cq(false);
			Settings_file.save_string(std::string("wsjtx"), std::string("filter_on_cq"), "false");
			Settings_file.write_settings();
		}
	}
}