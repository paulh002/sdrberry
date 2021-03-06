#pragma once
#include "AudioInput.h"
#include "lvgl/lvgl.h"
#include "Settings.h"
#include "sdrberry.h"

struct message
{
	int hh;
	int min;
	int sec;
	int snr;
	int correct_bits;
	double off;
	double hz0;
	std::string msg;
};

class gui_ft8
{
  public:
	void init(lv_obj_t *o_tab, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h);
	void add_line(int hh, int min, int sec, int snr, int correct_bits, double off, double hz0, std::string msg);
	void clear();

  private:
	lv_obj_t *table, *table_label;
	lv_style_t style_btn, ft8_style;
	std::vector<message> messages;
	int m_cycle_count{0};
	bool bclear{false};
};

extern gui_ft8 gft8;