#pragma once
#include "lvgl_.h"
#include "sdrberry.h"

class gui_gain
{
  private:
	lv_style_t style_btn, style_tile, settings_style;
	lv_group_t *button_group{nullptr};
	std::vector<lv_obj_t *> gain_sliders;
	std::vector<lv_obj_t *> gain_labels;
	lv_obj_t *pageobj;
	int width, channel;
	std::vector<std::string> rxgains;
	std::vector<SoapySDR::Range> rxRanges;
	bool visible;

	void gain_slider_event_cb_class(lv_event_t *e);

  public:
	void init(lv_obj_t *o_tab, lv_coord_t w, lv_coord_t h);
	void reset_gains();
	void set_gains();
	int gains_count();
	
	static constexpr auto gain_slider_event_cb = EventHandler<gui_gain, &gui_gain::gain_slider_event_cb_class>::staticHandler;
};

extern gui_gain guigain;