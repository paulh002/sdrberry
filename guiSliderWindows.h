#pragma once
#include <string>
#include <array>
#include <vector>
#include "lvgl_.h"

class guiSliderWindows
{
  private:
	lv_obj_t *buttonWindowObj{nullptr};
	lv_obj_t *Parent;
	lv_style_t style_btn;
	lv_group_t *button_group{nullptr};
	lv_obj_t *sliders[20]{nullptr};
	lv_event_code_t event;
	lv_obj_t *ui_Image1, *ui_Arc1, *label;
	void *thisPtr;
	int value;

	void btnWindowObj_event_handler_class(lv_event_t *e);
	void btnokWindowObj_event_handler_class(lv_event_t *e);
	void slider_handler_class(lv_event_t *e);
	void create_rotary_button(lv_obj_t *parent, int dy);

	public:
	  guiSliderWindows(lv_obj_t *parent, void *thisptr, std::string name, std::vector<std::string> buttons,int val, lv_event_code_t eventIndex, int width, int height);
	  ~guiSliderWindows();

	  static constexpr auto btnWindowObj_event_handler = EventHandler<guiSliderWindows, &guiSliderWindows::btnWindowObj_event_handler_class>::staticHandler;
	  static constexpr auto btnokWindowObj_event_handler = EventHandler<guiSliderWindows, &guiSliderWindows::btnokWindowObj_event_handler_class>::staticHandler;
	  static constexpr auto slider_handler = EventHandler<guiSliderWindows, &guiSliderWindows::slider_handler_class>::staticHandler;
};

