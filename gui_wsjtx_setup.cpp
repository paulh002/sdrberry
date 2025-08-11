#include "gui_wsjtx_setup.h"
#include "gui_ft8.h"
#include "pskreporter.h"

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
	int cHeight = lv_obj_get_content_height(o_tab);
	int cWidth = lv_obj_get_content_width(o_tab);
	float fraction = 14.0f / 80.0f;
	
	filter_cq = lv_checkbox_create(o_tab);
	lv_obj_align(filter_cq, LV_ALIGN_TOP_LEFT, x_margin, button_height_margin + y_margin + (button_height * 3) / 4);
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
	call = Settings_file.get_string("wsjtx", "call");
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
	
	pskbutton = lv_btn_create(o_tab);
	//lv_group_add_obj(m_button_group, pskbutton);
	lv_obj_add_style(pskbutton, &style_btn, 0);
	lv_obj_add_event_cb(pskbutton, psk_button_handler, LV_EVENT_CLICKED, (void *)this);
	lv_obj_align(pskbutton, LV_ALIGN_TOP_LEFT, x_margin + button_width, button_height_margin + y_margin + button_height_margin / 2);
	lv_obj_set_size(pskbutton, button_width - 20, button_height - 10);
	lv_obj_t *lv_label = lv_label_create(pskbutton);
	lv_label_set_text(lv_label, "Psk reporter");
	lv_obj_center(lv_label);

	lv_style_init(&tablestyle);
	lv_style_set_radius(&tablestyle, 0);
	lv_style_set_bg_color(&tablestyle, lv_color_black());

	float fraction1 = 45.0f / 80.0f;
	psktable = lv_table_create(o_tab);
	lv_obj_add_style(psktable, &tablestyle, 0);
	//lv_obj_clear_flag(psktable, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_pos(psktable, 2 * (x_margin + button_width) , y_margin + ibutton_y * button_height_margin);
	lv_obj_set_size(psktable, cWidth * fraction1, h - 20 );
	lv_obj_add_event_cb(psktable, psk_draw_part_event, LV_EVENT_DRAW_PART_BEGIN, (void *)this);

	lv_obj_set_style_pad_top(psktable, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(psktable, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_left(psktable, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_right(psktable, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(psktable, 0, LV_PART_ITEMS);

	lv_obj_set_style_pad_left(psktable, 12, LV_PART_ITEMS | LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(psktable, 1, LV_PART_ITEMS | LV_STATE_DEFAULT);

	lv_table_set_cell_value(psktable, 0, 0, "Locator");
	lv_table_set_col_width(psktable, 0, cWidth / 8);
	lv_table_set_cell_value(psktable, 0, 1, "Call");
	lv_table_set_col_width(psktable, 1, cWidth / 8);
	lv_table_set_cell_value(psktable, 0, 2, "Frequency");
	lv_table_set_col_width(psktable, 2, cWidth / 7);
	lv_table_set_cell_value(psktable, 0, 3, "SNR");
	lv_table_set_col_width(psktable, 3, cWidth  / 12);
	lv_table_set_cell_value(psktable, 0, 4, "UTC");
	lv_table_set_col_width(psktable, 4, cWidth / 8);
}

void gui_wsjtx_setup::psk_draw_part_event_class(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_table_t *table = (lv_table_t *)obj;
	lv_obj_draw_part_dsc_t *dsc = (lv_obj_draw_part_dsc_t *)lv_event_get_param(e);
	/*If the cells are drawn...*/
	if (dsc->part == LV_PART_ITEMS)
	{
		uint32_t row = dsc->id / lv_table_get_col_cnt(obj);
		uint32_t col = dsc->id - row * lv_table_get_col_cnt(obj);

		/*Make the texts in the first cell center aligned*/
		/*MAke every 2nd row grayish*/
		if ((row != 0 && row % 2) == 0)
		{
			dsc->rect_dsc->bg_color = lv_color_mix(lv_palette_main(LV_PALETTE_GREY), dsc->rect_dsc->bg_color, LV_OPA_10);
			dsc->rect_dsc->bg_opa = LV_OPA_COVER;
		}
	}
}

void gui_wsjtx_setup::psk_button_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_CLICKED && kb == nullptr)
	{
		std::vector<ReceptionReport> reports;
		std::vector<ActiveCallsign> activeList;
		pskreporter(call, reports, activeList);
		lv_table_set_row_cnt(psktable, 1);
		int row = 1;
		for (auto col : reports)
		{
			lv_table_set_cell_value(psktable, row, 0, col.locator.c_str());
			lv_table_set_cell_value(psktable, row, 1, col.call.c_str());
			lv_table_set_cell_value(psktable, row, 2, to_string(col.frequency).c_str());
			lv_table_set_cell_value(psktable, row, 3, col.SNR.c_str());
			lv_table_set_cell_value(psktable, row, 4, col.timeUTC.c_str());
			//lv_table_set_cell_value(psktable, row, 0, col.call);
			row++;
		}
	}
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

