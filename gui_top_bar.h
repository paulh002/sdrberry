#pragma once
#include "lvgl.h"

extern const int screenWidth;
extern const int screenHeight;
extern const int bottomHeight;
extern const int topHeight;
extern const int tunerHeight;
extern const int rightWidth;

class gui_top_bar
{
  private:
	lv_obj_t *bg_top;
	lv_obj_t *label_status;
	lv_obj_t *label_date_time;
	lv_style_t top_style;
	lv_style_t LabelTextStyle;
	lv_style_t TimeTextStyle;
	std::string label;

  public:
	void setup_top_bar(lv_obj_t *scr);
	void set_time_label();
	void set_label_status(std::string s);
	std::string getLabel();
};

extern gui_top_bar GuiTopBar;