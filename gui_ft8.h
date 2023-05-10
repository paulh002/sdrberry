#pragma once
#include "AudioInput.h"
#include "lvgl.h"
#include "Settings.h"
#include "sdrberry.h"

class message
{
  public:
	int hh;
	int min;
	int sec;
	int snr;
	int correct_bits;
	double off;
	int hz0;
	std::string msg;
};

class gui_ft8
{
  public:
	void init(lv_obj_t *o_tab, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h);
	void add_line(int hh, int min, int sec, int snr, int correct_bits, double off, int hz0, std::string msg);
	void clear();
	void set_group();
	void reset();
	void ScrollLatestItem();
	void ScrollFirstItem();
	void QsoScrollLatestItem();
	void QsoScrollFirstItem();
	void add_qso(struct message msg);
	void cpy_qso(int row);
	void clr_qso();
	std::string getcall() { return call; }
	int getQsoLogRows();
	std::string getQso(int row);

  private:
	lv_obj_t *table, *table_label, *qsoTable, *qsoLabel;
	lv_style_t style_btn, ft8_style;
	std::vector<message> messages, qsoMessages;
	int m_cycle_count{0}, qsoRowCount{0};
	bool bclear{false};
	void Scroll(lv_obj_t *table, lv_coord_t currScrollPos);
	std::string call;
};

extern gui_ft8 gft8;