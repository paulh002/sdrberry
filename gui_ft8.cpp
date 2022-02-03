#include "gui_ft8.h"

extern const int screenWidth;
extern const int screenHeight;
extern const int bottomHeight;
extern const int topHeight;
extern const int tunerHeight;

gui_ft8 gft8;

void gui_ft8::init(lv_obj_t *o_tab, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h)
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

	lv_style_init(&ft8_style);
	lv_style_set_radius(&ft8_style, 0);
	lv_style_set_bg_color(&ft8_style, lv_color_black());
	
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
	//m_button_group = lv_group_create();
	lv_obj_set_style_pad_hor(o_tab, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(o_tab, 0, LV_PART_MAIN);
	
	table = lv_table_create(o_tab);
	lv_obj_add_style(table, &ft8_style, 0);
	//lv_obj_align(table, LV_ALIGN_TOP_LEFT, w, h);
	lv_obj_set_pos(table, x, y);
	lv_obj_set_size(table, w, h-130);

	lv_obj_set_style_pad_top(table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_left(table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_right(table, 2, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(table, 1, LV_PART_ITEMS);
	
	lv_table_set_cell_value(table, 0, 0, "Time");
	lv_table_set_col_width(table, 0, w/8);
	lv_table_set_cell_value(table, 0, 1, "snr");
	lv_table_set_col_width(table, 1, w/12);
	lv_table_set_cell_value(table, 0, 2, "Time");
	lv_table_set_col_width(table, 2, w/10);
	lv_table_set_cell_value(table, 0, 3, "Offset");
	lv_table_set_col_width(table, 3, w/9);
	lv_table_set_cell_value(table, 0, 4, "Hz");
	lv_table_set_col_width(table, 4, w/8);
	lv_table_set_cell_value(table, 0, 5, "Message");
	lv_table_set_col_width(table, 5, (int)((float)w/2.3));
	m_cycle_count++;
}

void gui_ft8::add_line(int hh, int min, int sec, int snr, int correct_bits, double off,double hz0, string msg)
{
	char str[128];
	
	unique_lock<mutex> gui_lock(gui_mutex);
	if (bclear)
	{
		lv_table_set_row_cnt(table, 1);
		m_cycle_count = 1;
		bclear = false;
	}
	sprintf(str,"%02d:%02d:%02d", hh, min, sec);
	lv_table_set_cell_value(table, m_cycle_count, 0, str);

	sprintf(str,"%3d",snr);
	lv_table_set_cell_value(table, m_cycle_count, 1, str);

	sprintf(str, "%3d", correct_bits);
	lv_table_set_cell_value(table, m_cycle_count, 2, str);

	sprintf(str, "%5.2f", off);
	lv_table_set_cell_value(table, m_cycle_count, 3, str);

	sprintf(str, "%6.1f", hz0);
	lv_table_set_cell_value(table, m_cycle_count, 4, str);

	sprintf(str, "%6.1f", hz0);
	lv_table_set_cell_value(table, m_cycle_count, 5, msg.c_str());

	m_cycle_count++;
}

void gui_ft8::clear()
{
	
	bclear = true;
}