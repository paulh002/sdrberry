
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
#include "screen.h"
#include "WebServer.h"
#include "gui_rx.h"
#include "gui_bar.h"
#include <linux/input.h>

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

std::pair<bool,int> Spectrum::cursor_intersect(lv_point_t p)
{
	int i = 0;
	for (auto mark : markers)
	{
		if (mark != nullptr && abs(mark->pos.x - p.x) < 10)
			return std::make_pair(true,i);
		i++;
	}
	return std::make_pair(false,0);
}

void Spectrum::pressing_event_cb_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);

	lv_indev_t *indev = lv_indev_get_act();
	lv_indev_type_t indev_type = lv_indev_get_type(indev);
	uint32_t btn_id = lv_indev_get_button(indev);

	if (indev_type == LV_INDEV_TYPE_POINTER && code == LV_EVENT_RELEASED && btn_id == BTN_RIGHT)
	{
		drag_marker_rightbutton = 0;
		vfo.set_frequency_to_left(newspanstartfreq, vfo.get_active_vfo(), true);
		vfo.set_vfo(vfo.get_frequency());
		//SpectrumGraph.SetFftParts();
		gbar.updateweb();
	}

	if (indev_type == LV_INDEV_TYPE_POINTER && code == LV_EVENT_PRESSING && btn_id == BTN_RIGHT)
	{
		lv_point_t p;
		lv_indev_get_point(indev, &p);

		if (lv_indev_get_button(indev) == BTN_RIGHT && p.x != p_drag.x)
		{
			p_drag = p;
			long long df{0LL}, spanfreq;
			int span = vfo.get_span();
			spanfreq = vfo.get_sdr_frequency(); // vfo.get_sdr_span_frequency(); // f max left
			df = p.x * (span / screenWidth);
			if (!drag_marker_rightbutton)
			{
				drag_frequency_shift = df;
				drag_marker_rightbutton = 1;
			}
			else
			{
				drag_frequency = df - drag_frequency_shift;
				drag_frequency_shift = df;
			}
			//printf("freq %lld df %lld\n", drag_frequency, df);
			newspanstartfreq = spanfreq - drag_frequency;
			vfo.set_frequency_to_left(newspanstartfreq, vfo.get_active_vfo(), false);
			vfo.set_vfo(vfo.get_frequency());
		}
		return;
	}
	
	if (indev_type == LV_INDEV_TYPE_POINTER && code == LV_EVENT_RELEASED && btn_id == BTN_LEFT)
	{
		// make sure only the marker is dragged, fast mouse movements will skipp multiple x possitions
		drag_marker = 0;
	}

	if (indev_type == LV_INDEV_TYPE_POINTER && code == LV_EVENT_PRESSING && btn_id == BTN_LEFT)
	{
		lv_point_t p;
		lv_indev_get_point(indev, &p);

		if (p.x > 0)
		{
			auto ret = cursor_intersect(p);
			if (ret.first || drag_marker) // close to marker or drag mode
			{
				if (drag_marker) // make sure the same marker is dragged
				{
					set_marker(drag_marker - 1, (data_set.size() * p.x) / screenWidth);
				}
				else
				{
					set_marker(ret.second, (data_set.size() * p.x) / screenWidth);
					drag_marker = ret.second + 1;
				}
			}
			else
			{
				long long f;
				int span = vfo.get_span();
				f = vfo.get_sdr_span_frequency();
				f = p.x * (span / screenWidth) + f;
				if (vfo.get_frequency() != f)
					vfo.set_vfo(f);
			}
		}
	}
}

void Spectrum::draw_event_cb_class(lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);
	lv_obj_t *obj = lv_event_get_target(e);

	lv_obj_draw_part_dsc_t *dsc = (lv_obj_draw_part_dsc_t *)lv_event_get_param(e);

	if (dsc->part == LV_PART_ITEMS && dsc->sub_part_ptr == ser)
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
		if (lv_obj_draw_part_check_type(dsc, &lv_chart_class, LV_CHART_DRAW_PART_CURSOR))
		{
			for (auto cursor : markers)
			{
				if (cursor != nullptr && dsc->sub_part_ptr == cursor)
				{
					draw_marker_label(cursor, dsc);
				}
			}
			if (dsc->sub_part_ptr == FrequencyCursor && active_markers > 0)
				draw_marker_label(FrequencyCursor, dsc);
		}
	}
}

void Spectrum::setSignalStrenthOffset(int offset)
{
	signal_strength_offset = offset;
}

