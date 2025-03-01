#pragma once
#include "Settings.h"
#include "gui_vfo.h"
#include "lvgl_.h"
#include "sdrberry.h"
#include "vfo.h"

class gui_sdr
{
  private:
	long span;
	lv_style_t style_btn, style_tile;
	vector<int> sample_rates;
	lv_obj_t *d_samplerate, *d_decimate;
	lv_obj_t *d_receivers;
	lv_obj_t *span_slider_label, *span_slider;
	lv_obj_t *d_bandwitdth, *d_antenna;
	lv_group_t *button_group{nullptr};

	void receivers_button_handler_class(lv_event_t *e);
	void span_slider_event_cb_class(lv_event_t *e);
	void bandwidth_button_handler_class(lv_event_t *e);
	void samplerate_button_handler_class(lv_event_t *e);
	void decimate_button_handler_class(lv_event_t *e);
	void antenna_button_handler_class(lv_event_t *e);
	
  public:
	void init(lv_obj_t *o_tab, lv_coord_t w, lv_coord_t h);
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
	void init_bandwidth();
	void init_antenna();
	int get_bandwidth_sel() { return lv_dropdown_get_selected(d_bandwitdth); }
	int get_current_rx_channel();
	int get_current_tx_channel();
	std::string getAntenna();
	long getBandwidth();
	
	
	static constexpr auto receivers_button_handler = EventHandler<gui_sdr, &gui_sdr::receivers_button_handler_class>::staticHandler;
	static constexpr auto span_slider_event_cb = EventHandler<gui_sdr, &gui_sdr::span_slider_event_cb_class>::staticHandler;
	static constexpr auto bandwidth_button_handler = EventHandler<gui_sdr, &gui_sdr::bandwidth_button_handler_class>::staticHandler;
	static constexpr auto samplerate_button_handler = EventHandler<gui_sdr, &gui_sdr::samplerate_button_handler_class>::staticHandler;
	static constexpr auto decimate_button_handler = EventHandler<gui_sdr, &gui_sdr::decimate_button_handler_class>::staticHandler;
	static constexpr auto antenna_button_handler = EventHandler<gui_sdr, &gui_sdr::antenna_button_handler_class>::staticHandler;
};

extern gui_sdr guisdr;