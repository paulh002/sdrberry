#pragma once
#include "lvgl_.h"
#include <atomic>

class gui_squelch
{
  private:
	lv_style_t style_btn;
	lv_obj_t *attack_release_slider_label, *attack_release_slider;
	lv_obj_t *threshold_slider_label, *threshold_slider;
	lv_obj_t *button[10];
	int ibuttons{0};
	const int number_of_buttons{4};
	const int top_y{10};
	std::atomic<int> squelch_mode{0};
	lv_group_t *m_button_group{nullptr};
	int buttonsdragc;

	std::atomic<int> attack_release;
	std::atomic<int> threshold;
	
	public:
	  void init(lv_obj_t *o_tab, lv_obj_t *tabbuttons, lv_coord_t w);

	  void threshold_slider_event_cb_class(lv_event_t *e);
	  void attack_release_slider_event_cb_class(lv_event_t *e);
	  void button_handler_class(lv_event_t *e);
	  inline int get_squelch_mode() { return squelch_mode; }
	  void set_attack_release_slider(int attack_release);
	  void set_threshold_slider(int _threshold);
	  int get_threshold() { return threshold.load(); }
	  int get_attack_release() { return attack_release.load(); }
	  int get_mode() { return squelch_mode.load(); }
	  void set_group();
	  void set_sdr_state();

	  static constexpr auto button_handler = EventHandler<gui_squelch, &gui_squelch::button_handler_class>::staticHandler;
	  static constexpr auto threshold_slider_event_cb = EventHandler<gui_squelch, &gui_squelch::threshold_slider_event_cb_class>::staticHandler;
	  static constexpr auto attack_release_slider_event_cb = EventHandler<gui_squelch, &gui_squelch::attack_release_slider_event_cb_class>::staticHandler;
};

extern gui_squelch guisquelch;
