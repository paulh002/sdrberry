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
	int			get_vol_range();
	int			getbuttons() 	{return ibuttons;}
	void		set_mode(int mode);
	lv_obj_t*	get_button_obj(int i)
	{
		if (i >= ibuttons)
			return nullptr;
		return button[i];
	}
	void		get_filter_range(vector<string> &filters);
	
private:
	lv_style_t	style_btn;
	lv_obj_t	*button[20];

	int			ibuttons {0};
	const int	number_of_buttons {5};
	lv_obj_t*	vol_slider, *vol_slider_label;
	const int max_volume {100};
};

extern gui_bar gbar;