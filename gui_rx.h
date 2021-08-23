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
extern const int rightWidth;


class gui_rx
{
public:
	void gui_rx_init(lv_obj_t* o_tab, lv_coord_t w);
	int getbuttons()
	{
		return ibuttons;
	}
	lv_obj_t* get_button_obj(int i)
	{
		if (i >= ibuttons)
			return nullptr;
		return rx_button[i];
	}
	void	add_sample_rate(int samplerate);
	
private:
	int				mode;	
	lv_obj_t*		mic_slider_label, *mic_slider, *drv_slider, *drv_slider_label;
	lv_style_t		style_btn;
	lv_obj_t		*rx_button[10];
	int				ibuttons;
	vector<int>		sample_rates;
	lv_obj_t*		d_samplerate;
};

extern gui_rx	Gui_rx;