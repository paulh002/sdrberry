#pragma once
#include "lvgl.h"

const uint32_t MSG_TEXTMESSAGE = 1;

class VfoKeyPad
{
  public:
	VfoKeyPad(lv_obj_t *parent);
	~VfoKeyPad();
	lv_obj_t *getWin() { return win; }

  private:
	lv_obj_t *win, *closeBtn;
};

extern void CreateVfoKeyPadWindow(lv_obj_t *parent);