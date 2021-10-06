#pragma once
#include <string>
#include <vector>

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
void	set_gain_range();
void	step_gain_slider(int step);
void	step_vol_slider(int step);
int		get_vol_range();
void	get_gain_range(int &max_gain, int &min_gain);
void	get_filter_range(std::vector<std::string> &filters);


