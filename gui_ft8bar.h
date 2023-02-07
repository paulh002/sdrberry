#pragma once
#include "lvgl.h"
#include "Settings.h"
#include "FT8Generator.h"
#include "AMModulator.h"

class gui_ft8bar
{
  public:
	gui_ft8bar();
	~gui_ft8bar();
	void init(lv_obj_t *o_parent, lv_group_t *button_group, lv_group_t *keyboard_group, int mode, lv_coord_t w, lv_coord_t h);
	void hide(bool hide);
	int getbuttons() { return number_of_buttons; }
	lv_obj_t *getspintx() { return button[number_of_buttons]; }
	lv_obj_t *getspinrx() { return button[number_of_buttons+3]; }
	void setmonitor(bool mon);
	void setMessage(std::string callsign, int db);
	void SetFrequency();
	void Transmit();
	void SetTxMessage(std::string msg = "");
	void SetFilter(std::string msg);
	std::string GetFilter();
	std::string GetCall() { return call; }
	void ClearTransmit();
	void SetFilterCall();
	void ClearMessage();

  private:
	lv_obj_t *barview, *frequence, *table, *Textfield,*FilterField;
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
	ModulatorParameters param;
};

extern gui_ft8bar guift8bar;