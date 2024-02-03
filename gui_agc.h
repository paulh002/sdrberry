#pragma once
#include "lvgl_.h"
#include <atomic>

extern const int screenWidth;
extern const int screenHeight;
extern const int bottomHeight;
extern const int topHeight;
extern const int tunerHeight;

class Gui_agc
{
  private:
	lv_style_t style_btn;
	lv_obj_t *ratio_slider_label, *ratio_slider;
	lv_obj_t *threshold_slider_label, *threshold_slider;
	lv_obj_t *atack_slider_label, *atack_slider;
	lv_obj_t *release_slider_label, *release_slider;
	lv_obj_t *button[10];
	int ibuttons{0};
	const int number_of_buttons{4};
	const int top_y{10};
	std::atomic<int> agc_mode{0};
	lv_group_t *m_button_group{nullptr};

	std::atomic<int> atack;
	std::atomic<int> release;
	std::atomic<int> ratio;
	std::atomic<int> threshold;

	void agc_button_handler_class(lv_event_t *e);
	void threshold_slider_event_cb_class(lv_event_t *e);
	void atack_slider_event_cb_class(lv_event_t *e);
	void release_slider_event_cb_class(lv_event_t *e);
	void ratio_slider_event_cb_class(lv_event_t * e);

  public:
	void init(lv_obj_t *o_tab, lv_coord_t w);
	void set_ratio_range();
	void set_ratio_range(int min, int max);
	void set_ratio_slider(int ratio);
	void set_group();
	void set_agc_mode(int m);

	float get_atack();
	float get_release();
	float get_ratio();
	int get_threshold();
	inline int get_agc_mode() {return agc_mode;}

	void set_atack_slider(int t);
	void set_threshold_slider(int t);
	void set_release_slider(int t);


	static constexpr auto agc_button_handler = EventHandler<Gui_agc, &Gui_agc::agc_button_handler_class>::staticHandler;
	static constexpr auto threshold_slider_event_cb = EventHandler<Gui_agc, &Gui_agc::threshold_slider_event_cb_class>::staticHandler;
	static constexpr auto atack_slider_event_cb = EventHandler<Gui_agc, &Gui_agc::atack_slider_event_cb_class>::staticHandler;
	static constexpr auto release_slider_event_cb = EventHandler<Gui_agc, &Gui_agc::release_slider_event_cb_class>::staticHandler;
	static constexpr auto ratio_slider_event_cb = EventHandler<Gui_agc, &Gui_agc::ratio_slider_event_cb_class>::staticHandler;

	//const int	max_threshold {200};
};

extern Gui_agc gagc;