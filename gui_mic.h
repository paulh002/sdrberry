#pragma once
#include "lvgl_.h"
#include "audiobar.h"
#include "SMeterFilter.h"
#include <memory>

class gui_mic
{
private:
	lv_obj_t *mic_bar, *alc_bar, *smeter_bar;
	lv_style_t mic_style;
	audiobar audiobar_mic, audiobar_alc, audiobar_s;
	std::unique_ptr<SMeterFilter> mic_filter, alc_filter, s_filter;
	
public:
	void init(lv_obj_t *parent, lv_coord_t w, lv_coord_t h);
	void set_value_mic(float value);
	void set_value_alc(float value);
	void set_value_s(float value);
};

extern gui_mic guimic;