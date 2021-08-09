#pragma once

extern const int screenWidth;
extern const int screenHeight;
extern const int bottomHeight;
extern const int topHeight;
extern const int tunerHeight;
extern const int rightWidth;


void	setup_right_pane(lv_obj_t* scr);
void	set_gain_range(int min, int max);
void	hide_agc_slider(void);
void	set_vol_slider(int volume);
void	set_gain_slider(int gain);
void	set_filter_slider(int filter);