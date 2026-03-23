#include <cstdio>
#include <cstring>
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
#include "screen.h"

Waterfall::Waterfall(lv_obj_t *parent, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h,
					 float r, waterfallFlow flow, partialspectrum p, int margin)
	: obj_parent(parent), xpos(x), width(w), height(h), resampleRate(r), waterfallflow(flow), partialSpectrum(p),
	  excludeMargin(margin), max(50.0), min(0.0), factor(0.0f)
{
	lv_obj_set_style_pad_hor(parent, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(parent, 0, LV_PART_MAIN);

	canvas_buffer.resize(LV_CANVAS_BUF_SIZE(w, h, LV_COLOR_FORMAT_RGB565, LV_DRAW_BUF_STRIDE_ALIGN));
	canvas = lv_canvas_create(parent);
	lv_canvas_set_buffer(canvas, canvas_buffer.data(), w, h, LV_COLOR_FORMAT_RGB565);
	lv_canvas_fill_bg(canvas, lv_color_black(), LV_OPA_COVER);
	lv_obj_set_pos(canvas, x, y);
	temp_buffer.resize(LV_CANVAS_BUF_SIZE(width, height - 1, LV_COLOR_FORMAT_RGB565, LV_DRAW_BUF_STRIDE_ALIGN));
	NumberOfBins = width - 2 * excludeMargin;
	SetPartial(partialSpectrum, resampleRate);
	printf("Waterfall constructor: NumberOfBins %d x %d y%d w %d h %d \n", NumberOfBins, xpos, y, width, height );
}

void Waterfall::Size(lv_coord_t y, lv_coord_t h)
{
	lv_obj_set_pos(canvas, xpos, y);
	height = h;
	canvas_buffer.resize(LV_CANVAS_BUF_SIZE(width, h, LV_COLOR_FORMAT_RGB565, LV_DRAW_BUF_STRIDE_ALIGN));
	temp_buffer.resize(LV_CANVAS_BUF_SIZE(width, height - 1, LV_COLOR_FORMAT_RGB565, LV_DRAW_BUF_STRIDE_ALIGN));
	lv_canvas_set_buffer(canvas, canvas_buffer.data(), width, h, LV_COLOR_FORMAT_RGB565);
	printf("Waterfall Size message: NumberOfBins %d x %d y%d w %d h%d \n", NumberOfBins, xpos, y, width, height);
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
	
	lv_draw_buf_t *buf_ptr = lv_canvas_get_draw_buf(canvas);
	uint32_t line_width = buf_ptr->header.w;
	uint32_t dest_stride = buf_ptr->header.stride;
	uint32_t src_stride = buf_ptr->header.stride;
	uint32_t line_bytes = (line_width * lv_color_format_get_bpp((lv_color_format_t)buf_ptr->header.cf) + 7) >> 3;

	int32_t start_y, end_y;
	
	if (waterfallflow == up)
	{
		start_y = 1;
		end_y = buf_ptr->header.h - 1;
		uint8_t *src_bufc = (uint8_t *)lv_draw_buf_goto_xy(buf_ptr, 0, 0);
		uint8_t *dest_bufc = (uint8_t *)temp_buffer.data();

		for (; start_y < end_y; start_y++)
		{
			lv_memcpy(dest_bufc, src_bufc, line_bytes);
			dest_bufc += dest_stride;
			src_bufc += src_stride;
		}

		src_bufc = (uint8_t *)temp_buffer.data();
		dest_bufc = (uint8_t *)lv_draw_buf_goto_xy(buf_ptr, 0, 1);
		start_y = 0;
		end_y = buf_ptr->header.h - 1;

		for (; start_y < end_y; start_y++)
		{
			lv_memcpy(dest_bufc, src_bufc, line_bytes);
			dest_bufc += dest_stride;
			src_bufc += src_stride;
		}
	}
	else
	{
		start_y = 0;
		end_y = buf_ptr->header.h - 1;
		uint8_t *src_bufc = (uint8_t *)lv_draw_buf_goto_xy(buf_ptr, 0, 0);
		uint8_t *dest_bufc = (uint8_t *)temp_buffer.data();
		
		for (; start_y < end_y; start_y++)
		{
			lv_memcpy(dest_bufc, src_bufc, line_bytes);
			dest_bufc += dest_stride;
			src_bufc += src_stride;
		}

		src_bufc = (uint8_t *)temp_buffer.data();
		dest_bufc = (uint8_t *)lv_draw_buf_goto_xy(buf_ptr, 0, 1);
		start_y = 1;
		end_y = buf_ptr->header.h - 1;

		for (; start_y < end_y; start_y++)
		{
			lv_memcpy(dest_bufc, src_bufc, line_bytes);
			dest_bufc += dest_stride;
			src_bufc += src_stride;
		}
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
			lv_canvas_set_px(canvas, i, height - 1, c, LV_OPA_COVER);
		else
			lv_canvas_set_px(canvas, i, 0, c, LV_OPA_COVER);
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


