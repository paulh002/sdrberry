#pragma once
#include <atomic>
#include "lvgl.h"
#include "Settings.h"

class gui_cal
{
	void init(lv_obj_t *o_tab, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h);

  public:
	void init(lv_obj_t *o_parent, lv_group_t *button_group, lv_group_t *keyboard_group, lv_coord_t w, lv_coord_t h);
	void hide(bool hide);
	lv_obj_t *get_txgain_slider_label() { return txgainlabel; }
	lv_obj_t *get_txphase_slider_label() { return txphaselabel; }
	lv_obj_t *get_rxgain_slider_label() { return rxgainlabel; }
	lv_obj_t *get_rxphase_slider_label() { return rxphaselabel; }
	float getRxPhase() { return (float)calRxPhase / 1000.0; }
	float getRxGain() { return 1.0 + (float)calRxGain / 500.0; }
	float getTxPhase() { return (float)calTxPhase / 1000.0; }
	float getTxGain() { return 1.0 + (float)calTxGain / 500.0; }

	void setRxPhase(int p) { calRxPhase = p; };
	void setRxGain(int g) { calRxGain = g; };
	void setTxPhase(int p) { calTxPhase = p; };
	void setTxGain(int g) { calTxGain = g; };

  private:
	lv_obj_t *barview, *txgainslider,*txgainlabel, *txphaseslider, *txphaselabel;
	lv_obj_t *rxgainslider, *rxgainlabel, *rxphaseslider, *rxphaselabel;
	lv_style_t style_btn;
	lv_group_t *buttonGroup{};
	std::atomic<int> calRxPhase, calRxGain, calTxPhase, calTxGain;
};

extern gui_cal gcal;