void Spectrum::init(lv_obj_t *scr, lv_coord_t x, lv_coord_t y, lv_coord_t w, lv_coord_t h, float ifrate)
{
	int hor_lines = hor_lines_large;
	int waterfallsize = Settings_file.get_int("Radio", "waterfallsize", 3);
	signal_strength_offset = Settings_file.get_int("Radio", "s-meter-offset", 200);

	height = h;
	width = w;
	xx = x;
	yy = y;
	fontsize = 20;
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
	lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 100);
	lv_obj_set_style_pad_hor(scr, 0, LV_PART_MAIN);
	lv_obj_set_style_pad_ver(scr, 0, LV_PART_MAIN);
	lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
	//LV_CHART_AXIS_PRIMARY_X
	//lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 0, 6, 1, true, 80);
	lv_chart_set_div_line_count(chart, hor_lines, vert_lines);
	//lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 0, 0, vert_lines, 1, true, 100);
	lv_obj_add_event_cb(chart, draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, (void *)this);
	lv_obj_add_event_cb(chart, pressing_event_cb, LV_EVENT_PRESSING, (void *)this);
	lv_obj_add_event_cb(chart, pressing_event_cb, LV_EVENT_RELEASED, (void *)this);
	lv_obj_add_event_cb(lv_obj_get_parent(chart), click_event_cb, LV_EVENT_CLICKED, (void *)this);
	FrequencyCursor = lv_chart_add_cursor(chart, lv_palette_main(LV_PALETTE_BLUE), LV_DIR_BOTTOM | LV_DIR_TOP);

	lv_obj_set_style_size(chart, 0, LV_PART_INDICATOR);
	ser = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_RED), LV_CHART_AXIS_PRIMARY_Y);
	lv_obj_clear_flag(scr, LV_OBJ_FLAG_SCROLLABLE);
	for (int i = 0; i < nfft_samples / 2; i++)
	{
		data_set.push_back(0);
		data_set_peak.push_back(0);
		data_set_nonfiltered.push_back(0);
	}
	lv_chart_set_point_count(chart, data_set.size());
	lv_chart_set_ext_y_array(chart, ser, (lv_coord_t *)data_set.data());
	avg_filter.resize(nfft_samples);
	fft = std::make_unique<FastFourier>(nfft_samples, 0, 0);
	if (waterfallsize > 0)
	{
		waterfall = std::make_unique<Waterfall>(scr, x, heightChart + fontsize, w, heightWaterfall - fontsize, 0.0, down, allparts, 12);
	}
	SetFftParts();
}

void Spectrum::setWaterfallSize(int waterfallsize)
{
	if (waterfallsize)
	{
		lv_obj_set_pos(chart, xx, yy + fontsize);
		heightChart = height - (height * waterfallsize) / 10;
		heightWaterfall = (height * waterfallsize) / 10;
		lv_obj_set_size(chart, width, heightChart);
		lv_chart_set_axis_tick(chart, LV_CHART_AXIS_SECONDARY_X, 0, 0, vert_lines, 1, true, 100);
		
	}
	else
	{
		lv_obj_set_pos(chart, xx, yy);
		lv_obj_set_size(chart, width, height - fontsize);
		lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 0, 0, vert_lines, 1, true, 100);
	}
	
	waterfall->Size(heightChart + fontsize, heightWaterfall - fontsize);
}

void Spectrum::DrawDisplay()
{
	upload_fft();
	load_data();
	if (waterfall)
		waterfall->Draw(guirx.get_waterfallgain());

	if (webserver.isEnabled())
	{
		nlohmann::json message, data;

		message.clear();
		message.emplace("type", "spectrum");
		message.emplace("data", data_set_nonfiltered);
		webserver.SendMessage(message);
	}
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
	lv_chart_set_cursor_point(chart, FrequencyCursor, ser, pos);
//	printf("Pos: freq: %ld sdr %ld offset %d pos: %d span %d \n", (long)vfo.get_frequency(), (long)vfo.get_sdr_frequency(), offset, pos, span);
}

void Spectrum::enable_second_data_series(bool enable)
{
	hold_peak = enable;
	if (hold_peak)
	{
		peak_ser = lv_chart_add_series(chart, lv_palette_main(LV_PALETTE_BLUE), LV_CHART_AXIS_PRIMARY_Y);
		for (auto m : markers)
		{
			if (m != nullptr)
				m->ser = peak_ser;
		}
	}
	else
	{
		if (peak_ser)
		{
			lv_chart_remove_series(chart, peak_ser);
			for (auto &value : data_set_peak)
				value = 0;
			peak_ser = nullptr;
		}
		for (auto m : markers)
		{
			if (m != nullptr)
				m->ser = ser;
		}
	}
}

