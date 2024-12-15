#pragma once
#include "lvgl_.h"
#include "Settings.h"
#include "AMModulator.h"
#include "Waterfall.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

enum class ft8status_t
{
	idle,
	monitor,
	cq,
	response	
};

class gui_ft8bar
{
  private:
	lv_obj_t *barview, *frequence, *table, *Textfield, *FilterField, *wsjtxmode;
	lv_obj_t *tx_slider_label, *tx_slider, *if_slider, *if_slider_label;
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
	std::unique_ptr<Waterfall> waterfall;
	float ft8_rate{12000.0};
	std::string msgsend;
	int rxtxmode;
	ft8status_t ft8status;

	void ft8bar_button_handler_class(lv_event_t *e);
	void filter_event_handler_class(lv_event_t *e);
	void mode_event_handler_class(lv_event_t *e);
	void textarea_event_handler_class(lv_event_t *e);
	void if_slider_event_cb_class(lv_event_t *e);
	void tx_slider_event_cb_class(lv_event_t *e);
	void freq_event_handler_class(lv_event_t *e);

  public:
	gui_ft8bar();
	~gui_ft8bar();
	void init(lv_obj_t *o_parent, lv_group_t *button_group, lv_group_t *keyboard_group, int mode, lv_coord_t w, lv_coord_t h);
	void hide(bool hide);
	int getbuttons() { return number_of_buttons; }
	void setmonitor(bool mon);
	void setMessage(std::string callsign, int db, int row = 1);
	void SetFrequency();
	int Transmit(lv_obj_t * obj);
	void SetTxMessage(std::string msg = "");
	void SetFilter(std::string msg);
	std::string GetFilter();
	std::string GetCall() { return call; }
	void ClearTransmit();
	void SetFilterCall();
	void ClearMessage();
	void Process(const IQSampleVector &input);
	void WaterfallSetMaxMin(float _max, float _min);
	void WaterfallReset();
	void DrawWaterfall(int noisefloor);
	void set_if(int ifgain);
	lv_obj_t *getfrequency() { return frequence; }
	lv_obj_t *getwsjtxmode() { return wsjtxmode; }
	bool getmonitorstate() { return lv_obj_get_state(button[0]) & LV_STATE_CHECKED; }
	void setmodeclickable(bool clickable);
	void setrxtxmode(int mode) { rxtxmode = mode; }
	int getrxtxmode() { return rxtxmode; }
	ft8status_t get_status() { return ft8status; }
	void web_wsjtxfreq();
	void CQButton();
	void TXButton();
	void ClearButton();
	void MonitorButton();
	void LogButton();
	json get_txmessage();
	void MessageNo(std::string message);
	void get_buttons();
	void set_frequency(json message);
	void Log();

	static constexpr auto ft8bar_button_handler = EventHandler<gui_ft8bar, &gui_ft8bar::ft8bar_button_handler_class>::staticHandler;
	static constexpr auto filter_event_handler = EventHandler<gui_ft8bar, &gui_ft8bar::filter_event_handler_class>::staticHandler;
	static constexpr auto mode_event_handler = EventHandler<gui_ft8bar, &gui_ft8bar::mode_event_handler_class>::staticHandler;
	static constexpr auto textarea_event_handler = EventHandler<gui_ft8bar, &gui_ft8bar::textarea_event_handler_class>::staticHandler;
	static constexpr auto if_slider_event_cb = EventHandler<gui_ft8bar, &gui_ft8bar::if_slider_event_cb_class>::staticHandler;
	static constexpr auto tx_slider_event_cb = EventHandler<gui_ft8bar, &gui_ft8bar::tx_slider_event_cb_class>::staticHandler;
	static constexpr auto freq_event_handler = EventHandler<gui_ft8bar, &gui_ft8bar::freq_event_handler_class>::staticHandler;

	
};

extern gui_ft8bar guift8bar;