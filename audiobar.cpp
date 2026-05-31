#include "audiobar.h"


LV_IMG_DECLARE(ui_img_indicator_hor); // assets\indicator_hor.png

void audiobar::init(lv_obj_t *parent, lv_coord_t w, lv_coord_t h)
{
	bar = lv_slider_create(parent);
	lv_slider_set_value(bar, 25, LV_ANIM_OFF);
	if (lv_slider_get_mode(bar) == LV_SLIDER_MODE_RANGE)
		lv_slider_set_left_value(bar, 0, LV_ANIM_OFF);
	lv_obj_set_width(bar, 290);
	lv_obj_set_height(bar, 30);
	lv_obj_set_x(bar, lv_pct(1));
	lv_obj_set_y(bar, lv_pct(14));
	lv_obj_set_align(bar, LV_ALIGN_CENTER);
	lv_obj_set_style_radius(bar, 4, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(bar, lv_color_hex(0x18181A), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(bar, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_main_stop(bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_stop(bar, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_pad_left(bar, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_pad_right(bar, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_pad_top(bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_pad_bottom(bar, 0, LV_PART_MAIN | LV_STATE_DEFAULT);

	lv_obj_set_style_radius(bar, 4, LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_color(bar, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(bar, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_main_stop(bar, 0, LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_grad_stop(bar, 255, LV_PART_INDICATOR | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_img_src(bar, &ui_img_indicator_hor, LV_PART_INDICATOR | LV_STATE_DEFAULT);

	lv_obj_set_style_bg_color(bar, lv_color_hex(0xFFFFFF), LV_PART_KNOB | LV_STATE_DEFAULT);
	lv_obj_set_style_bg_opa(bar, 0, LV_PART_KNOB | LV_STATE_DEFAULT);

	lv_obj_set_style_border_color(bar, lv_palette_main(LV_PALETTE_CYAN), LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_opa(bar, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_set_style_border_width(bar, 2, LV_PART_MAIN | LV_STATE_DEFAULT);
}

void audiobar::align(lv_obj_t *parent, lv_align_t align, lv_coord_t x_ofs, lv_coord_t y_ofs)
{
	lv_obj_align_to(bar, parent, align, x_ofs, y_ofs);
}

void audiobar::value(int value)
{
	if (value > maxvalue)
		value = maxvalue;
	if (value < 0)
		value = 0;

	lv_bar_set_value(bar, value, LV_ANIM_ON);
}

void audiobar::SetRange(int range)
{
	maxvalue = range;
	lv_bar_set_range(bar, 0, range);
}
