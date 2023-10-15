#pragma once
#include "lvgl.h"

const uint32_t MSG_TEXTMESSAGE = 1;

class HexKeyboardWindow
{
  public:
	HexKeyboardWindow(lv_obj_t *parent);
	~HexKeyboardWindow();
	lv_obj_t *getWin() { return win; }

  private:
	lv_obj_t *win, *closeBtn;
};

extern void CreateHexKeyboardWindow(lv_obj_t *parent);