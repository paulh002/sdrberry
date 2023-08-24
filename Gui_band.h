#pragma once

#include "lvgl.h"
#include <SoapySDR/ConverterRegistry.hpp>
#include <SoapySDR/Device.hpp>
#include <SoapySDR/Formats.hpp>
#include <SoapySDR/Modules.hpp>
#include <SoapySDR/Registry.hpp>
#include <SoapySDR/Types.hpp>
#include <SoapySDR/Version.hpp>

#define MAX_BAND_BUTTONS 100

class Gui_band
{
  public:
	void init_button_gui(lv_obj_t *o_tab, lv_coord_t w, lv_coord_t h, SoapySDR::RangeList);
	int getbuttons()
	{
		return ibuttons;
	}
	lv_obj_t *get_button_obj(int i)
	{
		if (i >= ibuttons)
			return nullptr;
		return button[i];
	}
	void set_gui(int band);
	void set_group();

  private:
	lv_obj_t *tab;
	lv_obj_t *button[100];
	lv_obj_t *limitvfocheckbox, *bandfiltercheckbox;
	lv_style_t style_btn;

	int button_width;
	int button_height;
	int button_width_margin;
	int button_height_margin;
	int ibuttons;
	lv_group_t *m_button_group{nullptr};
};

int getIndex(vector<int> v, int s);
extern Gui_band gui_band_instance;