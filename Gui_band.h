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

private:
	lv_obj_t	*tab;
	lv_obj_t	*button[100];
	lv_style_t	style_btn;
	
	int			button_width;
	int			button_height; 
	int			button_width_margin;
	int			button_height_margin;
};

extern Gui_band	gui_band_instance;