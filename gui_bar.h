#pragma once

#include "AudioInput.h"
#include "Settings.h"
#include "gui_vfo.h"
#include "lvgl_.h"
#include "sdrberry.h"
#include "vfo.h"
#include "guiButtonWindows.h"
#include "guiSliderWindows.h"
#include "guiListWindows.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

const int maxifgain = 100;

class gui_bar
{
  private:
	lv_obj_t *barview;
	lv_style_t style_btn;
	lv_style_t ifGainStyleKnob, ifGainStyleIndicator;
	lv_obj_t *button[20]{nullptr};
	lv_obj_t *label[20]{nullptr};
	int ibuttons{0};
	int filter;
	const int number_of_buttons = 10;
	
	lv_obj_t *vol_slider, *vol_slider_label, *gain_slider, *gain_slider_label;
	lv_obj_t *if_slider_label, *if_slider;
	const int max_volume{100};
	vector<int> ifilters;
	lv_obj_t *cw_wpm, *cw_message, *cw_box, *cw_led;
	lv_style_t cw_style, style_selected_color;
	lv_group_t *buttongroup{nullptr};
	bool ifStyleState{false};
	std::unique_ptr<guiButtonWindows> attenuatorWindow, preampWindow, modeWindow, MarkerWindow;
	std::unique_ptr<guiSliderWindows> ritWindow;
	std::unique_ptr<guiListWindows> stepsWindow;
	int rit_value;
	int steps_value{1};
	std::atomic<float> ifgain;
	std::map<std::string, int> rf_gain;
	std::map<std::string, int> filter_to_mode_cutoff_frequencies;

	void bar_button_handler_class(lv_event_t *e);
	void gain_slider_event_class(lv_event_t *e);
	void if_slider_event_class(lv_event_t *e);
	void vol_slider_event_class(lv_event_t *e);
	void filter_slider_event_class(lv_event_t *e);
	

  public:
	gui_bar();
	~gui_bar();

	static constexpr auto bar_button_handler = EventHandler<gui_bar, &gui_bar::bar_button_handler_class>::staticHandler;
	static constexpr auto gain_slider_event_cb = EventHandler<gui_bar, &gui_bar::gain_slider_event_class>::staticHandler;
	static constexpr auto if_slider_event_cb = EventHandler<gui_bar, &gui_bar::if_slider_event_class>::staticHandler;
	static constexpr auto vol_slider_event_cb = EventHandler<gui_bar, &gui_bar::vol_slider_event_class>::staticHandler;
	static constexpr auto filter_slider_event_cb = EventHandler<gui_bar, &gui_bar::filter_slider_event_class>::staticHandler;

	void updateweb();
	void init(lv_obj_t *o_parent, lv_group_t *button_group, int mode, lv_coord_t w, lv_coord_t h);
	void set_vol_slider(int volume, bool web = true);
	void set_focus();
	void set_filter_dropdown(int filter);
	void set_filter_number(int filter);
	void step_vol_slider(int step);
	void step_gain_slider(int step);
	void select_option(int option);
	lv_obj_t *get_vol_slider_label() { return vol_slider_label; }
	lv_obj_t *get_gain_slider_label() { return gain_slider_label; }
	lv_obj_t *get_if_slider_label() { return if_slider_label; }
	int get_vol_range();
	float get_if();
	int get_if_slider();
	void set_if(int rf, bool web = true);
	void set_gain_range();
	void set_gain_slider(int gain, bool web = true);
	int getbuttons() { return ibuttons; }
	void set_mode(int mode);
	void set_cw_message(std::string message);
	void set_cw_wpm(int wpm);
	void hide_cw(bool hide);
	void set_led(bool status);
	void set_tx(bool tx);
	int get_rf_gain() { return lv_slider_get_value(gain_slider); }
	int get_volume() { return lv_slider_get_value(vol_slider); }
	void setIfGainOverflow(bool state);
	void hide(bool hide);
	void hidetx();
	void set_vfo(int active_vfo);
	bool get_noise();
	void web_filterfreq();
	void websetfilter(std::string message);
	void setTxButtons();
	int get_step_value();
	void change_step(int i);
	void update_step_button(int step);
	void get_gain_range(int &max_gain, int &min_gain);
	void set_gain_slider_band_from_config(bool web = true);
	int get_filter_frequency(int mode);

	lv_obj_t *get_button_obj(int i)
	{
		if (i >= ibuttons)
			return nullptr;
		return button[i];
	}
	void get_filter_range(vector<string> &filters);
};

extern gui_bar gbar;