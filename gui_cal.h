#pragma once
#include <atomic>
#include "lvgl_.h"
#include "Settings.h"

class gui_cal
{
  private:
	lv_obj_t *barview, *txgainslider, *txgainlabel, *txphaseslider, *txphaselabel;
	lv_obj_t *rxgainslider, *rxgainlabel, *rxphaseslider, *rxphaselabel;
	lv_obj_t *errorLabel, *correlationLabel;
	lv_style_t style_btn;
	lv_group_t *buttonGroup{};
	std::atomic<int> calRxPhase, calRxGain, calTxPhase, calTxGain;
	std::vector<int> calRxPhasePerBand, calRxGainPerBand, calTxPhasePerBand, calTxGainPerBand;
	int correctionDividerTx, correctionDividerRx;

	lv_obj_t *get_txgain_slider_label() { return txgainlabel; }
	lv_obj_t *get_txphase_slider_label() { return txphaselabel; }
	lv_obj_t *get_rxgain_slider_label() { return rxgainlabel; }
	lv_obj_t *get_rxphase_slider_label() { return rxphaselabel; }


	void setRxPhase(int p) { calRxPhase = p; };
	void setRxGain(int g) { calRxGain = g; };
	void setTxPhase(int p) { calTxPhase = p; };
	void setTxGain(int g) { calTxGain = g; };
	void SaveCalibrationTxGain();
	void SaveCalibrationTxPhase();
	void SaveCalibrationRxGain();
	void SaveCalibrationRxPhase();
	
	

	void gain_slider_event_cb_class(lv_event_t *e);
	void phase_slider_event_cb_class(lv_event_t *e);
	void rxphase_slider_event_cb_class(lv_event_t *e);
	void rxgain_slider_event_cb_class(lv_event_t *e);

  public:
	void init(lv_obj_t *o_tab, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h);
	void init(lv_obj_t *o_parent, lv_group_t *button_group, lv_group_t *keyboard_group, lv_coord_t w, lv_coord_t h);
	void hide(bool hide);

	float getRxPhase();
	float getRxGain();
	float getTxPhase();
	float getTxGain();
	void SetCalibrationBand(int bandIndex);
	void SetErrorCorrelation(double error, double correlation);
	
	static constexpr auto gain_slider_event_cb = EventHandler<gui_cal, &gui_cal::gain_slider_event_cb_class>::staticHandler;
	static constexpr auto phase_slider_event_cb = EventHandler<gui_cal, &gui_cal::phase_slider_event_cb_class>::staticHandler;
	static constexpr auto rxphase_slider_event_cb = EventHandler<gui_cal, &gui_cal::rxphase_slider_event_cb_class>::staticHandler;
	static constexpr auto rxgain_slider_event_cb = EventHandler<gui_cal, &gui_cal::rxgain_slider_event_cb_class>::staticHandler;
};

extern gui_cal gcal;