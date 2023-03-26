#include <cstdio>
#include <cmath>
#include <complex>
#include <liquid/liquid.h>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "Waterfall.h"
#include "sdrberry.h"
#include "sma.h"
#include "Modes.h"

extern const int screenWidth;
extern const int screenHeight;
extern const int bottomHeight;
extern const int topHeight;
extern const int tunerHeight;
extern const int rightWidth;

Waterfall::Waterfall(lv_obj_t *scr, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, float resampleRate)
	: width(w), height(h)
{
	lv_obj_set_style_pad_hor(scr, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(scr, 0, LV_PART_MAIN);
	
	cbuf.resize(LV_CANVAS_BUF_SIZE_TRUE_COLOR(w, h));
	canvas = lv_canvas_create(scr);
	lv_canvas_set_buffer(canvas, cbuf.data(), w, h, LV_IMG_CF_TRUE_COLOR);
	lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);
	lv_obj_set_pos(canvas,x,y);
	fft = std::make_unique<FastFourier>(width, resampleRate);
	waterfallfloor = Settings_file.get_int("Waterfall", "floor", 60);
}

Waterfall::~Waterfall()
{
	lv_obj_del(canvas);
}

void Waterfall::SetMode(int mode)
{
	if (mode == mode_lsb)
		fft->SetInvert(true);
	else
		fft->SetInvert(false);
}

void Waterfall::Process(const IQSampleVector &input)
{
	std::unique_lock<std::mutex> lock(mutexSingleEntry);
	fft->Process(input);
}

void Waterfall::Draw()
{
	std::unique_lock<std::mutex> lock(mutexSingleEntry);
	
	std::vector<uint8_t> buf;
	buf.resize(LV_CANVAS_BUF_SIZE_TRUE_COLOR(width, height -1));
	memcpy(buf.data(), cbuf.data() + LV_CANVAS_BUF_SIZE_TRUE_COLOR(width, 1), LV_CANVAS_BUF_SIZE_TRUE_COLOR(width, height - 1));
	lv_img_dsc_t img;
	img.data = (uint8_t *)buf.data();
	img.header.cf = LV_IMG_CF_TRUE_COLOR;
	img.header.w = width;
	img.header.h = height-1;
	lv_canvas_transform(canvas, &img, 0, LV_IMG_ZOOM_NONE, 0, 0, width, height-1, true);

	std::vector<float> f = fft->GetSquaredBins();
	int zz =0;
	for (lv_coord_t i = 0; i < width; i++)
	{
		if (i < width / 2)
			zz = (width / 2) + i;
		else
			zz = i - (width / 2);
		lv_color_t c = heatmap((float)waterfallfloor + 20.0 * log10(f.at(zz)), 0.0, 50.0);
		lv_canvas_set_px_color(canvas, i, height - 1, c);
		//printf("%f\n",  20 * log10(f.at(zz)));
	}
}

lv_color_t Waterfall::heatmap(float val, float min, float max)
{
	unsigned r = 0;
	unsigned g = 0;
	unsigned b = 0;

	val = (val - min) / (max - min);
	if (val <= 0.2)
	{
		b = (unsigned)((val / 0.2) * 255);
	}
	else if (val > 0.2 && val <= 0.7)
	{
		b = (unsigned)((1.0 - ((val - 0.2) / 0.5)) * 255);
	}
	if (val >= 0.2 && val <= 0.6)
	{
		g = (unsigned)(((val - 0.2) / 0.4) * 255);
	}
	else if (val > 0.6 && val <= 0.9)
	{
		g = (unsigned)((1.0 - ((val - 0.6) / 0.3)) * 255);
	}
	if (val >= 0.5)
	{
		r = (unsigned)(((val - 0.5) / 0.5) * 255);
	}
	//printf("%f %x %x %x\n", val, r, g, b);
	return lv_color_make(r, g, b);
}


