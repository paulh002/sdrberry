#pragma once
#include "lvgl_.h"

class ButtonBar
{
  public:
	void init(lv_obj_t *parent, int mode, lv_coord_t w, lv_coord_t h);
	void hide_buttonbar(bool enable);

  private:
	const int number_of_buttons{5};
	lv_obj_t *buttons[5], *buttonbar;
	lv_style_t button_background;
	lv_style_t style_btn;
	lv_group_t *button_group{nullptr};
};

extern ButtonBar buttonbar;