#pragma once
#include "AudioInput.h"
#include "lvgl_.h"
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
  private:
	lv_obj_t *table, *table_label, *qsoTable, *qsoLabel, *cqTable;
	lv_style_t style_btn, ft8_style;
	std::vector<message> messages, qsoMessages;
	int  qsoRowCount{0}, cqRowCount{0};
	int tableviewsize;
	void Scroll(lv_obj_t *table, lv_coord_t currScrollPos);
	std::string call;

	std::string getcall() { return call; }
	void cpy_qso(int row);
	void cpy_cq(int row);
	
	void add_qso(struct message msg);
	void add_cq(struct message msg);
	void QsoScrollLatestItem();
	void QsoScrollFirstItem();
	void ScrollLatestItem();
	void ScrollFirstItem();
	void CqScrollFirstItem();
	void CqScrollLatestItem();

	void qso_press_part_event_class(lv_event_t *e);
	void press_part_event_class(lv_event_t *e);
	void qso_draw_part_event_class(lv_event_t *e);
	void draw_part_event_class(lv_event_t *e);
	void cq_press_part_event_class(lv_event_t *e);
	
  public:
	void init(lv_obj_t *o_tab, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h);
	void add_line(int hh, int min, int sec, int snr, int correct_bits, double off, int hz0, std::string msg);
	void set_group();
	void reset();
	void clr_qso();
	void clr_cq();
	void cpy_cqtoqso(int row);
	int getQsoLogRows();
	std::string getQso(int row);
	void tableScrollLastItem() { ScrollLatestItem(); }

	static constexpr auto cq_press_part_event_cb = EventHandler<gui_ft8, &gui_ft8::cq_press_part_event_class>::staticHandler;
	static constexpr auto qso_press_part_event_cb = EventHandler<gui_ft8, &gui_ft8::qso_press_part_event_class>::staticHandler;
	static constexpr auto press_part_event_cb = EventHandler<gui_ft8, &gui_ft8::press_part_event_class>::staticHandler;
	static constexpr auto qso_draw_part_event_cb = EventHandler<gui_ft8, &gui_ft8::qso_draw_part_event_class>::staticHandler;
	static constexpr auto draw_part_event_cb = EventHandler<gui_ft8, &gui_ft8::draw_part_event_class>::staticHandler;
	
};

extern gui_ft8 gft8;