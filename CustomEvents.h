#pragma once
#include <cstdint>
#include <vector>
#include "lvgl_.h"

enum CustomEventsDefinitions
{
	LV_EVENT_CUSTOM,
	LV_EVENT_ATT_CLICKED,
	LV_EVENT_MODE_CLICKED,
	LV_EVENT_PREAMP_CLICKED,
	LV_SLIDER_EVENT_CUSTOM,
	LV_EVENT_RIT_VALUE_CHANGED,
	LV_SLIDER_EVENT_CUSTOM_OK,
	LV_EVENT_CUSTOM_LAST
};

class CustomEvents
{
  private:
	static bool initialized;
	static std::vector<uint32_t> custom_event;
	
  public:
	CustomEvents();
	static lv_event_code_t getCustomEvent(int index) { return (lv_event_code_t)custom_event[index]; }
};

