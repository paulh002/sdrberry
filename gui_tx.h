#pragma once
#include "AudioInput.h"
#include "lvgl.h"

extern const int screenWidth;
extern const int screenHeight;
extern const int bottomHeight;
extern const int topHeight;
extern const int tunerHeight;

class gui_tx
{
public:
	void gui_tx_init(lv_obj_t* o_tab, lv_coord_t w);
	void set_mic_slider(int volume);
	void step_mic_slider(int step);
	void set_drv_slider(int drive);
	void drv_mic_slider(int step);
	void set_tx_state(bool state);
	void set_drv_range();
	void add_sample_rate(int samplerate);
	void step_drv_slider(int step);
	void set_sample_rate(int rate);
	void clear_sample_rate();
	void set_group();
	
	int getbuttons()
	{
		return ibuttons;
	}
	
	lv_obj_t* get_button_obj(int i)
	{
		if (i >= ibuttons)
			return nullptr;
		return tx_button[i];
	}
	lv_obj_t* get_mic_label() {return mic_slider_label;}
	lv_obj_t* get_drv_label() {return drv_slider_label;}
	lv_obj_t* get_drv_slider() {return drv_slider;}
	int		  get_drv_pos();
	
private:
	lv_obj_t*		mic_slider_label, *mic_slider, *drv_slider, *drv_slider_label, *drp_samplerate; 

	lv_style_t		style_btn;
	lv_obj_t		*tx_button[10];
	vector<int>		sample_rates;
	int				ibuttons;
	const int		number_of_buttons {7};
	lv_group_t		*m_button_group{nullptr};
};

extern gui_tx	Gui_tx;