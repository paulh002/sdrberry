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
	void		init(lv_obj_t* o_parent, int mode, lv_coord_t w, lv_coord_t h);
	void		set_vol_slider(int volume);
	void		set_filter_slider(int filter);
	void		step_vol_slider(int step);
	lv_obj_t*	get_vol_slider_label() {return vol_slider_label;}
	lv_obj_t*	get_gain_slider_label() {return gain_slider_label;}
	int			get_vol_range();
	void		set_gain_range();
	void		update_gain_slider(int gain);
	void		set_gain_slider(int gain);
	int			getbuttons() 	{return ibuttons;}
	void		set_mode(int mode);
	void		check_agc();
	void		set_cw_message(std::string message);
	void		set_cw_wpm(int wpm);
	void		hide_cw(bool hide);
	void		set_led(bool status);
	
	lv_obj_t*	get_button_obj(int i)
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
	void		set_tx(bool tx);
	
private:
	lv_style_t	style_btn;
	lv_obj_t	*button[20] { nullptr };
	int			ibuttons {0};
	const int	number_of_buttons {10};
	lv_obj_t*	vol_slider, *vol_slider_label, *gain_slider, *gain_slider_label;
	const int max_volume {100};
	vector<int>	ifilters;
	lv_obj_t *cw_wpm, *cw_message, *cw_box, *cw_led;
	lv_style_t	cw_style;
};

extern gui_bar gbar;