#pragma once
#include <string>
#include <array>
#include <vector>
#include "lvgl_.h"

enum CustomSliderWindowsEvents
{
	LV_SLIDER_EVENT_CUSTOM,
	LV_EVENT_RIT_VALUE_CHANGED
};

constexpr int lv_custom_slider_events = sizeof(CustomSliderWindowsEvents);

class guiSliderWindows
{
  private:
	lv_obj_t *buttonWindowObj{nullptr};
	lv_obj_t *Parent;
	lv_style_t style_btn;
	lv_group_t *button_group{nullptr};
	lv_obj_t *sliders[20]{nullptr};
	lv_event_code_t event;
	void *thisPtr;
	
	void btnWindowObj_event_handler_class(lv_event_t *e);
	void slider_handler_class(lv_event_t *e);
	
	static bool initialized;
	static std::array<uint32_t, lv_custom_slider_events> custom_event;
	void InitEvents();
	
	public:
	  guiSliderWindows(lv_obj_t *parent, void *thisptr, std::string name, std::vector<std::string> buttons, lv_event_code_t eventIndex, int width, int height);
	  ~guiSliderWindows();

	  static lv_event_code_t getCustomEvent(int index) { return (lv_event_code_t)custom_event[index]; }
	  static constexpr auto btnWindowObj_event_handler = EventHandler<guiSliderWindows, &guiSliderWindows::btnWindowObj_event_handler_class>::staticHandler;
	  static constexpr auto slider_handler = EventHandler<guiSliderWindows, &guiSliderWindows::slider_handler_class>::staticHandler;
};

