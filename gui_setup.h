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

class gui_setup
{
public:
	void	init(lv_obj_t* o_tab, lv_coord_t w);
	void	add_sample_rate(int samplerate);
	void	set_sample_rate(int rate);
	int		get_sample_rate(int rate);
	void	set_radio(std::string name);
	void	clear_sample_rate();
private:
	lv_style_t		style_btn;
	
	vector<int>		sample_rates;
	lv_obj_t*		d_samplerate;
	lv_obj_t*		d_receivers;
};

extern gui_setup	gsetup;