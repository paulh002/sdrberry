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
	void	set_ratio_range();
	void	set_ratio_range(int min, int max);
	void	set_ratio_slider(int ratio);
	void	set_group();
	void	set_agc_mode(int m);

	lv_obj_t *get_ratio_slider_label() { return ratio_slider_label; }
	
	int getbuttons() {return ibuttons;	}
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

	lv_obj_t *get_atack_slider_label()
	{
		return atack_slider_label;
	}

	lv_obj_t *get_release_slider_label()
	{
		return release_slider_label;
	}
	
	int get_threshold();
	float get_atack();
	float get_release();
	float get_ratio();
	void set_atack_slider(int t);
	void set_threshold_slider(int t);
	void set_release_slider(int t);
	int get_agc_mode()
	{
		return agc_mode;
	}
	
	//const int	max_threshold {200};
	
private:
	lv_style_t	style_btn;
	lv_obj_t	*ratio_slider_label, *ratio_slider;
	lv_obj_t	*threshold_slider_label, *threshold_slider;
	lv_obj_t	*atack_slider_label, *atack_slider;
	lv_obj_t	*release_slider_label, *release_slider;
	lv_obj_t	*button[10];
	int			ibuttons {0};
	const int	number_of_buttons {4};
	const int	top_y {10};
	int			agc_mode {0};
	lv_group_t *m_button_group{nullptr};
};


extern Gui_agc gagc;