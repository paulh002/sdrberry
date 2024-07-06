#pragma once
#include <string>
#include <array>
#include <vector>
#include "lvgl_.h"

enum CustomButtonWindowsEvents
{
	LV_EVENT_CUSTOM,
	LV_EVENT_ATT_CLICKED,
	LV_EVENT_MODE_CLICKED,
	LV_EVENT_PREAMP_CLICKED
};

constexpr int lv_custom_events = sizeof(CustomButtonWindowsEvents);

class guiButtonWindows
{
  private:
	lv_obj_t *buttonWindowObj{nullptr};
	lv_obj_t *Parent;
	lv_style_t style_btn;
	lv_group_t *button_group{nullptr};
	lv_obj_t *button[20]{nullptr};
	int ibuttons{0};
	lv_event_code_t event;
	void *thisPtr;
	void buttonWindowObj_event_handler_class(lv_event_t *e);
	void buttons_handler_class(lv_event_t *e);
	void createButtons(std::vector<std::string> buttons);
	static bool initialized;
	static std::array<uint32_t, lv_custom_events> custom_event;
	void InitEvents();

  public:
	guiButtonWindows(lv_obj_t *parent, void *thisptr, std::string name, std::vector<std::string> buttons, lv_event_code_t eventIndex, int width, int height);
	~guiButtonWindows();
	
	static lv_event_code_t getCustomEvent(int index) { return (lv_event_code_t)custom_event[index]; }
	static constexpr auto buttonWindowObj_event_handler = EventHandler<guiButtonWindows, &guiButtonWindows::buttonWindowObj_event_handler_class>::staticHandler;
	static constexpr auto buttons_handler = EventHandler<guiButtonWindows, &guiButtonWindows::buttons_handler_class>::staticHandler;
	
};

