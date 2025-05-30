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
	lv_group_t* button_group{ nullptr };
	lv_style_t style_btn;
	vector<pair<lv_obj_t*, long long>> buttons;
	const int number_of_buttons{ 5 };
	lv_obj_t* drp_noise;
	lv_obj_t *check_cw, *waterfall_hold;
	lv_obj_t *noise_slider, *waterfall_slider;
	lv_obj_t *signal_strength_offset_slider_label, *signal_strength_offset_slider;
	lv_obj_t *noise_slider_label, *waterfall_slider_label, *waterfallsize_slider, *waterfallsize_slider_label;
	int waterfallgain, waterfallsize, spectrumgain;
	lv_obj_t *spectrum_slider_label, *spectrum_slider, *smeter_delay_slider_label, *smeter_delay_slider;
	lv_obj_t *tileview, *main_tile, *settings_tile, *gain_tile;

	void get_buttons(vector<long>& array);
	vector<pair<lv_obj_t*, long long>> get_buttons();
	void rx_button_handler_class(lv_event_t* e);
	void event_handler_morse_class(lv_event_t* e);
	void noise_handler_class(lv_event_t* e);
	void noise_slider_event_cb_class(lv_event_t *e);
	void waterfall_slider_event_cb_class(lv_event_t *e);
	void event_handler_hold_class(lv_event_t *e);
	void waterfallsize_slider_event_class(lv_event_t *e);
	void signal_strength_offset_event_class(lv_event_t *e);
	void spectrum_slider_event_cb_class(lv_event_t *e);
	void smeter_delay_event_cb_class(lv_event_t *e);

  public:
	void init(lv_obj_t *o_tab, lv_coord_t w);	
	void set_freq(lv_obj_t * obj, long long freq);
	bool get_cw();
	void set_cw(bool bcw);
	void toggle_cw();
	int get_noise();
	int get_waterfallgain() { return waterfallgain; }
	int get_spectrumgain() { return spectrumgain; }

	static constexpr auto rx_button_handler = EventHandler<gui_rx, &gui_rx::rx_button_handler_class>::staticHandler;
	static constexpr auto event_handler_morse = EventHandler<gui_rx, &gui_rx::event_handler_morse_class>::staticHandler;
	static constexpr auto noise_handler = EventHandler<gui_rx, &gui_rx::noise_handler_class>::staticHandler;
	static constexpr auto noise_slider_event_cb = EventHandler<gui_rx, &gui_rx::noise_slider_event_cb_class>::staticHandler;
	static constexpr auto waterfall_slider_event_cb = EventHandler<gui_rx, &gui_rx::waterfall_slider_event_cb_class>::staticHandler;
	static constexpr auto event_handler_hold = EventHandler<gui_rx, &gui_rx::event_handler_hold_class>::staticHandler;
	static constexpr auto waterfallsize_slider_event_cb = EventHandler<gui_rx, &gui_rx::waterfallsize_slider_event_class>::staticHandler;
	static constexpr auto signal_strength_offset_event_cb = EventHandler<gui_rx, &gui_rx::signal_strength_offset_event_class>::staticHandler;
	static constexpr auto spectrum_slider_event_cb = EventHandler<gui_rx, &gui_rx::spectrum_slider_event_cb_class>::staticHandler;
	static constexpr auto smeter_delay_event_cb = EventHandler<gui_rx, &gui_rx::smeter_delay_event_cb_class>::staticHandler;
	
};

extern gui_rx guirx;