void Spectrum::load_data()
{
	int i = data_set.size();
	if (i > 0)
	{
		lv_chart_set_point_count(chart, i);
		lv_chart_set_ext_y_array(chart, ser, (lv_coord_t *)data_set.data());
		if (peak_ser != nullptr)
		{
			lv_chart_set_ext_y_array(chart, peak_ser, (lv_coord_t *)data_set_peak.data());
		}
		lv_chart_refresh(chart);
	}
}

void Spectrum::upload_fft()
{
	int i{0};
	int noisefloor = guirx.get_spectrumgain();
	std::pair<vfo_spansetting, double> span_ex = vfo.compare_span_ex();

	switch (span_ex.first)
	{
	case span_is_ifrate:
	case span_between_ifrate:
	{
			int ii{}, value{};
			std::vector<float> fft_output = fft->GetLineatSquaredBins();
			data_set.resize(fft_output.size() / 2);
			data_set_peak.resize(fft_output.size() / 2);
			data_set_nonfiltered.resize(fft_output.size() / 2);
			finder.uploadData(fft_output);
			for (auto &col : fft_output)
			{
				value = noisefloor +(lv_coord_t)(20.0 * log10(col));
				if (value > (float)s_points_max)
					value = (float)s_points_max;
				if (i % 2)
				{
					data_set_nonfiltered[ii] = value;
					data_set[ii] = avg_filter[ii](value);
					data_set_peak[ii] = std::max(data_set_peak[ii], (lv_coord_t)value);
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
			data_set_peak.resize(fft_output.size() / 2);
			data_set_nonfiltered.resize(fft_output.size() / 2);
			finder.uploadData(fft_output);
			for (auto &col : fft_output)
			{
				if (i == (fft_output.size() / 2))
					break;
				value = noisefloor + (lv_coord_t)(20.0 * log10(col));
				if (value > (float)s_points_max)
					value = (float)s_points_max;
				data_set[i] = avg_filter[i](value);
				data_set_peak[i] = std::max(data_set_peak[i], (lv_coord_t)value);
				data_set_nonfiltered[i] = value;
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

void Spectrum::enable_marker(int marker, bool enable)
{
	if (marker < markers.size())
	{
		if (markers[marker] == nullptr && enable)
		{
			markers[marker] = lv_chart_add_cursor(chart, lv_palette_main(LV_PALETTE_LIME), LV_DIR_BOTTOM | LV_DIR_TOP);
			lv_chart_set_cursor_point(chart, markers[marker], (peak_ser == nullptr) ? ser : peak_ser, markers_location[marker]);
			active_markers++;
		}
		if (markers[marker] != nullptr && !enable)
		{
			lv_chart_remove_cursor(chart, markers[marker]);
			markers[marker] = nullptr;
			active_markers--;
		}
	}
}

void Spectrum::set_marker(int marker, int32_t offset)
{
	if (marker < markers.size())
	{
		markers_location[marker] = offset;
		if (markers[marker] != nullptr)
			lv_chart_set_cursor_point(chart, markers[marker], (peak_ser == nullptr) ? ser : peak_ser, markers_location[marker]);
	}
}

void Spectrum::draw_marker_label(lv_chart_cursor_t *cursor, lv_obj_draw_part_dsc_t *dsc)
{
	lv_coord_t *data_array = lv_chart_get_y_array(chart, cursor->ser);
	lv_coord_t v = data_array[cursor->point_id];
	char buf[16];

	lv_snprintf(buf, sizeof(buf), "%d db", v);

	lv_point_t size;
	lv_txt_get_size(&size, buf, LV_FONT_DEFAULT, 0, 0, LV_COORD_MAX, LV_TEXT_FLAG_NONE);

	lv_area_t a;
	a.x1 = dsc->p1->x + 10;
	a.x2 = a.x1 + size.x + 10;

	a.y1 = dsc->p1->y + 10;
	a.y2 = a.y1 + size.y + 10;

	lv_draw_label_dsc_t draw_label_dsc;
	lv_draw_label_dsc_init(&draw_label_dsc);
	draw_label_dsc.color = lv_color_white();
	a.x1 += 5;
	a.x2 -= 5;
	a.y1 += 5;
	a.y2 -= 5;
	lv_draw_label(dsc->draw_ctx, &draw_label_dsc, &a, buf, NULL);
}