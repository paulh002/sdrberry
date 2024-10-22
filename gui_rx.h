#pragma once
#include "AudioInput.h"
#include "lvgl_.h"
#include "Settings.h"
#include "sdrberry.h"
#include "vfo.h"
#include "gui_vfo.h"

class gui_rx
{
private:
	lv_group_t* m_button_group{ nullptr };
	lv_style_t style_btn;
	vector<pair<lv_obj_t*, long long>> buttons;
	const int number_of_buttons{ 5 };
	lv_obj_t* drp_noise;
	lv_obj_t *check_cw, *waterfall_hold;
	lv_obj_t *noise_slider, *waterfall_slider;
	lv_obj_t *noise_slider_label, *waterfall_slider_label;
	int waterfallgain;

	void get_buttons(vector<long>& array);
	vector<pair<lv_obj_t*, long long>> get_buttons();
	void rx_button_handler_class(lv_event_t* e);
	void event_handler_morse_class(lv_event_t* e);
	void noise_handler_class(lv_event_t* e);
	void noise_slider_event_cb_class(lv_event_t *e);
	void waterfall_slider_event_cb_class(lv_event_t *e);
	void event_handler_hold_class(lv_event_t *e);
	
  public:
	void init(lv_obj_t *o_tab, lv_coord_t w);	
	void set_freq(lv_obj_t * obj, long long freq);
	bool get_cw();
	void set_cw(bool bcw);
	void toggle_cw();
	int get_noise();
	int get_waterfallgain() { return waterfallgain; }

	static constexpr auto rx_button_handler = EventHandler<gui_rx, &gui_rx::rx_button_handler_class>::staticHandler;
	static constexpr auto event_handler_morse = EventHandler<gui_rx, &gui_rx::event_handler_morse_class>::staticHandler;
	static constexpr auto noise_handler = EventHandler<gui_rx, &gui_rx::noise_handler_class>::staticHandler;
	static constexpr auto noise_slider_event_cb = EventHandler<gui_rx, &gui_rx::noise_slider_event_cb_class>::staticHandler;
	static constexpr auto waterfall_slider_event_cb = EventHandler<gui_rx, &gui_rx::waterfall_slider_event_cb_class>::staticHandler;
	static constexpr auto event_handler_hold = EventHandler<gui_rx, &gui_rx::event_handler_hold_class>::staticHandler;
	

	
};

extern gui_rx guirx;