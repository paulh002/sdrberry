#include "meter.h"
#include "screen.h"
#include <stdio.h>

void meter::init(lv_obj_t *parent, int32_t x, int32_t y, int32_t w, int32_t h)
{
	parent_obj = parent;
	scale_line = lv_scale_create(parent);
	lv_scale_set_mode(scale_line, LV_SCALE_MODE_ROUND_OUTER);

	lv_obj_set_pos(scale_line, 0, 0);
	lv_obj_set_size(scale_line, w, h );
	lv_obj_clear_flag(scale_line, LV_OBJ_FLAG_SCROLLABLE);
	
	lv_style_init(&scale_style);
	lv_style_set_radius(&scale_style, 0);
	lv_style_set_bg_color(&scale_style, lv_color_black());
	lv_style_set_bg_opa(&scale_style, LV_OPA_TRANSP);
	lv_obj_add_style(scale_line, &scale_style, 0);
	lv_obj_set_style_pad_hor(scale_line, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(scale_line, 0, LV_PART_MAIN);
	//lv_obj_set_style_size(scale_line, lv_area_get_width(&area), LV_PART_MAIN);
	//lv_scale_set_range(scale_line, 0, 15);
	lv_scale_set_angle_range(scale_line, 55);
	lv_scale_set_rotation(scale_line, 242);
	int center_pos, center_pos_bottom, arc_width = 4, arc_width_color = 8, bottom_y = 50, pad_top = 30, pad_radial = 0;
	needle_length = 260;
	if (screenWidth > 1440)
	{
		center_pos = 350;
		center_pos_bottom = 350;
		needle_length = 500;
	}
	else if (screenWidth > 1280)
	{
		center_pos = 250;
		center_pos_bottom = 250;
	}
	else if (screenWidth > 800)
	{
		center_pos = 250;
		center_pos_bottom = 250;
	}
	else
	{
		center_pos = 200;
		center_pos_bottom = 200;
		arc_width = 2;
		arc_width_color = 2;
		bottom_y = 20;
		pad_top = 20;
		needle_length = 220;
		pad_radial = -10;
	}

	static const char *custom_labels[9] = {"S", "1", "3", "5", "7", "9", "20", "40 db", NULL};
	lv_scale_set_text_src(scale_line, custom_labels);

	lv_obj_set_style_pad_radial(scale_line, pad_radial, LV_PART_INDICATOR);
	lv_obj_set_style_pad_top(scale_line, pad_top, LV_PART_MAIN);
	lv_obj_set_style_pad_bottom(scale_line, 0, LV_PART_INDICATOR);
	lv_scale_set_center_pos(scale_line, 0, center_pos);
	//lv_obj_add_event_cb(scale_line, draw_event_cb, LV_EVENT_DRAW_TASK_ADDED, (void *)this);
	lv_obj_add_flag(scale_line, LV_OBJ_FLAG_SEND_DRAW_TASK_EVENTS);
	lv_style_set_arc_width(&scale_style, arc_width);
	
	lv_style_init(&red_style);
	lv_style_set_arc_color(&red_style, lv_palette_main(LV_PALETTE_RED));
	lv_style_set_arc_width(&red_style, arc_width_color /*8U*/); /*Tick width*/

	lv_style_init(&green_style);
	lv_style_set_arc_color(&green_style, lv_palette_main(LV_PALETTE_LIGHT_GREEN));
	lv_style_set_arc_width(&green_style, arc_width_color /*8U*/); /*Tick width*/

	lv_scale_set_range(scale_line, 0, 100);
	lv_scale_set_total_tick_count(scale_line, 15);
	lv_scale_set_major_tick_every(scale_line, 2);

	red_section_top = lv_scale_add_section(scale_line);
	green_section_top = lv_scale_add_section(scale_line);
	lv_scale_section_set_range(red_section_top, 72, 100);
	lv_scale_section_set_range(green_section_top, 0, 72);
	lv_scale_set_section_offset(scale_line, red_section_top, -5);
	lv_scale_set_section_offset(scale_line, green_section_top, -5);
	lv_scale_set_section_style_main(scale_line, green_section_top, &green_style);
	lv_scale_set_section_style_main(scale_line, red_section_top, &red_style);

	needle_line = lv_line_create(scale_line);
	lv_obj_update_layout(scale_line);
	lv_obj_set_style_line_width(needle_line, 3, LV_PART_MAIN);
	lv_obj_set_style_line_rounded(needle_line, true, LV_PART_MAIN);
	lv_obj_update_layout(needle_line);

	lv_style_init(&bottom_style);
	lv_style_set_radius(&bottom_style, 0);
	lv_style_set_bg_color(&bottom_style, lv_color_black());
	lv_style_set_bg_opa(&bottom_style, LV_OPA_TRANSP);

	bottom_line = lv_scale_create(parent);
	lv_scale_set_mode(bottom_line, LV_SCALE_MODE_ROUND_OUTER);
	lv_obj_set_pos(bottom_line, 50, bottom_y); // 50
	lv_obj_set_size(bottom_line, w - 100, h - 100);
	lv_obj_clear_flag(bottom_line, LV_OBJ_FLAG_SCROLLABLE);
	lv_obj_add_style(bottom_line, &bottom_style, 0);
	lv_obj_set_style_pad_hor(bottom_line, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(bottom_line, 0, LV_PART_MAIN);
	// lv_obj_set_style_size(scale_line, lv_area_get_width(&area), LV_PART_MAIN);
	lv_obj_set_style_pad_top(bottom_line, 30, LV_PART_MAIN);
	lv_scale_set_range(bottom_line, 0, 10);
	lv_scale_set_angle_range(bottom_line, 55);
	lv_scale_set_rotation(bottom_line, 242);
	lv_scale_set_center_pos(bottom_line, 0, center_pos_bottom);
	lv_scale_set_total_tick_count(bottom_line, 11);
	lv_scale_set_major_tick_every(bottom_line, 2);
	lv_obj_set_style_pad_radial(bottom_line, pad_radial, LV_PART_INDICATOR);
	
	lv_style_init(&blue_style);
	lv_style_set_arc_color(&blue_style, lv_palette_main(LV_PALETTE_BLUE));
	lv_style_set_arc_width(&blue_style, arc_width_color); /*Tick width*/
	blue_section = lv_scale_add_section(bottom_line);
	lv_scale_section_set_range(blue_section, 0, 8);
	lv_scale_set_section_style_main(bottom_line, blue_section, &blue_style);
	red_section = lv_scale_add_section(bottom_line);
	lv_scale_section_set_range(red_section, 8, 10);
	lv_scale_set_section_style_main(bottom_line, red_section, &red_style);
	lv_scale_set_section_offset(bottom_line, red_section, - 5);
	lv_scale_set_section_offset(bottom_line, blue_section, -5);

	lv_obj_move_foreground(scale_line);
}

void meter::set_needle(int32_t value)
{
	lv_scale_set_line_needle_value(scale_line, needle_line, needle_length, value);
}


