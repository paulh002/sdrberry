#pragma once
#include "AudioInput.h"
#include "lvgl_.h"
#include "Settings.h"
#include "sdrberry.h"
#include "audiobar.h"
#include "SMeterFilter.h"

class gui_recorder
{
  private:
	lv_style_t style_btn;
	lv_obj_t *buttons[10];
	lv_group_t *button_group{nullptr};
	audiobar audiobar_recording;
	std::unique_ptr<SMeterFilter> smeter_filter;
	
	void button_handler_class(lv_event_t *e);
	void event_handler_txbuttons_class(lv_event_t *e);
	
  public:
	void init(lv_obj_t *parent, lv_coord_t w);
	void set_value(float value);

	static constexpr auto button_handler = EventHandler<gui_recorder, &gui_recorder::button_handler_class>::staticHandler;
	static constexpr auto event_handler_txbuttons = EventHandler<gui_recorder, &gui_recorder::event_handler_txbuttons_class>::staticHandler;
};

extern gui_recorder grecorder;