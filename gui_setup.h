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
	lv_style_t style_btn, style_tile, text_style;
	lv_obj_t *brightness_slider_label, *brightness_slider, *shutdownbutton;
	lv_obj_t *d_audio, *audio_label;
	lv_group_t *button_group{nullptr};
	lv_obj_t *cal_label, *calibration_dropdown;
	lv_obj_t *calbox, *dcbox, *autocalbox, *webbox;
	lv_obj_t *tileview, *settings_main, *settings_i2c, *settings_i2c_input, *settings_i2c_output;

	void calbox_event_cb_class(lv_event_t *e);
	void brightness_slider_event_cb_class(lv_event_t *e);
	void audio_button_handler_class(lv_event_t *e);
	void dcbox_event_cb_class(lv_event_t *e);
	void cal_button_handler_class(lv_event_t *e);
	void webbox_event_class(lv_event_t *e);
	void shutdown_button_handler_class(lv_event_t *e);
	void do_shutdown_button_handler_class(lv_event_t *e);
	int get_maxbrightness();
	
  public:
	void init(lv_obj_t *o_tab, lv_group_t *keyboard_group, lv_coord_t w, lv_coord_t h);
	void set_brightness(int brightness);
	int get_brightness();
	bool get_calibration() { return (lv_obj_get_state(calbox) & LV_STATE_CHECKED); }

	void set_group();

	static constexpr auto calbox_event_cb = EventHandler<gui_setup, &gui_setup::calbox_event_cb_class>::staticHandler;
	static constexpr auto brightness_slider_event_cb = EventHandler<gui_setup, &gui_setup::brightness_slider_event_cb_class>::staticHandler;
	static constexpr auto audio_button_handler = EventHandler<gui_setup, &gui_setup::audio_button_handler_class>::staticHandler;
	static constexpr auto dcbox_event_cb = EventHandler<gui_setup, &gui_setup::dcbox_event_cb_class>::staticHandler;
	static constexpr auto cal_button_handler = EventHandler<gui_setup, &gui_setup::cal_button_handler_class>::staticHandler;
	static constexpr auto webbox_event_cb = EventHandler<gui_setup, &gui_setup::webbox_event_class>::staticHandler;
	static constexpr auto shutdown_button_handler = EventHandler<gui_setup, &gui_setup::shutdown_button_handler_class>::staticHandler;
	static constexpr auto do_shutdown_button_handler = EventHandler<gui_setup, &gui_setup::do_shutdown_button_handler_class>::staticHandler;
	
};

extern gui_setup gsetup;