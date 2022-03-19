#pragma once

#include "AudioInput.h"
#include "lvgl/lvgl.h"
#include "Settings.h"
#include "sdrberry.h"
#include "vfo.h"
#include "gui_vfo.h"

extern const int screenWidth;
extern const int screenHeight;
extern const int bottomHeight;
extern const int topHeight;
extern const int tunerHeight;

class gui_bar
{
public:
  gui_bar();
  ~gui_bar();
  void init(lv_obj_t *o_parent, lv_group_t *button_group, int mode, lv_coord_t w, lv_coord_t h);
  void set_vol_slider(int volume);
  void set_focus();
  void set_filter_slider(int filter);
  void step_vol_slider(int step);
  void step_gain_slider(int step);
  void select_option(int option);
  lv_obj_t *get_vol_slider_label() { return vol_slider_label; }
  lv_obj_t *get_gain_slider_label() { return gain_slider_label; }
  lv_obj_t *get_if_slider_label() { return if_slider_label; }
  int get_vol_range();
  float get_if();
  void set_if(int rf);
  void set_gain_range();
  void update_gain_slider(int gain);
  void set_gain_slider(int gain);
  int getbuttons() { return ibuttons; }
  void set_mode(int mode);
  void check_agc();
  void set_cw_message(std::string message);
  void set_cw_wpm(int wpm);
  void hide_cw(bool hide);
  void set_led(bool status);
  void set_tx(bool tx);
  void get_gain_range(int &max_gain, int &min_gain);
  
  lv_obj_t *get_button_obj(int i)
  {
	  if (i >= ibuttons)
		  return nullptr;
	  return button[i];
	}
	void		get_filter_range(vector<string> &filters);
	int			get_ifilters(int sel)
	{
		return ifilters[sel];
	}

	atomic<float> m_if;
	
private:
	lv_style_t	style_btn;
	lv_obj_t	*button[20] { nullptr };
	int			ibuttons {0};
	int			filter;
	const int	number_of_buttons {12};
	lv_obj_t*	vol_slider, *vol_slider_label, *gain_slider, *gain_slider_label;
	lv_obj_t	*if_slider_label, *if_slider;
	const int	max_volume {100};
	vector<int>	ifilters;
	lv_obj_t	*cw_wpm, *cw_message, *cw_box, *cw_led;
	lv_style_t	cw_style, style_selected_color;
	lv_group_t	*m_button_group{nullptr};
	
};

extern gui_bar gbar;