#pragma once
#include <string>
#include <vector>
#include "lvgl_.h"


class guiButtonWindows
{
  private:
	lv_obj_t *buttonWindowObj{nullptr};
	lv_obj_t *Parent;
	lv_style_t style_btn;
	lv_group_t *button_group{nullptr};
	lv_obj_t *button[20]{nullptr};
	int ibuttons{0};
	void *thisPtr;
	void buttonWindowObj_event_handler_class(lv_event_t *e);
	void buttons_handler_class(lv_event_t *e);
	void createButtons(std::vector<std::string> buttons);

  public:
	guiButtonWindows(lv_obj_t *parent, void *thisptr, std::string name, std::vector<std::string> buttons, int width, int height);
	~guiButtonWindows();

	static constexpr auto buttonWindowObj_event_handler = EventHandler<guiButtonWindows, &guiButtonWindows::buttonWindowObj_event_handler_class>::staticHandler;
	static constexpr auto buttons_handler = EventHandler<guiButtonWindows, &guiButtonWindows::buttons_handler_class>::staticHandler;
	
};

