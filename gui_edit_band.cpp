#include "gui_edit_band.h"
#include "screen.h"
#include "Settings.h"
#include "CustomEvents.h"
#include "Keypad.h"
#include "Gui_band.h"

gui_edit_band guieditband;

void gui_edit_band::init(lv_obj_t *o_tab, lv_coord_t w, lv_coord_t h, lv_group_t *bg, lv_group_t *keyboard_group)
{
	int max_rows = 5;
	const lv_coord_t x_page_margin = 5;
	const lv_coord_t x_margin = 10;
	const lv_coord_t y_margin = 10;
	const int x_number_buttons = 5;
	const lv_coord_t tab_margin = 20;
	int cHeight = lv_obj_get_content_height(o_tab);
	int cWidth = lv_obj_get_content_width(o_tab);
	
	parent = o_tab;
	if (screenHeight < 500)
		max_rows = 4;
	keyboardgroup = keyboard_group;
	button_width_margin = (((w - 15) - tab_margin) / x_number_buttons);
	button_width = (((w - 15) - tab_margin) / x_number_buttons) - x_margin;
	button_height = h / max_rows - y_margin - y_margin; // 40;
	button_height_margin = button_height + y_margin;
	float fraction = 50.0f / 80.0f;
	selected = 0;
	
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

	lv_style_init(&tablestyle);
	lv_style_set_radius(&tablestyle, 0);
	lv_style_set_bg_color(&tablestyle, lv_color_black());

	lv_style_set_bg_grad_color(&tablestyle, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_bg_grad_dir(&tablestyle, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&tablestyle, 255);
	lv_style_set_border_color(&tablestyle, lv_color_make(0x9b, 0x36, 0x36)); // lv_color_make(0x2e, 0x44, 0xb2)
	lv_style_set_border_width(&tablestyle, 2);
	lv_style_set_border_opa(&tablestyle, 255);
	lv_style_set_outline_color(&tablestyle, lv_color_black());
	lv_style_set_outline_opa(&tablestyle, 255);
	
	
	SaveObj = lv_btn_create(o_tab);
	lv_obj_add_style(SaveObj, &style_btn, 0);
	lv_obj_add_event_cb(SaveObj, save_button_handler, LV_EVENT_CLICKED, (void *)this);
	lv_obj_align(SaveObj, LV_ALIGN_TOP_LEFT, cWidth - button_width_margin - x_margin, cHeight - button_height_margin);
	lv_obj_set_size(SaveObj, button_width, button_height);
	lv_obj_t *lv_label = lv_label_create(SaveObj);
	lv_label_set_text(lv_label, "Save");
	lv_obj_center(lv_label);

	CancelObj = lv_btn_create(o_tab);
	lv_obj_add_style(CancelObj, &style_btn, 0);
	lv_obj_add_event_cb(CancelObj, cancel_button_handler, LV_EVENT_CLICKED, (void *)this);
	lv_obj_align(CancelObj, LV_ALIGN_TOP_LEFT, cWidth - button_width_margin - x_margin, cHeight - 2 *  button_height_margin);
	lv_obj_set_size(CancelObj, button_width, button_height);
	lv_label = lv_label_create(CancelObj);
	lv_label_set_text(lv_label, "Cancel");
	lv_obj_center(lv_label);
	
	AddObj = lv_btn_create(o_tab);
	lv_obj_add_style(AddObj, &style_btn, 0);
	lv_obj_add_event_cb(AddObj, add_button_handler, LV_EVENT_CLICKED, (void *)this);
	lv_obj_align(AddObj, LV_ALIGN_TOP_LEFT, cWidth - button_width_margin - x_margin, cHeight - 4 * button_height_margin);
	lv_obj_set_size(AddObj, button_width, button_height);
	lv_label = lv_label_create(AddObj);
	lv_label_set_text(lv_label, "Add");
	lv_obj_center(lv_label);

	DelObj = lv_btn_create(o_tab);
	lv_obj_add_style(DelObj, &style_btn, 0);
	lv_obj_add_event_cb(DelObj, del_button_handler, LV_EVENT_CLICKED, (void *)this);
	lv_obj_align(DelObj, LV_ALIGN_TOP_LEFT, cWidth - button_width_margin - x_margin, cHeight - 3 * button_height_margin);
	lv_obj_set_size(DelObj, button_width, button_height);
	lv_label = lv_label_create(DelObj);
	lv_label_set_text(lv_label, "Delete");
	lv_obj_center(lv_label);
	
	table = lv_table_create(o_tab);
	lv_obj_add_style(table, &tablestyle, 0);
	lv_obj_set_pos(table, x_page_margin, y_margin);
	lv_obj_set_size(table, cWidth * fraction, cHeight - y_margin);
	//lv_obj_add_event_cb(table, table_draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, (void *)this);
	lv_obj_add_event_cb(table, table_press_part_event_cb, LV_EVENT_PRESSED, (void *)this);

	lv_obj_set_style_pad_top(table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_left(table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_right(table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(table, 0, LV_PART_ITEMS);

	lv_obj_set_style_pad_left(table, 0, LV_PART_ITEMS);
	lv_obj_set_style_pad_right(table, 0, LV_PART_ITEMS);

	lv_table_set_cell_value(table, 0, 0, "Band");
	lv_table_set_col_width(table, 0, cWidth / 16);
	lv_table_set_cell_value(table, 0, 1, "Unit");
	lv_table_set_col_width(table, 1, cWidth / 16);
	lv_table_set_cell_value(table, 0, 2, "F low");
	lv_table_set_col_width(table, 2, cWidth / 5);
	lv_table_set_cell_value(table, 0, 3, "F high");
	lv_table_set_col_width(table, 3, cWidth / 5);
	lv_table_set_cell_value(table, 0, 4, "Mode");
	lv_table_set_col_width(table, 4, cWidth / 16);
	fill_band_table();

	lv_obj_add_event_cb(table, table_custom_keypad_handler, customLVevents.getCustomEvent(LV_KEYPAD_EVENT_CUSTOM), (void *)this);
	lv_obj_add_event_cb(table, table_draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, (void *)this);
	
}

void gui_edit_band::fill_band_table()
{
	int band;
	std::string label;
	auto it_m = begin(Settings_file.labels);
	auto it_mode = begin(Settings_file.mode);
	auto it_f_low = begin(Settings_file.f_low);
	auto it_f_high = begin(Settings_file.f_high);
	for (int i = 0; i < Settings_file.meters.size(); i++)
	{
		band = Settings_file.meters.at(i);
		label = (string)*it_m;
		long f_low = (long)*it_f_low;
		long f_high = (long)*it_f_high;
		lv_table_set_cell_value(table, 1 + i, 0, to_string(band).c_str());
		lv_table_set_cell_value(table, 1 + i, 1, label.c_str());
		lv_table_set_cell_value(table, 1 + i, 2, to_string(f_low).c_str());
		lv_table_set_cell_value(table, 1 + i, 3, to_string(f_high).c_str());
		lv_table_set_cell_value(table, 1 + i, 4, it_mode->c_str());
		it_m++;
		it_f_low++;
		it_f_high++;
		it_mode++;
	}
}

void gui_edit_band::save_button_handler_class(lv_event_t *e)
{
	int count = lv_table_get_row_cnt(table);
	Settings_file.meters.clear();
	Settings_file.labels.clear();
	Settings_file.f_low.clear();
	Settings_file.f_high.clear();
	Settings_file.mode.clear();
	
	for (int i = 1; i < count; i++) // skip header
	{
		Settings_file.meters.push_back(atoi(lv_table_get_cell_value(table, i, 0)));
		Settings_file.labels.push_back(std::string(lv_table_get_cell_value(table, i, 1)));
		Settings_file.f_low.push_back(atol(lv_table_get_cell_value(table, i, 2)));
		Settings_file.f_high.push_back(atol(lv_table_get_cell_value(table, i, 3)));
		Settings_file.mode.push_back(std::string(lv_table_get_cell_value(table, i, 4)));
	}
	Settings_file.save_band();
	gui_band_instance.reload_buttons();
}

void gui_edit_band::cancel_button_handler_class(lv_event_t *e)
{
	lv_table_set_row_cnt(table, 1);
	fill_band_table();
}

void gui_edit_band::table_press_part_event_cb_class(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	uint16_t row, col;
	int count;
	
	lv_table_get_selected_cell(obj, &row, &col);
	count = lv_table_get_row_cnt(obj);
	if (col == 0 && row > 0 && row < count && row == selected)
	{
		CreateKeyPadWindow(lv_scr_act(), table, keyboardgroup, 1);
		column = 0;
		rowno = row;
	}
	if (col == 2 && row > 0 && row < count && row == selected)
	{
		CreateKeyPadWindow(lv_scr_act(), table, keyboardgroup, 0);
		column = 2;
		rowno = row;
	}
	if (col == 3 && row > 0 && row < count && row == selected)
	{
		CreateKeyPadWindow(lv_scr_act(), table, keyboardgroup, 0);
		column = 3;
		rowno = row;
	}
	if (col == 4 && row > 0 && row < count && row == selected)
	{
		std::string str(lv_table_get_cell_value(obj, row, 4));
		if (str == "lsb")
		{
			lv_table_set_cell_value(obj, row, col, "usb");
		}
		else if (str == "usb")
		{
			lv_table_set_cell_value(obj, row, col, "dsb");
		}
		else if (str == "dsb")
		{
			lv_table_set_cell_value(obj, row, col, "am");
		}
		else if (str == "am")
		{
			lv_table_set_cell_value(obj, row, col, "fm");
		}
		else if (str == "fm")
		{
			lv_table_set_cell_value(obj, row, col, "lsb");
		}
	}
	if (col == 1 && row > 0 && row < count && row == selected)
	{
		std::string str(lv_table_get_cell_value(obj, row, 1));
		if (str == "m")
		{
			lv_table_set_cell_value(obj, row, col, "cm");
		}
		else if (str == "cm")
		{
			lv_table_set_cell_value(obj, row, col, "m");
		}
	}
	if (selected != row && row > 0)
		selected = row;
}

void gui_edit_band::table_custom_keypad_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	long f = (long)lv_event_get_param(e);

	if (code == customLVevents.getCustomEvent(LV_KEYPAD_EVENT_CUSTOM))
	{
		switch (column)
		{
		case 0:
			if (f < 999)
				lv_table_set_cell_value(table, rowno, column, to_string(f).c_str());
			break;
		case 2:
		case 3:
			lv_table_set_cell_value(table, rowno, column, to_string(f).c_str());
			break;
		}
	}
}

void gui_edit_band::table_draw_part_event_cb_class(lv_event_t *e)
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
		if (row == 0)
		{
			dsc->rect_dsc->bg_color = lv_color_mix(lv_palette_main(LV_PALETTE_CYAN), dsc->rect_dsc->bg_color, LV_OPA_10);
			dsc->rect_dsc->bg_opa = LV_OPA_COVER;
		}
		/*MAke every 2nd row grayish*/
		if ((row != 0 && row % 2) == 0)
		{
			dsc->rect_dsc->bg_color = lv_color_mix(lv_palette_main(LV_PALETTE_GREY), dsc->rect_dsc->bg_color, LV_OPA_10);
			dsc->rect_dsc->bg_opa = LV_OPA_COVER;
		}

		if (row == selected && row > 0)
		{
			dsc->rect_dsc->bg_color = lv_color_mix(lv_palette_main(LV_PALETTE_ORANGE), dsc->rect_dsc->bg_color, LV_OPA_50);
			dsc->rect_dsc->bg_opa = LV_OPA_COVER;
		}
	}
}

void gui_edit_band::add_button_handler_class(lv_event_t *e)
{
	int count = lv_table_get_row_cnt(table);
	lv_table_set_row_cnt(table, count + 1);
	if (selected > 0)
	{
		do
		{
			lv_table_set_cell_value(table, count, 0, lv_table_get_cell_value(table, count - 1, 0));
			lv_table_set_cell_value(table, count, 1, lv_table_get_cell_value(table, count - 1, 1));
			lv_table_set_cell_value(table, count, 2, lv_table_get_cell_value(table, count - 1, 2));
			lv_table_set_cell_value(table, count, 3, lv_table_get_cell_value(table, count - 1, 3));
			lv_table_set_cell_value(table, count, 4, lv_table_get_cell_value(table, count - 1, 4));
			count--;
		} while (count > selected);
	}
}

void gui_edit_band::del_button_handler_class(lv_event_t *e)
{
	int count = lv_table_get_row_cnt(table);
	
	if (selected > 0 && count > 1)
	{
		int row = selected;
		do
		{
			if (row < count - 1)
			{
				lv_table_set_cell_value(table, row, 0, lv_table_get_cell_value(table, row + 1, 0));
				lv_table_set_cell_value(table, row, 1, lv_table_get_cell_value(table, row + 1, 1));
				lv_table_set_cell_value(table, row, 2, lv_table_get_cell_value(table, row + 1, 2));
				lv_table_set_cell_value(table, row, 3, lv_table_get_cell_value(table, row + 1, 3));
				lv_table_set_cell_value(table, row, 4, lv_table_get_cell_value(table, row + 1, 4));
			}
			row++;
		} while (row < count);
		lv_table_set_row_cnt(table, count - 1);
		if (selected >= count - 1)
			selected--;
	}
}