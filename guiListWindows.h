#pragma once
#include <string>
#include <array>
#include <vector>
#include "lvgl_.h"

class guiListWindows
{
  private:
	lv_obj_t *listWindowObj{nullptr};
	lv_obj_t *Parent;
	lv_style_t style_btn;
	lv_group_t *button_group{nullptr};
	lv_obj_t *sliders[20]{nullptr};
	lv_event_code_t event;
	lv_obj_t *ui_Image1, *ui_Arc1, *label;
	lv_obj_t *list;
	void *thisPtr;
	int row_selected{-1};

	void btnWindowObj_event_handler_class(lv_event_t *e);
	void btnokWindowObj_event_handler_class(lv_event_t *e);
	void draw_part_event_class(lv_event_t *e);
	void list_handler_class(lv_event_t *e);

  public:
	guiListWindows(lv_obj_t *parent, void *thisptr, std::string name, std::vector<std::string> buttons, int val, lv_event_code_t eventIndex, int width, int height);
	~guiListWindows();

	static constexpr auto btnWindowObj_event_handler = EventHandler<guiListWindows, &guiListWindows::btnWindowObj_event_handler_class>::staticHandler;
	static constexpr auto btnokWindowObj_event_handler = EventHandler<guiListWindows, &guiListWindows::btnokWindowObj_event_handler_class>::staticHandler;
	static constexpr auto list_handler = EventHandler<guiListWindows, &guiListWindows::list_handler_class>::staticHandler;
	static constexpr auto draw_part_event = EventHandler<guiListWindows, &guiListWindows::draw_part_event_class>::staticHandler;
};

