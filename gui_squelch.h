#pragma once
#include "lvgl_.h"
#include <atomic>

class gui_squelch
{
  private:
	lv_style_t style_btn;
	lv_obj_t *bandwidth_slider_label, *bandwidth_slider;
	lv_obj_t *threshold_slider_label, *threshold_slider;
	lv_obj_t *button[10];
	int ibuttons{0};
	const int number_of_buttons{3};
	const int top_y{10};
	std::atomic<int> squelch_mode{0};
	lv_group_t *m_button_group{nullptr};

	std::atomic<int> bandwidth;
	std::atomic<int> threshold;
	
	public:
	  void init(lv_obj_t *o_tab, lv_obj_t *tabbuttons, lv_coord_t w);

	  void threshold_slider_event_cb_class(lv_event_t *e);
	  void bandwidth_slider_event_cb_class(lv_event_t *e);
	  void button_handler_class(lv_event_t *e);
	  inline int get_squelch_mode() { return squelch_mode; }
	  void set_bandwidth_slider(int bandwidth);
	  void set_threshold_slider(int _threshold);
	  int get_threshold() { return threshold.load(); }
	  int get_bandwidth() { return bandwidth.load(); }
	  int get_mode() { return squelch_mode.load(); }
	  void set_group();

	  static constexpr auto button_handler = EventHandler<gui_squelch, &gui_squelch::button_handler_class>::staticHandler;
	  static constexpr auto threshold_slider_event_cb = EventHandler<gui_squelch, &gui_squelch::threshold_slider_event_cb_class>::staticHandler;
	  static constexpr auto bandwidth_slider_event_cb = EventHandler<gui_squelch, &gui_squelch::bandwidth_slider_event_cb_class>::staticHandler;
};

extern gui_squelch guisquelch;
