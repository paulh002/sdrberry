#pragma once
#include <string>
#include <vector>

extern const int screenWidth;
extern const int screenHeight;
extern const int bottomHeight;
extern const int topHeight;
extern const int tunerHeight;
extern const int rightWidth;

class Gui_right_pane
{
public:
	
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
	void	set_agc_slider(int pos);
	int		get_agc_slider();

	lv_style_t	style_btn;
	lv_obj_t*	bUsb, *bLsb, *bAM, *bFM, *bCW, *bFT8, *bg_right; 
	lv_obj_t*	agc_slider, *agc_slider_label;
	lv_obj_t*	gain_slider, *gain_slider_label;
	lv_obj_t*	vol_slider, *vol_slider_label;
	lv_obj_t*	fil_slider, *fil_slider_label;

private:
	const int nobuttons = 4;
	const int bottombutton_width = (rightWidth / nobuttons) - 2;
	const int bottombutton_width1 = (rightWidth / nobuttons);
	int button_height, button_margin = 18;
};

extern Gui_right_pane	grp;