#pragma once
#include "AudioInput.h"
#include "lvgl_.h"

extern const int screenWidth;
extern const int screenHeight;
extern const int bottomHeight;
extern const int topHeight;
extern const int tunerHeight;

class gui_tx
{
  private:
	lv_obj_t *mic_slider_label, *mic_slider, *drv_slider, *drv_slider_label, *drp_samplerate;
	lv_obj_t *digital_slider, *digital_slider_label;

	lv_style_t style_btn;
	lv_obj_t *tx_button[10];
	vector<int> sample_rates;
	int ibuttons;
	const int number_of_buttons{7};
	lv_group_t *m_button_group{nullptr};

	lv_obj_t *get_button_obj(int i);
	void mic_slider_event_cb_class(lv_event_t *e);
	void tx_button_handler_class(lv_event_t *e);
	void drv_slider_event_cb_class(lv_event_t *e);
	void digital_slider_event_cb_class(lv_event_t *e);
	
  public:
	void gui_tx_init(lv_obj_t *o_tab, lv_coord_t w);
	void set_mic_slider(int volume);
	void set_digital_slider(int volume);
	void set_drv_slider(int drive);
	void set_tx_state(bool state);
	void set_drv_range();
	void add_sample_rate(int samplerate);
	void step_drv_slider(int step);
	void set_sample_rate(int rate);
	void clear_sample_rate();
	void set_group();
	int get_drv_pos();
	void set_split(bool _split);

	static constexpr auto mic_slider_event_cb = EventHandler<gui_tx, &gui_tx::mic_slider_event_cb_class>::staticHandler;
	static constexpr auto tx_button_handler = EventHandler<gui_tx, &gui_tx::tx_button_handler_class>::staticHandler;
	static constexpr auto drv_slider_event_cb = EventHandler<gui_tx, &gui_tx::drv_slider_event_cb_class>::staticHandler;
	static constexpr auto digital_slider_event_cb = EventHandler<gui_tx, &gui_tx::digital_slider_event_cb_class>::staticHandler;
};

extern gui_tx	Gui_tx;