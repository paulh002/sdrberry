#pragma once
#include "Spectrum.h"
#include "gui_vfo.h"
#include "lvgl_.h"
#include <cstdint>
#include <memory>

class SecondScreen
{
  public:
	void init(lv_obj_t *parent_, lv_group_t *keyboard_group);
	void deinit();
	void DrawDisplay();
	void ProcessWaterfall(const IQSampleVector &input);
	void set_s_meter(double value);
	void setWaterfallSize(int waterfallsize);
	void init_vfo();
	void set_cursor_mode(int mode);

  private:
	int32_t x,
		y,
		width,
		height,
		vfo_height;
	lv_obj_t *parent{}, *spec;
	std::unique_ptr<Spectrum> SpectrumGraph_page;
	std::unique_ptr<gui_vfo> guivfo;
	// std::unique_ptr<gui_top_bar> GuiTopBar_page;
	std::string top_label;
};

extern std::unique_ptr<SecondScreen> secondscreen;
