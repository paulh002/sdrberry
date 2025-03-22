#pragma once
#include "lvgl_.h"

const uint32_t MSG_TEXTMESSAGE = 1;

class VfoKeyPad
{
  private:
	lv_obj_t *win, *closeBtn;
	void textarea_event_handler_class(lv_event_t *e);

  public: 
	VfoKeyPad(lv_obj_t *parent, lv_group_t *keyboard_group);
	~VfoKeyPad();
	lv_obj_t *getWin() { return win; }

	static constexpr auto textarea_event_handler = EventHandler<VfoKeyPad, &VfoKeyPad::textarea_event_handler_class>::staticHandler;
};

extern void CreateVfoKeyPadWindow(lv_obj_t *parent, lv_group_t *keyboard_group);