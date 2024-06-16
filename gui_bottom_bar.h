#pragma once
#include "lvgl_.h"
#include "sdrberry.h"

extern const int screenWidth;
extern const int screenHeight;
extern const int bottomHeight;
extern const int topHeight;
extern const int tunerHeight;

class gui_bottom_bar
{
  private:
	lv_obj_t *barview;
	lv_style_t style_btn;
	lv_obj_t *button[20]{nullptr};
	lv_group_t *buttongroup{nullptr};
	const int number_of_buttons{4};
	int ibuttons{0};

	void bar_button_handler_class(lv_event_t *e);
	
  public:
	void init(lv_obj_t *o_parent, lv_group_t *button_group, int mode, lv_coord_t w, lv_coord_t h);
	static constexpr auto bottonbar_button_handler = EventHandler<gui_bottom_bar, &gui_bottom_bar::bar_button_handler_class>::staticHandler;
};

extern gui_bottom_bar gbottombar;

