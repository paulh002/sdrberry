#pragma once
#include "lvgl.h"
#include "Settings.h"
#include "FT8Generator.h"
#include "AMModulator.h"
#include "Waterfall.h"

class gui_ft8bar
{
  public:
	gui_ft8bar();
	~gui_ft8bar();
	void init(lv_obj_t *o_parent, lv_group_t *button_group, lv_group_t *keyboard_group, int mode, lv_coord_t w, lv_coord_t h);
	void hide(bool hide);
	int getbuttons() { return number_of_buttons; }
	void setmonitor(bool mon);
	void setMessage(std::string callsign, int db, int row = 1);
	void SetFrequency();
	void Transmit(lv_obj_t * obj);
	void SetTxMessage(std::string msg = "");
	void SetFilter(std::string msg);
	std::string GetFilter();
	std::string GetCall() { return call; }
	void ClearTransmit();
	void SetFilterCall();
	void ClearMessage();
	void Process(const IQSampleVector &input);
	void DrawWaterfall();

  private:
	lv_obj_t *barview, *frequence, *table, *Textfield,*FilterField;
	lv_obj_t *tx_slider_label, *tx_slider;
	lv_style_t style_btn, ft8_style;
	lv_group_t *buttonGroup{}, *keyboardgroup;
	int txbutton, rxbutton;
	const int number_of_buttons{6};
	lv_obj_t *button[20]{nullptr};
	int ibuttons{0};
	std::string call;
	std::string locator;
	std::string cq;
	std::string encodedMessage;
	atomic<bool> transmitting{false};
	unique_ptr<FT8Generator> ft8generator;
	std::unique_ptr<Waterfall> waterfall;
	float ft8_rate{12000.0};
};

extern gui_ft8bar guift8bar;