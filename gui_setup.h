#pragma once
#include "AudioInput.h"
#include "AudioOutput.h"
#include "Settings.h"
#include "gui_vfo.h"
#include "lvgl_.h"
#include "sdrberry.h"
#include "vfo.h"


class gui_setup
{
  private:
	long span;
	lv_style_t style_btn, style_tile;
	vector<int> sample_rates;
	lv_obj_t *d_samplerate, *d_decimate;
	lv_obj_t *d_receivers;
	lv_obj_t *span_slider_label, *span_slider;
	lv_obj_t *brightness_slider_label, *brightness_slider;
	lv_obj_t *d_audio, *d_bandwitdth;
	lv_group_t *button_group{nullptr};
	lv_obj_t *cal_label, *calibration_dropdown;
	lv_obj_t *calbox, *dcbox, *autocalbox, *webbox;
	lv_obj_t *tileview, *settings_main, *settings_i2c;

	void receivers_button_handler_class(lv_event_t *e);
	void calbox_event_cb_class(lv_event_t *e);
	void span_slider_event_cb_class(lv_event_t *e);
	void brightness_slider_event_cb_class(lv_event_t *e);
	void audio_button_handler_class(lv_event_t *e);
	void bandwidth_button_handler_class(lv_event_t *e);
	void samplerate_button_handler_class(lv_event_t *e);
	void dcbox_event_cb_class(lv_event_t *e);
	void cal_button_handler_class(lv_event_t *e);
	void webbox_event_class(lv_event_t *e);
	void decimate_button_handler_class(lv_event_t *e);
	int get_maxbrightness();

  public:
	void init(lv_obj_t *o_tab, lv_coord_t w, lv_coord_t h, AudioOutput &audioDevice);
	void add_sample_rate(int samplerate);
	void set_sample_rate(int rate);
	void set_samplerate();
	int get_sample_rate(int rate);
	int get_decimation();
	void set_radio(std::string name);
	void clear_sample_rate();
	lv_obj_t *get_span_slider_label() { return span_slider_label; }
	void set_span_range(long span);
	void set_span_value(long span);
	long get_span() { return span; }
	void set_brightness(int brightness);
	int get_brightness();
	bool get_calibration() { return (lv_obj_get_state(calbox) & LV_STATE_CHECKED); }

	void set_group();
	void init_bandwidth();
	int get_bandwidth_sel() { return lv_dropdown_get_selected(d_bandwitdth); }
	int get_current_rx_channel();
	int get_current_tx_channel();

	static constexpr auto receivers_button_handler = EventHandler<gui_setup, &gui_setup::receivers_button_handler_class>::staticHandler;
	static constexpr auto calbox_event_cb = EventHandler<gui_setup, &gui_setup::calbox_event_cb_class>::staticHandler;
	static constexpr auto span_slider_event_cb = EventHandler<gui_setup, &gui_setup::span_slider_event_cb_class>::staticHandler;
	static constexpr auto brightness_slider_event_cb = EventHandler<gui_setup, &gui_setup::brightness_slider_event_cb_class>::staticHandler;
	static constexpr auto audio_button_handler = EventHandler<gui_setup, &gui_setup::audio_button_handler_class>::staticHandler;
	static constexpr auto bandwidth_button_handler = EventHandler<gui_setup, &gui_setup::bandwidth_button_handler_class>::staticHandler;
	static constexpr auto samplerate_button_handler = EventHandler<gui_setup, &gui_setup::samplerate_button_handler_class>::staticHandler;
	static constexpr auto dcbox_event_cb = EventHandler<gui_setup, &gui_setup::dcbox_event_cb_class>::staticHandler;
	static constexpr auto cal_button_handler = EventHandler<gui_setup, &gui_setup::cal_button_handler_class>::staticHandler;
	static constexpr auto webbox_event_cb = EventHandler<gui_setup, &gui_setup::webbox_event_class>::staticHandler;
	static constexpr auto decimate_button_handler = EventHandler<gui_setup, &gui_setup::decimate_button_handler_class>::staticHandler;
};

extern gui_setup gsetup;