#pragma once
#include "lvgl_.h"
#include "meter.h"
#include "sma.h"
#include <memory>
#include "SMeterFilter.h"

class gui_vfo
{
  private:
	lv_style_t tuner_style;
	lv_obj_t *vfo1_frequency;
	lv_obj_t *vfo2_frequency;
	lv_obj_t *bg_tuner1;
	lv_obj_t *bg_tuner2;
	lv_obj_t *bg_smeter;
	lv_obj_t *band_label, *band_label2;
	lv_obj_t *mode_label, *mode_label2;
	lv_obj_t *rxtx_label, *rxtx_label2;
	lv_obj_t *span_label, *span_label2;
	lv_obj_t *cw_led;
	lv_obj_t *mode_split2;

	lv_group_t *keyboardgroup;
	bool rxtx{true};
	bool split{false};
	int mode[2];
	std::unique_ptr<SMeterFilter> smeter_filter;
	int smeter_delay;
	meter smeter;

	void bg_tuner1_clickevent_cb_class(lv_event_t *e);

  public:
	void gui_vfo_init(lv_obj_t *scr, int x, int y, int w, int h, lv_group_t *keyboard_group);
	void set_vfo_gui(int vfo, long long freq, int vfo_rx, int vfo_mode_no, int vfo_band, int vfo_band_index);
	void set_span(int span);
	void set_split(bool _split);
	bool get_split();
	void set_s_meter(float value);
	void set_smeter_delay(int delay) { smeter_delay = delay; }
	// static constexpr auto smeter_event_cb = EventHandler<gui_vfo, &gui_vfo::smeter_event_cb_class>::staticHandler;
	static constexpr auto bg_tuner1_clickevent_cb = EventHandler<gui_vfo, &gui_vfo::bg_tuner1_clickevent_cb_class>::staticHandler;
};

extern gui_vfo gui_vfo_inst;
