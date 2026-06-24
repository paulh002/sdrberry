#pragma once
#include "AudioInput.h"
#include "lvgl_.h"
#include "tempMeter.h"
#include "tempSensor.h"

class gui_tx
{
  private:
	lv_obj_t *mic_slider_label, *mic_slider, *drv_slider, *drv_slider_label, *drp_samplerate;
	lv_obj_t *digital_slider, *digital_slider_label, *tune_slider, *tune_slider_label, *playback_slider, *playback_slider_label;
	lv_obj_t *tileview, *speech_tile, *tx_tile, *recorder_tile;
	lv_obj_t *temp_meter;

	tempMeter tempmeter;
	std::unique_ptr<tempSensor> tempsensor;
	std::map<std::string, int> mic_gain_map;
	std::map<std::string, int> digital_gain_map;
	std::map<std::string, int> tune_map;
	std::map<std::string, int> drive_map;
	std::map<std::string, int> playback_map;

	lv_style_t style_btn;
	lv_obj_t *tx_button[10];
	std::vector<int> sample_rates;
	int ibuttons;
	int split_button;
	const int number_of_buttons{5};
	lv_group_t *m_button_group{nullptr};
	bool disabled;
	int old_mode;

	lv_obj_t *get_button_obj(int i);
	void mic_slider_event_cb_class(lv_event_t *e);
	void tx_button_handler_class(lv_event_t *e);
	void drv_slider_event_cb_class(lv_event_t *e);
	void digital_slider_event_cb_class(lv_event_t *e);
	void tune_slider_event_cb_class(lv_event_t *e);
	void playback_slider_event_cb_class(lv_event_t *e);

  public:
	void gui_tx_init(lv_obj_t *o_tab, lv_coord_t w, bool disable);
	void set_mic_slider(int volume);
	void set_tune_slider(int volume);
	void set_digital_slider(int volume);
	void set_drv_slider(int drive);
	void set_drv_range();
	void set_playback_slider(int volume);
	void add_sample_rate(int samplerate);
	void set_sample_rate(int rate);
	void clear_sample_rate();
	void set_group();
	int get_drv_pos();
	void set_split(bool _split);
	void enable_tx(bool enable);
	void get_measurements();
	bool get_split();
	void set_gain_sliders_band_from_config();

	static constexpr auto mic_slider_event_cb = EventHandler<gui_tx, &gui_tx::mic_slider_event_cb_class>::staticHandler;
	static constexpr auto tx_button_handler = EventHandler<gui_tx, &gui_tx::tx_button_handler_class>::staticHandler;
	static constexpr auto drv_slider_event_cb = EventHandler<gui_tx, &gui_tx::drv_slider_event_cb_class>::staticHandler;
	static constexpr auto digital_slider_event_cb = EventHandler<gui_tx, &gui_tx::digital_slider_event_cb_class>::staticHandler;
	static constexpr auto tune_slider_event_cb = EventHandler<gui_tx, &gui_tx::tune_slider_event_cb_class>::staticHandler;
	static constexpr auto playback_slider_event_cb = EventHandler<gui_tx, &gui_tx::playback_slider_event_cb_class>::staticHandler;
};

extern gui_tx	Gui_tx;