#pragma once
#include "lvgl.h"

class GuiFt8Setting
{
  public:
	void init(lv_obj_t *o_tab, lv_group_t *keyboard_group);
	void set_group();

  private:
	lv_obj_t *Textfield;
};

extern GuiFt8Setting guift8setting;