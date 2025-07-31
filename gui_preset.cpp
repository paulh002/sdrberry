#include "gui_preset.h"
#include "vfo.h"
#include "VfoKeyPad.h"
#include "table.h"
#include "Settings.h"

gui_preset guipreset;

void gui_preset::init(lv_obj_t *o_tab, lv_coord_t w, lv_coord_t h, lv_group_t *bg, lv_group_t *keyboard_group)
{
	const lv_coord_t x_page_margin = 5;
	const lv_coord_t x_margin = 10;
	const lv_coord_t y_margin = 5;
	const int x_number_buttons = 6;
	const int y_number_buttons = 4;
	const lv_coord_t tab_margin = 20;

	int button_width_margin = ((w - tab_margin) / x_number_buttons);
	int button_width = ((w - tab_margin) / x_number_buttons) - x_margin;
	int button_height = 30;
	int button_height_margin = button_height + y_margin;
	int ibutton_x = 0, ibutton_y = 0;
	int cHeight = lv_obj_get_content_height(o_tab);
	int cWidth = lv_obj_get_content_width(o_tab);
	float fraction = 14.0f / 80.0f;

	lv_obj_set_style_pad_top(o_tab, 10, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(o_tab, 5, LV_PART_MAIN);
	lv_obj_set_style_pad_right(o_tab, 5, LV_PART_MAIN);

	tileview = lv_tileview_create(o_tab);
	lv_obj_clear_flag(tileview, LV_OBJ_FLAG_SCROLL_ELASTIC);
	main_tile = lv_tileview_add_tile(tileview, 0, 0, LV_DIR_BOTTOM | LV_DIR_TOP);
	edit_tile = lv_tileview_add_tile(tileview, 0, 1, LV_DIR_BOTTOM | LV_DIR_TOP);

	lv_obj_set_style_pad_top(main_tile, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_top(edit_tile, 0, LV_PART_MAIN);

	row_selected = display_row_selected = 0;
	keyboardgroup = keyboard_group;
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

	SaveObj = lv_btn_create(edit_tile);
	lv_obj_add_style(SaveObj, &style_btn, 0);
	lv_obj_add_event_cb(SaveObj, save_button_handler, LV_EVENT_CLICKED, (void *)this);
	lv_obj_align(SaveObj, LV_ALIGN_TOP_LEFT, cWidth - button_width_margin - x_margin, cHeight - button_height_margin);
	lv_obj_set_size(SaveObj, button_width, button_height);
	lv_obj_t *lv_label = lv_label_create(SaveObj);
	lv_label_set_text(lv_label, "Save");
	lv_obj_center(lv_label);

	deleteButton = lv_btn_create(edit_tile);
	lv_obj_add_style(deleteButton, &style_btn, 0);
	lv_obj_add_event_cb(deleteButton, delete_button_handler, LV_EVENT_CLICKED, (void *)this);
	lv_obj_align(deleteButton, LV_ALIGN_TOP_LEFT, cWidth - button_width_margin - x_margin, cHeight - 4 * button_height_margin);
	lv_obj_set_size(deleteButton, button_width, button_height);
	lv_label = lv_label_create(deleteButton);
	lv_label_set_text(lv_label, "Delete");
	lv_obj_center(lv_label);

	updateButton = lv_btn_create(edit_tile);
	lv_obj_add_style(updateButton, &style_btn, 0);
	lv_obj_add_event_cb(updateButton, update_button_handler, LV_EVENT_CLICKED, (void *)this);
	lv_obj_align(updateButton, LV_ALIGN_TOP_LEFT, cWidth - button_width_margin - x_margin, cHeight - 3 * button_height_margin);
	lv_obj_set_size(updateButton, button_width, button_height);
	lv_label = lv_label_create(updateButton);
	lv_label_set_text(lv_label, "Update");
	lv_obj_center(lv_label);

	addButton = lv_btn_create(edit_tile);
	lv_obj_add_style(addButton, &style_btn, 0);
	lv_obj_add_event_cb(addButton, add_button_handler, LV_EVENT_CLICKED, (void *)this);
	lv_obj_align(addButton, LV_ALIGN_TOP_LEFT, cWidth - button_width_margin - x_margin, cHeight - 2 * button_height_margin);
	lv_obj_set_size(addButton, button_width, button_height);
	lv_label = lv_label_create(addButton);
	lv_label_set_text(lv_label, "Add");
	lv_obj_center(lv_label);
	
	lv_style_init(&tablestyle);
	lv_style_set_radius(&tablestyle, 0);
	lv_style_set_bg_color(&tablestyle, lv_color_black());
	// lv_style_set_bg_color(&tablestyle, lv_color_make(0x60, 0x60, 0x60));
	lv_style_set_bg_grad_color(&tablestyle, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_bg_grad_dir(&tablestyle, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&tablestyle, 255);
	lv_style_set_border_color(&tablestyle, lv_color_make(0x9b, 0x36, 0x36)); // lv_color_make(0x2e, 0x44, 0xb2)
	lv_style_set_border_width(&tablestyle, 2);
	lv_style_set_border_opa(&tablestyle, 255);
	lv_style_set_outline_color(&tablestyle, lv_color_black());
	lv_style_set_outline_opa(&tablestyle, 255);

	float fraction1 = 60.0f / 80.0f;
	preset_table = lv_table_create(edit_tile);
	lv_obj_add_style(preset_table, &tablestyle, 0);
	lv_obj_clear_flag(preset_table, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_pos(preset_table, x_margin, y_margin + ibutton_y * button_height_margin);
	lv_obj_set_size(preset_table, cWidth * fraction1, cHeight - (y_margin + ibutton_y * button_height_margin));
	lv_obj_add_event_cb(preset_table, table_draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, (void *)this);
	lv_obj_add_event_cb(preset_table, table_press_part_event_cb, LV_EVENT_PRESSED, (void *)this);

	lv_obj_set_style_pad_top(preset_table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(preset_table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_left(preset_table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_right(preset_table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(preset_table, 0, LV_PART_ITEMS);

	lv_obj_set_style_pad_left(preset_table, 20, LV_PART_ITEMS);
	lv_obj_set_style_pad_right(preset_table, 0, LV_PART_ITEMS);

	lv_table_set_cell_value(preset_table, 0, 0, "Preset");
	lv_table_set_col_width(preset_table, 0, cWidth / 10);
	lv_table_set_cell_value(preset_table, 0, 1, "Band");
	lv_table_set_col_width(preset_table, 1, cWidth / 10);
	lv_table_set_cell_value(preset_table, 0, 2, "Frequency");
	lv_table_set_col_width(preset_table, 2, cWidth * fraction1 - (cWidth / 12));

	display_table = lv_table_create(main_tile);
	lv_obj_add_style(display_table, &tablestyle, 0);
	lv_obj_clear_flag(display_table, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_set_pos(display_table, x_margin, y_margin + ibutton_y * button_height_margin);
	lv_obj_set_size(display_table, cWidth * fraction1, cHeight - (y_margin + ibutton_y * button_height_margin));
	lv_obj_add_event_cb(display_table, display_draw_part_event_cb, LV_EVENT_DRAW_PART_BEGIN, (void *)this);
	lv_obj_add_event_cb(display_table, display_press_part_event_cb, LV_EVENT_PRESSED, (void *)this);

	lv_obj_set_style_pad_top(display_table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(display_table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_left(display_table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_right(display_table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(display_table, 0, LV_PART_ITEMS);

	lv_obj_set_style_pad_left(display_table, 20, LV_PART_ITEMS);
	lv_obj_set_style_pad_right(display_table, 0, LV_PART_ITEMS);

	lv_table_set_cell_value(display_table, 0, 0, "Preset");
	lv_table_set_col_width(display_table, 0, cWidth / 10);
	lv_table_set_cell_value(display_table, 0, 1, "Band");
	lv_table_set_col_width(display_table, 1, cWidth / 10);
	lv_table_set_cell_value(display_table, 0, 2, "Frequency");
	lv_table_set_col_width(display_table, 2, cWidth * fraction1 - (cWidth / 12));
	
	

	preset_list = Settings_file.get_map_string_pair("preselect", "list");
	int i = 1;
	for (auto col : preset_list)
	{
		lv_table_set_cell_value(preset_table, i, 0, std::to_string(i).c_str());
		lv_table_set_cell_value(preset_table, i, 1, col.second.first.c_str());

		lv_table_set_cell_value(display_table, i, 0, std::to_string(i).c_str());
		lv_table_set_cell_value(display_table, i, 1, col.second.first.c_str());

		char str[80];
		long freq = col.second.second;
		if (freq > 10000000LU)
		{
			sprintf(str, "%3ld.%03ld,%02ld", (long)(freq / 1000000), (long)((freq / 1000) % 1000), (long)((freq / 10) % 100));
		}
		else
		{
			sprintf(str, "%3ld.%03ld,%02ld", (long)(freq / 1000000), (long)((freq / 1000) % 1000), (long)((freq / 10) % 100));
		}
		lv_table_set_cell_value(preset_table, i, 2, str);
		lv_table_set_cell_value(display_table, i, 2, str);
		i++;
	}
	lv_obj_set_tile_id(tileview, 0, 0, LV_ANIM_OFF);
	//lv_group_add_obj(bg, addressObj);
}

void gui_preset::reload_display_table()
{
	int i = 1;
	for (auto col : preset_list)
	{
		lv_table_set_cell_value(display_table, i, 0, std::to_string(i).c_str());
		lv_table_set_cell_value(preset_table, i, 1, col.second.first.c_str());

		char str[80];
		long freq = col.second.second;
		if (freq > 10000000LU)
		{
			sprintf(str, "%3ld.%03ld,%02ld", (long)(freq / 1000000), (long)((freq / 1000) % 1000), (long)((freq / 10) % 100));
		}
		else
		{
			sprintf(str, "%3ld.%03ld,%02ld", (long)(freq / 1000000), (long)((freq / 1000) % 1000), (long)((freq / 10) % 100));
		}
		lv_table_set_cell_value(display_table, i, 2, str);
		i++;
	}
}

void gui_preset::save_button_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_CLICKED)
	{
		Settings_file.set_map_string_pair("preselect", "list", preset_list);
		reload_display_table();
	}
}

void gui_preset::add_button_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_CLICKED)
	{
		int preset = 1;
		long freq = vfo.get_active_vfo_freq();		
		uint32_t row = lv_table_get_row_cnt(preset_table);
		if (row > 1)
		{
			preset = row;
		}
		
		char str[80];
		if (freq > 10000000LU)
		{
			sprintf(str, "%3ld.%03ld,%02ld", (long)(freq / 1000000), (long)((freq / 1000) % 1000), (long)((freq / 10) % 100));
		}
		else
		{
			sprintf(str, "%3ld.%03ld,%02ld", (long)(freq / 1000000), (long)((freq / 1000) % 1000), (long)((freq / 10) % 100));
		}
		lv_table_set_cell_value(preset_table, preset, 0, std::to_string(row).c_str());
		lv_table_set_cell_value(preset_table, preset, 1, vfo.get_band_in_text().c_str());
		lv_table_set_cell_value(preset_table, preset, 2, str);

		preset_list[preset].first = vfo.get_band_in_text();
		preset_list[preset].second = freq;
	}
}

void gui_preset::update_button_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_CLICKED)
	{
		long freq = vfo.get_active_vfo_freq();
		if (row_selected > 0)
		{
			char str[80];
			if (freq > 10000000LU)
			{
				sprintf(str, "%3ld.%03ld,%02ld", (long)(freq / 1000000), (long)((freq / 1000) % 1000), (long)((freq / 10) % 100));
			}
			else
			{
				sprintf(str, "%3ld.%03ld,%02ld", (long)(freq / 1000000), (long)((freq / 1000) % 1000), (long)((freq / 10) % 100));
			}
			lv_table_set_cell_value(preset_table, row_selected, 0, std::to_string(row_selected).c_str());
			lv_table_set_cell_value(preset_table, row_selected, 1, std::to_string(vfo.get_band_in_meters()).c_str());
			lv_table_set_cell_value(preset_table, row_selected, 2, str);

			preset_list[row_selected].first = vfo.get_band_in_text();
			preset_list[row_selected].second = freq;
			row_selected = 0;
		}
	}
}

void gui_preset::delete_button_handler_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);
	if (code == LV_EVENT_CLICKED)
	{
		if (row_selected > 0)
		{
			lv_table_remove_rows(preset_table, row_selected, 1);
			if (lv_table_get_row_cnt(preset_table) > 1)
			{
				for (int i = 1; i < lv_table_get_row_cnt(preset_table); i++)
				{
					lv_table_set_cell_value(preset_table, i, 0, std::to_string(i).c_str());
				}
				lv_obj_invalidate(preset_table);
				preset_list.erase(row_selected);
			}
			row_selected = 0;
		}
	}
}

void gui_preset::table_press_part_event_class(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_table_t *table = (lv_table_t *)obj;
	uint16_t row, col;
	
	// Do something
	lv_table_get_selected_cell(obj, &row, &col);
	if (lv_table_get_row_cnt(obj) < (row + 1) || row == 0)
		return;
	row_selected = row;
}

void gui_preset::table_draw_part_event_class(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_table_t *table = (lv_table_t *)obj;
	lv_obj_draw_part_dsc_t *dsc = (lv_obj_draw_part_dsc_t *)lv_event_get_param(e);
	/*If the cells are drawn...*/
	if (dsc->part == LV_PART_ITEMS)
	{
		uint32_t row = dsc->id / lv_table_get_col_cnt(obj);
		uint32_t col = dsc->id - row * lv_table_get_col_cnt(obj);
		if (row == row_selected && row > 0)
		{
			dsc->rect_dsc->bg_color = lv_color_mix(lv_palette_main(LV_PALETTE_ORANGE), dsc->rect_dsc->bg_color, LV_OPA_30);
			dsc->rect_dsc->bg_opa = LV_OPA_COVER;
		}
	}
}

void gui_preset::display_press_part_event_class(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_table_t *table = (lv_table_t *)obj;
	uint16_t row, col;

	// Do something
	lv_table_get_selected_cell(obj, &row, &col);
	if (lv_table_get_row_cnt(obj) < (row + 1) || row == 0)
		return;
	display_row_selected = row;

	std::string s = std::string(lv_table_get_cell_value(display_table, row, 2));
	s = strlib::remove_non_digits(s);
	long freq = std::atol(s.c_str()) * 10;
	if (vfo.is_vfo_limit_ham_band())
	{
		if (vfo.checkVfoBandRange(freq))
		{
			vfo.set_band_freq(freq);
		}
		else
		{
			static const char *btns[] = {""};
			lv_obj_t *mbox1 = lv_msgbox_create(NULL, "VFO", "Out of Ham band range", btns, true);
			lv_obj_center(mbox1);
		}
	}
	else
	{
		vfo.set_vfo(freq);
	}
}
	
void gui_preset::display_draw_part_event_class(lv_event_t *e)
{
	lv_obj_t *obj = lv_event_get_target(e);
	lv_table_t *table = (lv_table_t *)obj;
	lv_obj_draw_part_dsc_t *dsc = (lv_obj_draw_part_dsc_t *)lv_event_get_param(e);
	/*If the cells are drawn...*/
	if (dsc->part == LV_PART_ITEMS)
	{
		uint32_t row = dsc->id / lv_table_get_col_cnt(obj);
		uint32_t col = dsc->id - row * lv_table_get_col_cnt(obj);
		if (row == display_row_selected && row > 0)
		{
			dsc->rect_dsc->bg_color = lv_color_mix(lv_palette_main(LV_PALETTE_ORANGE), dsc->rect_dsc->bg_color, LV_OPA_30);
			dsc->rect_dsc->bg_opa = LV_OPA_COVER;
		}
	}
}
