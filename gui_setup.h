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
	lv_obj_t*		get_span_slider_label()	{return span_slider_label;}
	int		get_span() {return m_span.load();}
	void	set_span_range(int span);
	void	set_span_value(int span);
	void	set_brightness(int brightness);
	int		get_brightness();
	bool	get_cw();
	void	set_cw(bool bcw);
	void	toggle_cw();
	void	set_group();
	void set_contour_value(int speed);
	void set_floor_value(int floor);
	int get_contour_value();
	void init_bandwidth();
	int get_bandwidth_sel() { return lv_dropdown_get_selected(d_bandwitdth); }
	double			m_ifrate;
	
private:
	lv_style_t		style_btn;
	vector<int>		sample_rates;
	lv_obj_t*		d_samplerate;
	lv_obj_t*		d_receivers;
	lv_obj_t*		span_slider_label, *span_slider;
	lv_obj_t*		brightness_slider_label, *brightness_slider;
	lv_obj_t*		d_audio, *check_cw, *d_bandwitdth;
	atomic<int>		m_span;
	lv_group_t		*m_button_group{nullptr};
	lv_obj_t *contour_slider_label, *contour_slider;
	lv_obj_t *floor_slider_label, *floor_slider;
};

extern gui_setup	gsetup;