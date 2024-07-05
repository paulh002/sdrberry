
#include <atomic>
#include <cmath>
#include <complex>
#include <condition_variable>
#include <cstdio>
#include <liquid/liquid.h>
#include <mutex>
#include <vector>
#include <algorithm>
#include "Spectrum.h"
#include "sma.h"
#include "DouglasPeucker.h"
#include "gui_setup.h"

using namespace std;

const int noise_floor{20};
const int hor_lines_small{6};
const int hor_lines_large{8};
const int vert_lines{9};

Spectrum SpectrumGraph;

void Spectrum::click_event_cb_class(lv_event_t *e)
{
	lv_point_t p;

	lv_indev_t *indev = lv_indev_get_act();
	lv_indev_type_t indev_type = lv_indev_get_type(indev);
	if (indev_type == LV_INDEV_TYPE_POINTER)
	{
		lv_indev_get_point(indev, &p);
		if (p.x > 90 * screenWidth / 100)
			vfo.setVfoFrequency(1);
		if (p.x < 10 * screenWidth / 100)
			vfo.setVfoFrequency(-1);
	}
}

void Spectrum::pressing_event_cb_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);

	lv_indev_t *indev = lv_indev_get_act();
	lv_indev_type_t indev_type = lv_indev_get_type(indev);
	if (indev_type == LV_INDEV_TYPE_POINTER)
	{
		lv_point_t p;
		lv_indev_get_point(indev, &p);
		if (p.x > 0)
		{
			long long f;
			int span = vfo.get_span();
			f = vfo.get_sdr_span_frequency();
			f = p.x *(span / screenWidth) + f;
			if (vfo.get_frequency() != f)
				vfo.set_vfo(f);
		}
	}
}

void Spectrum::draw_event_cb_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);

	lv_obj_draw_part_dsc_t *dsc = (lv_obj_draw_part_dsc_t *)lv_event_get_param(e);

	if (dsc->part == LV_PART_ITEMS)
	{
		if (!dsc->p1 || !dsc->p2)
			return;

		/*Add a line mask that keeps the area below the line*/
		lv_draw_mask_line_param_t line_mask_param;
		lv_draw_mask_line_points_init(&line_mask_param, dsc->p1->x, dsc->p1->y, dsc->p2->x, dsc->p2->y,
									  LV_DRAW_MASK_LINE_SIDE_BOTTOM);
		int16_t line_mask_id = lv_draw_mask_add(&line_mask_param, NULL);

		/*Add a fade effect: transparent bottom covering top*/
		lv_coord_t h = lv_obj_get_height(obj);
		lv_draw_mask_fade_param_t fade_mask_param;
		lv_draw_mask_fade_init(&fade_mask_param, &obj->coords, LV_OPA_COVER, obj->coords.y1 + h / 8, LV_OPA_50,
							   obj->coords.y2);
		int16_t fade_mask_id = lv_draw_mask_add(&fade_mask_param, NULL);

		/*Draw a rectangle that will be affected by the mask*/
		lv_draw_rect_dsc_t draw_rect_dsc;
		lv_draw_rect_dsc_init(&draw_rect_dsc);
		draw_rect_dsc.bg_opa = LV_OPA_40;
		draw_rect_dsc.bg_color = dsc->line_dsc->color;

		lv_area_t a;
		a.x1 = dsc->p1->x;
		a.x2 = dsc->p2->x - 1;
		a.y1 = LV_MIN(dsc->p1->y, dsc->p2->y);
		a.y2 = obj->coords.y2;
		lv_draw_rect(dsc->draw_ctx, &draw_rect_dsc, &a);

		/*Remove the masks*/
		lv_draw_mask_free_param(&line_mask_param);
		lv_draw_mask_free_param(&fade_mask_param);
		lv_draw_mask_remove_id(line_mask_id);
		lv_draw_mask_remove_id(fade_mask_id);
	}

	if (code == LV_EVENT_DRAW_PART_BEGIN)
	{
		/*Set the markers' text*/
		if (dsc->part == LV_PART_TICKS && dsc->id == LV_CHART_AXIS_PRIMARY_X)
		{
			std::pair<vfo_spansetting, double> span_ex = vfo.compare_span_ex();
			int span = gsetup.get_span();
			int ii; 
			double offset{0}, f{};

			switch (span_ex.first)
			{
			case span_is_ifrate:
				f = (double)vfo.get_sdr_frequency() - (double)(span / 2.0);
				ii = span / (vert_lines - 1);
				break;
			case span_between_ifrate:
				f = (double)vfo.get_sdr_frequency() - (double)vfo.get_minoffset() ;
				ii = span / (vert_lines - 1);
				break;
			case span_lower_halfrate:
				offset = vfo.get_vfo_offset() / span;
				f = (double)vfo.get_sdr_frequency() + offset * (double)span;
				ii = span / (vert_lines - 1);
				break;
			}
			f = f + dsc->value * ii;
			long l = (long)round(f / 1000.0);
			lv_snprintf(dsc->text, 19, "%ld", l);
		}
	}
}

