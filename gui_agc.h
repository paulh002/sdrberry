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

class Gui_agc
{
public:
	void	init(lv_obj_t* o_tab, lv_coord_t w);
	void	set_gain_range();
	void	step_gain_slider(int step);
	void	set_gain_range(int min, int max);
	void	set_gain_slider(int gain);
	void	get_gain_range(int &max_gain, int &min_gain);
	void	update_gain_slider(int gain);
	void	set_group();
	lv_obj_t *get_gain_slider_label() {return gain_slider_label;}
	
	int getbuttons()
	{
		return ibuttons;
	}
	lv_obj_t* get_button_obj(int i)
	{
		if (i >= ibuttons)
			return nullptr;
		return button[i];
	}
	
	lv_obj_t *get_threshold_slider_label()
	{
		return threshold_slider_label;
	}
	
	lv_obj_t *get_slope_slider_label()
	{
		return slope_slider_label;
	}
	
	lv_obj_t *get_delay_slider_label()
	{
		return delay_slider_label;
	}
	int get_threshold()
	{
		return lv_slider_get_value(threshold_slider) * -1;// - max_threshold;
	}
	
	int get_slope()
	{
		return lv_slider_get_value(slope_slider);
	}
	
	void set_slope_slider(int t);
	void set_threshold_slider(int t);
	void set_delay_slider(int t);
	int get_agc_mode()
	{
		return agc_mode;
	}
	void	set_agc_mode(int m)
	{
		agc_mode = m;
	}
	
	const int	max_threshold {150};
	
private:
	lv_style_t	style_btn;
	lv_obj_t	*gain_slider_label, *gain_slider;
	lv_obj_t	*threshold_slider_label, *threshold_slider;
	lv_obj_t	*slope_slider_label, *slope_slider;
	lv_obj_t	*delay_slider_label, *delay_slider;
	lv_obj_t	*button[10];
	int			ibuttons {0};
	const int	number_of_buttons {4};
	const int	top_y {10};
	int			agc_mode {0};
	lv_group_t *m_button_group{nullptr};
};


extern Gui_agc gagc;