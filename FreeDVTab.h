#pragma once
#include "lvgl.h"
#include "Waterfall.h"

class FreeDVTab
{
  public:
	void init(lv_obj_t *scr, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h);
	void DrawWaterfall();
	void Process(const IQSampleVector &input);
	void SetMode(int mode);

  private:
	std::unique_ptr<Waterfall> waterfall;
};

extern FreeDVTab freeDVTab;