void Spectrum::init(lv_obj_t *scr, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, float ifrate)
{
	int hor_lines = hor_lines_large;
	int heightChart, fontsize = 20, heightWaterfall;
	int waterfallsize = Settings_file.get_int("Radio", "waterfallsize", 3);
	signal_strength_offset = Settings_file.get_int("Radio", "s-meter-offset", 100);
		
	if (waterfallsize < 0 || waterfallsize > 10 || waterfallsize == 1)
		waterfallsize = 0;

	lv_style_init(&Spectrum_style);
	lv_style_set_radius(&Spectrum_style, 0);
	lv_style_set_bg_color(&Spectrum_style, lv_color_black());
	
	chart = lv_chart_create(scr);
	lv_obj_add_style(chart, &Spectrum_style, 0);
	
	heightChart = h;
	heightWaterfall = 0;
	if (waterfallsize)
	{
		lv_obj_set_pos(chart, x, y + fontsize);
		heightChart = h - (h * waterfallsize) / 10;
		heightWaterfall = (h * waterfallsize) / 10;
		lv_obj_set_size(chart, w, heightChart);
		lv_chart_set_axis_tick(chart, LV_CHART_AXIS_SECONDARY_X, 0, 0, vert_lines, 1, true, 100);
	}
	else
	{
		lv_obj_set_pos(chart, x, y);
		lv_obj_set_size(chart, w, h - fontsize);
		lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 0, 0, vert_lines, 1, true, 100);
	}
	if (waterfallsize > 3)
		hor_lines = hor_lines_small;
	lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 130);
	lv_obj_set_style_pad_hor(scr, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(scr, 0, LV_PART_MAIN);
	lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
	//LV_CHART_AXIS_PRIMARY_X
	//lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 0, 6, 1, true, 80);
	lv_chart_set_div_line_count(chart, hor_lines, vert_lines);
	//lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 0, 0, vert_lines, 1, true, 100);
	lv_obj_add_event_cb(chart, draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, (void *)this);
	lv_obj_add_event_cb(chart, pressing_event_cb, LV_EVENT_PRESSING, (void *)this);
	lv_obj_add_event_cb(lv_obj_get_parent(chart), click_event_cb, LV_EVENT_CLICKED, (void *)this);
	m_cursor = lv_chart_add_cursor(chart, lv_palette_main(LV_PALETTE_BLUE), LV_DIR_BOTTOM | LV_DIR_TOP);

	lv_obj_set_style_size(chart, 0, LV_PART_INDICATOR);
	ser = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
	lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
	for (int i = 0; i < nfft_samples / 2; i++)
		data_set.push_back(0);
	lv_chart_set_point_count(chart, data_set.size());
	lv_chart_set_ext_y_array(chart, ser, (lv_coord_t *)data_set.data());
	avg_filter.resize(nfft_samples);
	fft = std::make_unique<FastFourier>(nfft_samples, 0, 0);
	if (waterfallsize > 0)
	{
		int waterfallfloor = Settings_file.get_int("Radio", "waterfallfloor", 10);
		waterfall = std::make_unique<Waterfall>(scr, x, heightChart + fontsize, w, heightWaterfall - fontsize, 0.0, waterfallfloor, down, allparts, 12);
	}
	SetFftParts();
}

