#pragma once
#include "lvgl_.h"

class ButtonBar
{
  private:
	const int number_of_buttons{4};
	lv_obj_t *buttons[5], *buttonbar;
	lv_style_t button_background;
	lv_style_t style_btn;
	lv_group_t *button_group{nullptr};
	void bar_handler_class(lv_event_t *e);
	
  public:
	void init(lv_obj_t *parent, int mode, lv_coord_t w, lv_coord_t h);
	void hide_buttonbar(bool enable);
	void reset_buttonbar();
	static constexpr auto buttons_handler_cb = EventHandler<ButtonBar, &ButtonBar::bar_handler_class>::staticHandler;
	
};

extern ButtonBar buttonbar;