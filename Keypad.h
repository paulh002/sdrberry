#pragma once
#include <string>
#include "lvgl_.h"


const uint32_t MSG_TEXTMESSAGE = 1;

class KeyPad
{
  private:
	lv_obj_t *win, *closeBtn, *target;
	lv_obj_t *ta;
	int keyboard;
	void textarea_event_handler_class(lv_event_t *e);
	void btnm_event_handler_class(lv_event_t *e);
	long translate_frequency(std::string text);

  public:
	KeyPad(lv_obj_t *parent, lv_obj_t *target_, lv_group_t *keyboard_group, int keyboard);
	lv_obj_t *getWin() { return win; }

	static constexpr auto textarea_event_handler = EventHandler<KeyPad, &KeyPad::textarea_event_handler_class>::staticHandler;
	static constexpr auto btnm_event_handler = EventHandler<KeyPad, &KeyPad::btnm_event_handler_class>::staticHandler;
	
};

extern void CreateKeyPadWindow(lv_obj_t *parent, lv_obj_t *target_, lv_group_t *keyboard_group, int keyboard);