void Spectrum::DrawDisplay(int ns)
{
	noisefloor = ns;
	upload_fft();
	load_data();
	if (waterfall)
		waterfall->Draw((float)noisefloor);
}

void Spectrum::SetFftParts()
{
	std::pair<vfo_spansetting, double> span_ex = vfo.compare_span_ex();
	if (fft)
	{
		switch (span_ex.first)
		{
		case span_is_ifrate:
			fft->SetParameters(nfft_samples);
			break;
		case span_between_ifrate:
			fft->SetParameters(nfft_samples, (float)vfo.get_span() / (float)ifrate,  0.5f * ((float)ifrate - (float)vfo.get_span()) / ifrate);
			break;
		case span_lower_halfrate:
			fft->SetParameters(nfft_samples, ((float)vfo.get_span() * 2.0f) / ifrate);
			break;
		}
	}
	
	if (waterfall)
	{
		switch (span_ex.first)
		{
		case span_is_ifrate:
			waterfall->SetPartial(allparts);
			break;
		case span_between_ifrate:
			waterfall->SetPartial(regionpart, (float)vfo.get_span() / (float)ifrate, 0.5f * ((float)ifrate - (float)vfo.get_span()) / ifrate);
			break;
		case span_lower_halfrate:
			waterfall->SetPartial(lowerpart, ((float)vfo.get_span() * 2.0f) / ifrate);
			break;
		}
	}
}

void Spectrum::set_signal_strength(double strength)
{
	signal_strength = 40 * log10(strength) + signal_strength_offset;
	//printf("S %f offset %d \n", signal_strength.load(), signal_strength_offset);
}

void Spectrum::ProcessWaterfall(const IQSampleVector &input)
{
	fft->Process(input);
	if (waterfall)
		waterfall->Process(input);
}

void Spectrum::set_pos(int32_t offset)
{
	int pos;
	float d;
	int span = vfo.get_span();
	

	if (!data_set.size())
		return;

	d = (float)(data_set.size() * ((float)(offset + vfo.get_minoffset())) / (float)span);
	pos = d - 1;
	if (pos < 0)
		pos = 0;
	if (pos >= data_set.size())
		pos = data_set.size() - 1;
	lv_chart_set_cursor_point(chart, m_cursor, NULL, pos);
//	printf("Pos: freq: %ld sdr %ld offset %d pos: %d span %d \n", (long)vfo.get_frequency(), (long)vfo.get_sdr_frequency(), offset, pos, span);
}

void Spectrum::load_data()
{
	int i = data_set.size();
	if (i > 0)
	{
		lv_chart_set_point_count(chart, i);
		lv_chart_set_ext_y_array(chart, ser, (lv_coord_t *)data_set.data());
		lv_chart_refresh(chart);
	}
}

void Spectrum::upload_fft()
{
	int i{};
	std::pair<vfo_spansetting, double> span_ex = vfo.compare_span_ex();
	std::vector<int> peaks;

	switch (span_ex.first)
	{
	case span_is_ifrate:
	case span_between_ifrate:
	{
			int ii{}, value{};
			std::vector<float> fft_output = fft->GetLineatSquaredBins();
			data_set.resize(fft_output.size() / 2);
			finder.uploadData(fft_output);
			for (auto &col : fft_output)
			{
				value = noisefloor + (lv_coord_t)(20.0 * log10(col));
				if (value > (float)s_poits_max)
					value = (float)s_poits_max;
				if (i % 2)
				{
					data_set[ii] = avg_filter[ii](value);
					ii++;
				}
				i++;
			}
		}
		break;
	case span_lower_halfrate:
		{
			int value{};
			std::vector<float> fft_output = fft->GetSquaredBins();
			data_set.resize(fft_output.size() / 2);
			for (auto &col : fft_output)
			{
				if (i == (fft_output.size() / 2))
					break;
				value = noisefloor + (lv_coord_t)(20.0 * log10(col));
				if (value > (float)s_poits_max)
					value = (float)s_poits_max;
				data_set[i] = avg_filter[i](value);
				i++;
			}
		}
	break;
	}
}

float Spectrum::getSuppression()
{
	return finder.GetSuppression();
}