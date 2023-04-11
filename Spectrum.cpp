
#include "Spectrum.h"
#include "sdrberry.h"
#include "sma.h"
#include <atomic>
#include <cmath>
#include <complex>
#include <condition_variable>
#include <cstdio>
#include <liquid/liquid.h>
#include <mutex>
#include <vector>

using namespace std;

const int noise_floor{20};
const int hor_lines{8};
const int vert_lines{9};

Spectrum SpectrumGraph;

static void click_event_cb(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);

	lv_indev_t *indev = lv_indev_get_act();
	lv_indev_type_t indev_type = lv_indev_get_type(indev);
	if (indev_type == LV_INDEV_TYPE_POINTER)
	{
		lv_point_t p;
		lv_indev_get_point(indev, &p);

		long long f;
		int span = gsetup.get_span();
		if (!vfo.compare_span())
		{
			f = vfo.get_sdr_frequency() - (long long)(span / 2.0);
		}
		else
		{
			f = vfo.get_sdr_frequency();
		}
		vfo.set_vfo(p.x * (span / screenWidth) + f);
	}
}

static void draw_event_cb(lv_event_t *e)
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
			string str[vert_lines];
			int span = gsetup.get_span();
			long long f;
			int ii;

			if (!vfo.compare_span())
			{
				f = vfo.get_sdr_frequency() - (long long)(span / 2.0);
				ii = span / (vert_lines - 1);
			}
			else
			{
				f = vfo.get_sdr_frequency();
				ii = span / (vert_lines - 1);
			}

			for (int i = 0; i < vert_lines; i++)
			{
				char str1[20];
				//double l = (double)((f / 10ULL) % 1000000ULL) / 100.0;
				long l = (long)(f / 100ULL);
				sprintf(str1, "%ld", l);
				str[i] = string(str1);
				size_t pos = str[i].length();
				f += ii;
			}
			//sizeof(dsc->text)
			lv_snprintf(dsc->text, str[dsc->value].length(), "%s", str[dsc->value].c_str());
		}
	}
}

void Spectrum::init(lv_obj_t *scr, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, float ifrate)
{

	int heightChart, fontsize = 20, heightWaterfall;
	int waterfallsize = Settings_file.get_int("Radio", "waterfallsize", 3);

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
	lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
	lv_obj_set_style_pad_hor(scr, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(scr, 0, LV_PART_MAIN);
	lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
	//LV_CHART_AXIS_PRIMARY_X
	//lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 0, 6, 1, true, 80);
	lv_chart_set_div_line_count(chart, hor_lines, vert_lines);
	//lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 0, 0, vert_lines, 1, true, 100);
	lv_obj_add_event_cb(chart, draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);
	lv_obj_add_event_cb(chart, click_event_cb, LV_EVENT_CLICKED, NULL);
	m_cursor = lv_chart_add_cursor(chart, lv_palette_main(LV_PALETTE_BLUE), LV_DIR_BOTTOM | LV_DIR_TOP);

	lv_obj_set_style_size(chart, 0, LV_PART_INDICATOR);
	ser = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
	lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
	m_ifrate = ifrate;
	for (int i; i < nfft_samples / 2; i++)
		data_set.push_back(0);
	lv_chart_set_point_count(chart, data_set.size());
	lv_chart_set_ext_y_array(chart, ser, (lv_coord_t *)data_set.data());
	avg_filter.resize(nfft_samples);
	fft = std::make_unique<FastFourier>(nfft_samples, 0.0);
	if (waterfallsize > 0)
	{	
		int waterfallfloor = Settings_file.get_int("Radio", "waterfallfloor", 10);
		waterfall = std::make_unique<Waterfall>(scr, x, heightChart + fontsize, w, heightWaterfall - fontsize, 0.0, waterfallfloor, down, allparts, 12);
		int span = gsetup.get_span();
		if ((ifrate - (float)span) > 0.1)
			waterfall->SetSpan(span);
	}
}

void Spectrum::DrawWaterfall()
{
	upload_fft();
	if (waterfall)
		waterfall->Draw();
}

void Spectrum::SetSpan(int span)
{
	if (waterfall)
		waterfall->SetSpan(span);
}

void Spectrum::set_signal_strength(double strength)
{
	signal_strength = 20 * log10(strength) + 120;
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
	int span = gsetup.get_span();

	if (vfo.compare_span())
	{
		offset = offset - ((m_ifrate.load() / 2) * m_n.load());
		float div = m_ifrate.load() / nfft_samples;
		pos = (uint32_t)round(offset / div) - 1;
	}
	else
	{
		// currenlty nog negative offset still need to change
		//d = ((float)offset + (float)m_ifrate.load() / 2.0) / (float)m_ifrate.load();
		d = ((float)offset + (float)span / 2.0) / (float)span;
		pos = (d * (float)data_set.size()) - 1;
	}
	if (pos < 0)
		pos = 0;
	if (pos >= data_set.size())
		pos = data_set.size() - 1;
	lv_chart_set_cursor_point(chart, m_cursor, NULL, pos);
	//printf("sdr %ld offset %d pos: %d ifrate %f \n", (long)vfo.get_sdr_frequency(), offset, pos, m_ifrate.load());
}

void Spectrum::set_fft_if_rate(float ifrate, int n)
{
	m_ifrate.store(ifrate);
	m_n.store(n);
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
	int i{}, value{};

	int noisefloor = Settings_file.get_int("Radio", "noisefloor", noise_floor);
	if (vfo.compare_span())
	{
		std::vector<float> fft_output = fft->GetSquaredBins();
		data_set.resize(fft_output.size() / 2);
		for (auto &col : fft_output)
		{
			if (i == (fft_output.size() / 2))
				break;
			value = noisefloor + (lv_coord_t)(20.0 * log10(col));
			if (value > 99.0)
				value = 99.0;
			data_set[i] = avg_filter[i](value);
			i++;
		}
	}
	else
	{
		int ii = 0;
		std::vector<float> fft_output = fft->GetLineatSquaredBins();
		data_set.resize(fft_output.size()/2);
		for (auto &col : fft_output)
		{
			value = noisefloor + (lv_coord_t)(20.0 * log10(col));
			if (value > 99.0)
				value = 99.0;
			if (i % 2)
			{
				data_set[ii] = avg_filter[ii](value);
				ii++;
			}
			i++;
		}
	}
}


