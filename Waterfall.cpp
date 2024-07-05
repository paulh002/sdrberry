#include <cstdio>
#include <cmath>
#include <complex>
#include <liquid/liquid.h>
#include <vector>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "Waterfall.h"
#include "sma.h"
#include "Modes.h"

extern const int screenWidth;
extern const int screenHeight;
extern const int bottomHeight;
extern const int topHeight;
extern const int tunerHeight;
extern const int rightWidth;

Waterfall::Waterfall(lv_obj_t *scr, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
					 float r, int wfloor, waterfallFlow flow, partialspectrum p, int margin)
	: width(w), height(h), resampleRate(r), waterfallflow(flow), partialSpectrum(p),
	  excludeMargin(margin), max(50.0), min(0.0), factor(0.0f)
{
	lv_obj_set_style_pad_hor(scr, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(scr, 0, LV_PART_MAIN);
	
	cbuf.resize(LV_CANVAS_BUF_SIZE_TRUE_COLOR(w, h));
	canvas = lv_canvas_create(scr);
	lv_canvas_set_buffer(canvas, cbuf.data(), w, h, LV_IMG_CF_TRUE_COLOR);
	lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);
	lv_obj_set_pos(canvas,x,y);
	NumberOfBins = width - 2 * excludeMargin;
	SetPartial(partialSpectrum);
}

void Waterfall::SetMaxMin(float _max, float _min)
{
	max = _max;
	min = _min;
}

void Waterfall::SetPartial(partialspectrum p, float factor_, float downMixFrequency) 
{
	std::unique_lock<std::mutex> lock(mutexSingleEntry);
	partialSpectrum = p;
	resampleRate = factor_;
	switch (p)
	{
	case regionpart:
	case allparts:
		NumberOfBins = width - 2 * excludeMargin;
		break;
	case upperpart:
	case lowerpart:
		NumberOfBins = (width - 2 * excludeMargin) * 2;
		break;
	}
	fft.reset();
	fft = std::make_unique<FastFourier>(NumberOfBins, resampleRate, downMixFrequency);
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

void Waterfall::Draw(float waterfallfloor)
{
	std::unique_lock<std::mutex> lock(mutexSingleEntry);
	
	std::vector<uint8_t> buf;
	lv_img_dsc_t img;

	buf.resize(LV_CANVAS_BUF_SIZE_TRUE_COLOR(width, height -1));
	if (waterfallflow == up)
	{
		memcpy(buf.data(), cbuf.data() + LV_CANVAS_BUF_SIZE_TRUE_COLOR(width, 1), LV_CANVAS_BUF_SIZE_TRUE_COLOR(width, height - 1));
		img.data = (uint8_t *)buf.data();
		img.header.cf = LV_IMG_CF_TRUE_COLOR;
		img.header.w = width;
		img.header.h = height - 1;
		lv_canvas_transform(canvas, &img, 0, LV_IMG_ZOOM_NONE, 0, 0, width, height - 1, true);
	}
	else
	{
		memcpy(buf.data(), cbuf.data(), LV_CANVAS_BUF_SIZE_TRUE_COLOR(width, height - 1));
		img.data = (uint8_t *)buf.data();
		img.header.cf = LV_IMG_CF_TRUE_COLOR;
		img.header.w = width;
		img.header.h = height - 1;
		lv_canvas_transform(canvas, &img, 0, LV_IMG_ZOOM_NONE, 0, 1, width, height - 1, true);
	}

	std::vector<float> frequencySpectrum;
	if (partialSpectrum == allparts || partialSpectrum == regionpart)
		frequencySpectrum = fft->GetLineatSquaredBins();
	else
		frequencySpectrum = fft->GetSquaredBins();

	int zz = 0;	
	for (lv_coord_t i = excludeMargin; i < width - excludeMargin ; i++)
	{
		switch(partialSpectrum)
		{
		case upperpart:
			zz = (width / 2) + i - excludeMargin ;
			break;
		case allparts:
		case lowerpart:
			zz = i - excludeMargin;
			break;
		case regionpart:
			zz = i - excludeMargin;
			break;
		}
		lv_color_t c = heatmap(waterfallfloor + 20.0 * log10(frequencySpectrum.at(zz)), min, max);
		if (waterfallflow == up)
			lv_canvas_set_px_color(canvas, i, height - 1, c);
		else
			lv_canvas_set_px_color(canvas, i, 0, c);
		//printf("%f\n",  20 * log10(f.at(zz)));
	}
}

float Waterfall::lerp(float a, float b, float t)
{
	return a + t * (b - a);
}

// Resample array function
std::vector<float> Waterfall::resampleArray(const std::vector<float> &originalArray, size_t numPoints)
{
	std::vector<float> resampledArray(numPoints);

	float spacing = static_cast<float>(originalArray.size() - 1) / (numPoints - 1);

	for (size_t i = 0; i < numPoints; ++i)
	{
		float index = i * spacing;
		size_t lowIndex = static_cast<size_t>(index);
		size_t highIndex = std::min(lowIndex + 1, originalArray.size() - 1);

		float t = index - lowIndex;
		resampledArray[i] = lerp(originalArray[lowIndex], originalArray[highIndex], t);
	}

	return resampledArray;
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


