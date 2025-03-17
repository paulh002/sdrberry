#pragma once
#include <memory>
#include "lvgl_.h"
#include "cmeter.h"
#include "sma.h"

class gui_vfo
{
  private:
	lv_style_t text_style, label_style;
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
	lv_meter_scale_c *scale, *scale2;
	lv_meter_indicator_c *smeter_indic;
	std::unique_ptr<cmeter> cmeter_ptr;
	bool rxtx{true};
	bool split{false};
	int mode[2];
	SMA<2> smeter2;
	SMA<4> smeter4;
	SMA<6> smeter6;
	SMA<8> smeter8;
	int smeter_delay;

	void set_smeter_img(lv_obj_t *box, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h);
	void set_smeter_value(int32_t v);
	void smeter_event_cb_class(lv_event_t *e);

  public:
	void gui_vfo_init(lv_obj_t *scr);
	void set_vfo_gui(int vfo, long long freq, int vfo_rx, int vfo_mode_no, int vfo_band);
	void set_span(int span);
	void set_split(bool _split);
	bool get_split();
	void set_s_meter(double value);
	void set_smeter_delay(int delay) { smeter_delay = delay; }
	static constexpr auto smeter_event_cb = EventHandler<gui_vfo, &gui_vfo::smeter_event_cb_class>::staticHandler;
};

extern gui_vfo gui_vfo_inst;


