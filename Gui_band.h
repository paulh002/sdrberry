#pragma once

#include <SoapySDR/Version.hpp>
#include <SoapySDR/Modules.hpp>
#include <SoapySDR/Registry.hpp>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/ConverterRegistry.hpp>
#include <SoapySDR/Types.hpp>
#include <SoapySDR/Formats.hpp>
#include "lvgl/lvgl.h"

#define MAX_BAND_BUTTONS	100


class Gui_band
{
public:
	void init_button_gui(lv_obj_t *o_tab, lv_coord_t w, SoapySDR::RangeList);
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
	void		set_gui(int band);
	
private:
	lv_obj_t	*tab;
	lv_obj_t	*button[100];
	lv_style_t	style_btn;
	
	int			button_width;
	int			button_height; 
	int			button_width_margin;
	int			button_height_margin;
	int			ibuttons;
};

int getIndex(vector<int> v, int s);
extern Gui_band	gui_band_instance;