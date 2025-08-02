#pragma once
#include <lvgl_.h>
#include <string>

class tempMeter
{
  private:
	lv_obj_t *meter;
	lv_meter_scale_t *scale;
	lv_style_t scalestyle, meter_style;
	lv_obj_t  *parent_obj;
	lv_meter_indicator_t *needle;
	lv_obj_t *temp_label;
	int needle_length;

	void draw_event_cb_class(lv_event_t *e);

  public:
	lv_obj_t * init(lv_obj_t *parent, int32_t w, int32_t h);
	void set_pos(int32_t x, int32_t y);
	void set_indicator(int32_t value);
	void set_range(int32_t startvalue, int endvalue);
	void set_ticks(int32_t major, int32_t total);
	void set_section(int32_t min, int32_t max, lv_style_t *section_part_style);
	void set_labels(const char *txt_src[]);
	void set_section_range(int32_t start, int32_t end);
	
	static constexpr auto draw_event_cb = EventHandler<tempMeter, &tempMeter::draw_event_cb_class>::staticHandler;
};

