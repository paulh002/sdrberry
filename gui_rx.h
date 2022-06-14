#pragma once
#include "AudioInput.h"
#include "lvgl/lvgl.h"
#include "Settings.h"
#include "sdrberry.h"
#include "vfo.h"
#include "gui_vfo.h"

class gui_rx
{
  public:
	void init(lv_obj_t *o_tab, lv_coord_t w);
	vector<pair<lv_obj_t *, long long>> get_buttons();
	void set_freq(lv_obj_t * obj, long long freq);
	void get_buttons(vector<long> &array);
	int get_noise();
	
  private:
	lv_group_t *m_button_group{nullptr};
	lv_style_t style_btn;
	vector<pair<lv_obj_t *, long long>> buttons;
	const int number_of_buttons{5};
	lv_obj_t *drp_noise;
};

extern gui_rx guirx;