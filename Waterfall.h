#pragma once
#include "lvgl.h"
#include "sma.h"
#include <atomic>
#include <cmath>
#include <complex>
#include <condition_variable>
#include <cstdio>
#include <fftw3.h>
#include <liquid/liquid.h>
#include <mutex>
#include <vector>
#include "FastFourier.h"

extern const int screenWidth;
extern const int screenHeight;
extern const int bottomHeight;
extern const int topHeight;
extern const int tunerHeight;
extern const int rightWidth;

class Waterfall
{
  public:
	Waterfall(lv_obj_t *scr, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, float resampleRate);
	~Waterfall();
	void Process(const IQSampleVector &input);
	void Draw();
	void SetMode(int mode);

  private:
	lv_obj_t *canvas{};
	std::vector<uint8_t> cbuf;
	std::unique_ptr<FastFourier> fft;
	std::mutex mutexSingleEntry;
	lv_color_t heatmap(float val, float min, float max);
	lv_coord_t width, height;
};