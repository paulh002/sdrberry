#pragma once
#include "lvgl_.h"
#include <vector>

class meter
{
  private:
	lv_obj_t *scale_line, *bottom_line;
	lv_style_t scale_style, bottom_style, blue_style, red_style, green_style;
	lv_obj_t *needle_line, *parent_obj;
	std::vector<lv_scale_section_t *> sections;
	lv_scale_section_t *blue_section, *red_section, *green_section_top, *red_section_top;
	int needle_length;

  public:
	void init(lv_obj_t *parent, int32_t x, int32_t y, int32_t w, int32_t h);
	void set_needle(int32_t value);
	void set_range(int32_t startvalue, int endvalue);
	void set_ticks(int32_t major, int32_t total);
	void set_section(int32_t min, int32_t max, lv_style_t *section_part_style);
	void set_arc_witdh(int32_t value);
	int add_section(int32_t min, int32_t max, lv_style_t *section_part_style);
	void set_section_offset(uint32_t section, int32_t offset);
};
